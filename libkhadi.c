/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 */

#include "libkhadi.h"

#define KHADI__DECL_TLS(type, var) KHADI__DECLTLV(KHADI_INTERNAL, type, var)

#include <stdatomic.h>

#if defined(OS_LINUX)
# include <sched.h>
# include <pthread.h>
# include <semaphore.h>
#endif

#define LIBCO_MP
#if defined(LIBCO_MP)
#endif

#include "libco/libco.h"

#if defined(COMPILER_CLANG)
# if defined(ARCH_X64)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wsign-conversion"
#  pragma clang diagnostic ignored "-Wmissing-noreturn"
#  pragma clang diagnostic ignored "-Wcast-qual"
#  pragma clang diagnostic ignored "-Wcast-align"
# endif
#endif
#include "libco/libco.c"
#if defined(COMPILER_CLANG)
# pragma clang diagnostic pop
#endif

typedef cothread_t                  Khadi_Fiber;
typedef struct Khadi_Fiber_Metadata Khadi_Fiber_Metadata;

KHADI_INTERNAL void* khadiThreadActionFunction (void* arg);
KHADI_INTERNAL void* khadiThreadTaskFunction   (void *arg);
KHADI_INTERNAL void  khadiFiberFunction        (void);

struct Khadi_Config {
    Uint main_cpu;

    Byte _pad1[4];

    Uint *task_cpus;
    Size task_count;

    struct Khadi_Config_Action_CPUs {
        Uint cpu;
        Uint thread_count;
        Khadi_Action_Function *function;
    } *action_cpus;
    Size action_count;

    pthread_t *task_threads;
    pthread_t *action_threads;

    struct Khadi_Config_Fiber_Metadata {
        Size stack_size;
        Size count;
    } *fibers;
    Size fiber_count;
};

struct Khadi_Task {
    Khadi_Task_Function *func;
    void *arg;
    B64 is_launcher;
    Khadi_Launcher_Function *launcher_finalizer;
    Khadi_Fiber assigned_fiber;
    Queue_Locked_Entry queue_entry;
    Khadi_Counter *parent_counter; // Counter which this task will decrement upon completion
    Khadi_Counter *child_counter; // Counter which this task's children will decrement upon completion
};

struct Khadi_Action {
    void *command;
    Queue_Locked_Entry queue_entry;
    Khadi_Counter *parent_counter; // Counter which this action will decrement upon completion
};

struct Khadi_Fiber_Metadata {
    cothread_t id;
    Khadi_Task *assigned_task;
    B64 is_task_finished;
};

global_variable sem_t                  KG__semaphore_task_threads_init;
global_variable sem_t                  KG__semaphore_action_thread_init;

global_variable Khadi_Fiber_Metadata  *KG__fibers_metadata_map;
global_variable Khadi_Fiber           *KG__fibers_ring;
global_variable Queue_Locked_Entry    *KG__task_queue;
global_variable Queue_Locked_Entry    *KG__action_queue;

KHADI__DECL_TLS(int,         KGTLV__cpu_id);
KHADI__DECL_TLS(Khadi_Fiber, KGTLV__default_fiber_id);
KHADI__DECL_TLS(Khadi_Fiber, KGTLV__current_fiber_id);

KHADI_EXPORTED
Size khadiEnvGetCPUCount (void)
{
    cpu_set_t cs;
    CPU_ZERO(&cs);
    sched_getaffinity(0, sizeof(cs), &cs);

    Size count = 0;
    for (Size i = 0; i < 8; i++) {
        if (CPU_ISSET(i, &cs)) {
            count++;
        }
    }

    return count;
}

KHADI_EXPORTED
Size khadiEnvCurrentCPU (void)
{
    return (Size)KHADI_GET_THREAD_LOCAL_VARIABLE(KGTLV__cpu_id);
}

KHADI_EXPORTED
Khadi_Config* khadiConfigCreate (void)
{
    Khadi_Config *k = calloc(1, sizeof(*k));
    return k;
}

KHADI_EXPORTED
void khadiConfigSetMainCPU (Khadi_Config *k, Uint cpu)
{
    k->main_cpu = cpu;
}

KHADI_EXPORTED
void khadiConfigAddTaskCPU (Khadi_Config *k, Uint cpu)
{
    sbufAdd(k->task_cpus, cpu);
    k->task_count++;
}

KHADI_EXPORTED
void khadiConfigAddActionCPU (Khadi_Config *k, Uint cpu, Uint thread_count,
                              Khadi_Action_Function *func)
{
    struct Khadi_Config_Action_CPUs kdc = {0};
    kdc.cpu = cpu;
    kdc.thread_count = thread_count;
    kdc.function = func;

    sbufAdd(k->action_cpus, kdc);

    k->action_count++;
}

KHADI_EXPORTED
void khadiConfigAddFibers (Khadi_Config *k, Size stack_size, Size count)
{
    struct Khadi_Config_Fiber_Metadata kfm = {0};
    kfm.stack_size = stack_size;
    kfm.count = count;
    sbufAdd(k->fibers, kfm);
    k->fiber_count += count;
}

KHADI_EXPORTED
B32 khadiConfigConstruct (Khadi_Config *khadi)
{
    { // Create Fibers
        KG__fibers_ring = ringLockedCreate(cothread_t, khadi->fiber_count);
        for (Size i = 0; i < sbufElemin(khadi->fibers); i++) {
            for (Size j = 0; j < khadi->fibers[i].count; j++) {
                cothread_t co = co_create((Uint)khadi->fibers[i].stack_size, khadiFiberFunction);
                Khadi_Fiber_Metadata fm = {0};
                fm.id = co;
                mapInsert(KG__fibers_metadata_map, hashInteger((Uptr)co), fm);
                ringLockedPush(KG__fibers_ring, co);
            }
        }
    }

    { // Task
        KG__task_queue = queueLockedCreate();
    }

    { // Action
        KG__action_queue = queueLockedCreate();
    }

    { // Pin main thread to Core #0
        pthread_t mthread = pthread_self();
        cpu_set_t cpu;
        CPU_ZERO(&cpu);
        CPU_SET(0, &cpu);
        pthread_setaffinity_np(mthread, sizeof(cpu), &cpu);
    }

    { // Create Task and Data threads
        sem_init(&KG__semaphore_task_threads_init, 0, 0);
        { // Task threads
            for (Size i = 0; i < sbufElemin(khadi->task_cpus); i++) {
                cpu_set_t cpu;
                CPU_ZERO(&cpu);
                CPU_SET(khadi->task_cpus[i], &cpu);

                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);

                pthread_t thread;
                pthread_create(&thread, &attr, khadiThreadTaskFunction, NULL);
                sbufAdd(khadi->task_threads, thread);

                pthread_attr_destroy (&attr);
            }
        }

        sem_init(&KG__semaphore_action_thread_init, 0, 0);
        { // Action threads
            for (Size i = 0; i < sbufElemin(khadi->action_cpus); i++) {
                for (Size j = 0; j < khadi->action_cpus[i].thread_count; j++) {
                    cpu_set_t cpu;
                    CPU_ZERO(&cpu);
                    CPU_SET(khadi->action_cpus[i].cpu, &cpu);

                    pthread_attr_t attr;
                    pthread_attr_init(&attr);
                    pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);

                    pthread_t thread;
                    pthread_create(&thread, &attr, khadiThreadActionFunction,
                                   (void*)khadi->action_cpus[i].function);
                    sbufAdd(khadi->action_threads, thread);

                    pthread_attr_destroy(&attr);
                }
            }
        }

        { // Wait for threads to be created
            for (Size i = 0; i < sbufElemin(khadi->task_threads); i++) {
                sem_wait(&KG__semaphore_task_threads_init);
            }

            for (Size i = 0; i < sbufElemin(khadi->action_threads); i++) {
                sem_wait(&KG__semaphore_action_thread_init);
            }
        }
    }

    return true;
}

KHADI_EXPORTED
void khadiConfigDestruct (Khadi_Config *khadi)
{
    for (Size i = 0; i < sbufElemin(khadi->task_threads); i++) {
        pthread_cancel(khadi->task_threads[i]);
        pthread_join(khadi->task_threads[i], NULL);
    }

    for (Size i = 0; i < sbufElemin(khadi->action_threads); i++) {
        pthread_cancel(khadi->action_threads[i]);
        pthread_join(khadi->action_threads[i], NULL);
    }

    return;
}

KHADI_INTERNAL
Khadi_Fiber_Metadata* khadiFiberGetMetadata (Khadi_Fiber fiber)
{
    Khadi_Fiber_Metadata *fmp = mapGetRef(KG__fibers_metadata_map,
                                          hashInteger((Uptr)fiber));

    return fmp;
}

KHADI_INTERNAL
Khadi_Fiber_Metadata* khadiFiberGetMetadataThreadCurrent (void)
{
    Khadi_Fiber current_fiber_id = KHADI_GET_THREAD_LOCAL_VARIABLE(KGTLV__current_fiber_id);
    Khadi_Fiber_Metadata *result = khadiFiberGetMetadata(current_fiber_id);
    return result;
}

KHADI_INTERNAL
Khadi_Fiber khadiFiberAcquire (void)
{
    cothread_t co;
    ringLockedPull(KG__fibers_ring, &co);
    return co;
}

KHADI_INTERNAL
void khadiFiberRelease (Khadi_Fiber fiber)
{
    Khadi_Fiber_Metadata *fmp = khadiFiberGetMetadata(fiber);
    fmp->is_task_finished = false;

    ringLockedPush(KG__fibers_ring, fiber);
}

KHADI_INTERNAL
B64 khadiFiberIsTaskFinished (Khadi_Fiber fiber)
{
    Khadi_Fiber_Metadata fm = mapLookup(KG__fibers_metadata_map,
                                        hashInteger((Uptr)fiber));

    B64 result = fm.is_task_finished;

    return result;
}

KHADI_INTERNAL
void khadiFiberSwitchToThreadDefault (void)
{
    Khadi_Fiber default_fiber_id = KHADI_GET_THREAD_LOCAL_VARIABLE(KGTLV__default_fiber_id);
    co_switch(default_fiber_id);
}

KHADI_INTERNAL
void khadiFiberSetThreadCurrent (Khadi_Fiber fiber)
{
    KHADI_SET_THREAD_LOCAL_VARIABLE(KGTLV__current_fiber_id, fiber);
}

KHADI_INTERNAL
void khadiFiberResetThreadCurrent (void)
{
    KHADI_SET_THREAD_LOCAL_VARIABLE(KGTLV__current_fiber_id,
                                    KHADI_GET_THREAD_LOCAL_VARIABLE(KGTLV__default_fiber_id));
}

KHADI_INTERNAL
noreturn
void khadiFiberFunction (void)
{
    while (true) {
        Khadi_Fiber_Metadata *fmp = khadiFiberGetMetadataThreadCurrent();
        Khadi_Task *task = fmp->assigned_task;
        (task->func)(task->arg);
        fmp->is_task_finished = true;
        khadiFiberSwitchToThreadDefault();
    }
}

KHADI_EXPORTED
Khadi_Counter* khadiCounterCreate (void)
{
    Khadi_Counter *counter = malloc(sizeof(*counter));
    atomic_init(counter, 0);
    return counter;
}

KHADI_EXPORTED
void khadiCounterDestroy (Khadi_Counter *counter)
{
    free(counter);
}

KHADI_EXPORTED
U64 khadiCounterIsEqualTo (Khadi_Counter *counter, U64 value)
{
    U64 v2 = value;
    B32 equal = atomic_compare_exchange_strong(counter, &v2, value);
    return equal;
}

KHADI_EXPORTED
Khadi_Task* khadiTaskCreate (Khadi_Task_Function *func, void *arg)
{
    Khadi_Task *task = calloc(1, sizeof(*task));

    task->func = func;
    task->arg = arg;

    return task;
}

KHADI_EXPORTED
void khadiTaskDestroy (Khadi_Task *task)
{
    task->func = NULL;
    task->arg = NULL;
    free(task);
}

KHADI_EXPORTED
void khadiTaskWaitOnCounter (Khadi_Counter *counter)
{
    Khadi_Fiber_Metadata* fmp = khadiFiberGetMetadataThreadCurrent();
    Khadi_Task *task = fmp->assigned_task;

    task->child_counter = counter;
    khadiFiberSwitchToThreadDefault();
    task->child_counter = NULL;
}

KHADI_EXPORTED
void khadiTaskSuspend (void)
{
    Khadi_Counter *counter = khadiCounterCreate();
    khadiTaskWaitOnCounter(counter);
    khadiCounterDestroy(counter);
}

KHADI_EXPORTED
void khadiTaskSubmitAsync (Khadi_Task *task, Khadi_Counter *counter)
{
    atomic_fetch_add(counter, 1);
    task->parent_counter = counter;

    queueLockedEnqueue(KG__task_queue, &task->queue_entry);
}

KHADI_EXPORTED
void khadiTaskSubmitAsyncMany (Khadi_Task **tasks, Size count, Khadi_Counter *counter)
{
    atomic_fetch_add(counter, count);

    for (Size i = 0; i < count; i++) {
        tasks[i]->parent_counter = counter;
        queueLockedEnqueue(KG__task_queue, &(tasks[i])->queue_entry);
    }
}

KHADI_EXPORTED
void khadiTaskSubmitSync (Khadi_Task *task, Khadi_Counter *counter)
{
    khadiTaskSubmitAsync(task, counter);
    khadiTaskWaitOnCounter(counter);
}

KHADI_EXPORTED
void khadiTaskSubmitSyncMany (Khadi_Task **tasks, Size count, Khadi_Counter *counter)
{
    khadiTaskSubmitAsyncMany(tasks, count, counter);
    khadiTaskWaitOnCounter(counter);
}

KHADI_EXPORTED
void khadiTaskLaunch (Khadi_Launcher_Function *initializer,
                      Khadi_Launcher_Function *finalizer,
                      Khadi_Task_Function *func, void *arg)
{
    Khadi_Task *task = khadiTaskCreate(func, arg);
    task->is_launcher = true;
    task->launcher_finalizer = finalizer;

    Khadi_Counter *counter = khadiCounterCreate();
    khadiTaskSubmitAsync(task, counter);

    initializer();

    khadiCounterDestroy(counter);
    khadiTaskDestroy(task);
}

KHADI_INTERNAL
Khadi_Task* khadiTaskAccept (void)
{
    Queue_Locked_Entry *qr;
    queueLockedDequeue(KG__task_queue, qr);
    Khadi_Task *task = containerof(qr, Khadi_Task, queue_entry);

    return task;
}

KHADI_INTERNAL
void khadiTaskAssignFiber (Khadi_Task *task, Khadi_Fiber fiber)
{
    Khadi_Fiber_Metadata *fmp = khadiFiberGetMetadata(fiber);
    fmp->is_task_finished = false;
    fmp->assigned_task = task;

    task->assigned_fiber = fiber;
}

KHADI_INTERNAL
Khadi_Fiber khadiTaskRetriveFiber (Khadi_Task *task)
{
    Khadi_Fiber fiber = task->assigned_fiber;
    return fiber;
}

KHADI_INTERNAL
void khadiTaskExecute (Khadi_Task *task)
{
    khadiFiberSetThreadCurrent(task->assigned_fiber);
    co_switch(task->assigned_fiber);
    khadiFiberResetThreadCurrent();
}

KHADI_INTERNAL
B64 khadiTaskIsReady (Khadi_Task *task)
{
    B64 result = false;
    if (task->child_counter == NULL) {
        result = true;
    } else if (*(task->child_counter) == 0) {
        result = true;
    }

    return result;
}

KHADI_INTERNAL
void khadiTaskMarkDone(Khadi_Task *task)
{
    if (task->parent_counter != NULL) {
        atomic_fetch_sub(task->parent_counter, 1);
    }
}

KHADI_INTERNAL
void* khadiThreadTaskFunction (void *arg) {
    unused_variable(arg);

    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    {
        KHADI_SET_THREAD_LOCAL_VARIABLE(KGTLV__cpu_id, sched_getcpu());
        KHADI_SET_THREAD_LOCAL_VARIABLE(KGTLV__default_fiber_id, co_active());

        sem_post(&KG__semaphore_task_threads_init);

        printf("Task CPU: %zu\n", khadiEnvCurrentCPU());
    }

    while (true) {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        Khadi_Task *task = khadiTaskAccept();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        if (!khadiTaskIsReady(task)) {
            queueLockedEnqueue(KG__task_queue, &(task)->queue_entry);
            continue;
        }

        Khadi_Fiber fiber = khadiTaskRetriveFiber(task);
        if (fiber == NULL) {
            fiber = khadiFiberAcquire();
            khadiTaskAssignFiber(task, fiber);
        }

        khadiTaskExecute(task);

        if (khadiFiberIsTaskFinished(fiber)) {
            khadiTaskMarkDone(task);

            khadiFiberRelease(fiber);

            if (task->is_launcher) {
                task->launcher_finalizer();
            }
        } else {
            queueLockedEnqueue(KG__task_queue, &(task)->queue_entry);
        }
    }

    return NULL;
}


KHADI_EXPORTED
Khadi_Action* khadiActionCreate (void *command)
{
    Khadi_Action *action = calloc(1, sizeof(*action));

    action->command = command;

    return action;
}

KHADI_EXPORTED
void khadiActionDestroy (Khadi_Action *action)
{
    action->command = NULL;
    free(action);
}

KHADI_EXPORTED
void khadiActionSync (Khadi_Counter *counter)
{
    Khadi_Fiber_Metadata* fmp = khadiFiberGetMetadataThreadCurrent();
    Khadi_Task *task = fmp->assigned_task;

    task->child_counter = counter;
    khadiFiberSwitchToThreadDefault();
    task->child_counter = NULL;
}

KHADI_EXPORTED
void khadiActionSubmitAsync (Khadi_Action *action, Khadi_Counter *counter)
{
    atomic_fetch_add(counter, 1);
    action->parent_counter = counter;

    queueLockedEnqueue(KG__action_queue, &action->queue_entry);
}

KHADI_EXPORTED
void khadiActionSubmitAsyncMany (Khadi_Action **actions, Size count, Khadi_Counter *counter)
{
    atomic_fetch_add(counter, count);

    for (Size i = 0; i < count; i++) {
        actions[i]->parent_counter = counter;
        queueLockedEnqueue(KG__action_queue, &(actions[i])->queue_entry);
    }
}

KHADI_EXPORTED
void khadiActionSubmitSync (Khadi_Action *action, Khadi_Counter *counter)
{
    khadiActionSubmitAsync(action, counter);
    khadiActionSync(counter);
}

KHADI_EXPORTED
void khadiActionSubmitSyncMany (Khadi_Action **actions, Size count, Khadi_Counter *counter)
{
    khadiActionSubmitAsyncMany(actions, count, counter);
    khadiActionSync(counter);
}

KHADI_INTERNAL
Khadi_Action* khadiActionAccept (void)
{
    Queue_Locked_Entry *qr;
    queueLockedDequeue(KG__action_queue, qr);
    Khadi_Action *action = containerof(qr, Khadi_Action, queue_entry);

    return action;
}

KHADI_INTERNAL
void khadiActionMarkDone (Khadi_Action *action)
{
    if (action->parent_counter != NULL) {
        atomic_fetch_sub(action->parent_counter, 1);
    }
}

KHADI_INTERNAL
void* khadiThreadActionFunction (void* arg) {
    Khadi_Action_Function *function = (Khadi_Action_Function*)arg;

    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    {
        KHADI_SET_THREAD_LOCAL_VARIABLE(KGTLV__cpu_id, sched_getcpu());
        sem_post(&KG__semaphore_action_thread_init);

        printf("Action CPU: %zu\n", khadiEnvCurrentCPU());
    }

    while (true) {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        Khadi_Action *action = khadiActionAccept();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        function(action->command);

        khadiActionMarkDone(action);
    }

    return NULL;
}
