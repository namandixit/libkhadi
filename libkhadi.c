/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 */

#define KHADI_INTERNAL internal_function __attribute__ ((noinline))

#include "libkhadi.h"
#include <stdatomic.h>

#if defined(COMPILER_CLANG)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wstrict-prototypes"
# pragma clang diagnostic ignored "-Wmacro-redefined"
# pragma clang diagnostic ignored "-Wsign-conversion"
# pragma clang diagnostic ignored "-Wmissing-noreturn"
# pragma clang diagnostic ignored "-Wmissing-prototypes"
# pragma clang diagnostic ignored "-Wcast-qual"
# pragma clang diagnostic ignored "-Wcast-align"
#endif

#define LIBCO_MP
#include "libco/libco.c"

#if defined(COMPILER_CLANG)
# pragma clang diagnostic pop
#endif

KHADI_INTERNAL void* khadi__ThreadDataFunction (void* arg);
KHADI_INTERNAL void* khadi__ThreadTaskFunction (void *arg);

struct Khadi_Config {
    Uint main_cpu;

    Byte _pad1[4];

    Uint *task_cpus;

    struct Khadi_Config_Data_CPUs {
        Uint cpu;
        Uint thread_count;
    } *data_cpus;

    pthread_t *task_threads;
    pthread_t *data_threads;

    struct Khadi_Config_Fiber_Metadata {
        Size stack_size;
        Size count;
    } *fibers;
    Size fiber_count;
};

struct Khadi_Task {
    Khadi_Task_Function *func;
    void *arg;
    Khadi_Fiber assigned_fiber;
    Queue_Locked_Entry queue_entry;
    Khadi_Counter *parent_counter; // Counter which this task will decrement
    Khadi_Counter child_counter; // Counter which this task's children will decrement
};


struct Khadi_Fiber_Metadata {
    cothread_t id;
    B64 has_been_assigned_a_task;
};

global_variable sem_t                 KHADI_GLOBAL_semaphore_task_threads_init;
global_variable sem_t                 KHADI_GLOBAL_semaphore_data_thread_init;
global_variable Khadi_Fiber_Metadata *KHADI_GLOBAL_fibers_metadata_map;
global_variable Khadi_Fiber          *KHADI_GLOBAL_fibers_ring;
global_variable Queue_Locked_Entry   *KHADI_GLOBAL_task_queue;

global_variable thread_local int          KHADI_THREAD_LOCAL_cpu_id;
global_variable thread_local Khadi_Fiber  KHADI_GLOBAL_thread_default_fiber_id;

Khadi_Config* khadiCreate (void)
{
    Khadi_Config *k = calloc(1, sizeof(*k));
    return k;
}

void khadiSetMainCPU (Khadi_Config *k, Uint cpu) { k->main_cpu = cpu; }
void khadiAddTaskCPU (Khadi_Config *k, Uint cpu) { sbufAdd(k->task_cpus, cpu); }
void khadiAddDataCPU (Khadi_Config *k, Uint cpu, Uint thread_count)
{
    struct Khadi_Config_Data_CPUs kdc = {0};
    kdc.cpu = cpu;
    kdc.thread_count = thread_count;
    sbufAdd(k->data_cpus, kdc);
}

void khadiAddFibers (Khadi_Config *k, Size stack_size, Size count)
{
    struct Khadi_Config_Fiber_Metadata kfm = {0};
    kfm.stack_size = stack_size;
    kfm.count = count;
    sbufAdd(k->fibers, kfm);
    k->fiber_count++;
}

Size khadiGetCPUCount (void)
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

Size khadiCurrentCPU (void)
{
    return (Size)KHADI_THREAD_LOCAL_cpu_id;
}

B32 khadiInitialize (Khadi_Config *khadi)
{
    { // Create Fibers
        KHADI_GLOBAL_fibers_ring = ringLockedCreate(cothread_t, khadi->fiber_count);
        for (Size i = 0; i < sbufElemin(khadi->fibers); i++) {
            for (Size j = 0; j < khadi->fibers[i].count; i++) {
                cothread_t co = co_create((Uint)khadi->fibers[i].stack_size, NULL);
                Khadi_Fiber_Metadata fm = {0};
                fm.id = co;
                mapInsert(KHADI_GLOBAL_fibers_metadata_map, hashInteger((Uptr)co), fm);
                ringLockedPush(KHADI_GLOBAL_fibers_ring, co);
            }
        }
    }

    { // Task
        KHADI_GLOBAL_task_queue = queueLockedCreate();
    }

    { // Pin main thread to Core #0
        pthread_t mthread = pthread_self();
        cpu_set_t cpu;
        CPU_ZERO(&cpu);
        CPU_SET(0, &cpu);
        pthread_setaffinity_np(mthread, sizeof(cpu), &cpu);
    }

    { // Create Task and Data threads
        sem_init(&KHADI_GLOBAL_semaphore_task_threads_init, 0, 0);

        for (Size i = 0; i < sbufElemin(khadi->task_cpus); i++) {
            cpu_set_t cpu;
            CPU_ZERO(&cpu);
            CPU_SET(khadi->task_cpus[i], &cpu);

            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);

            pthread_t thread;
            pthread_create(&thread, &attr, khadi__ThreadTaskFunction, NULL);
            sbufAdd(khadi->task_threads, thread);

            pthread_attr_destroy (&attr);
        }

        sem_init(&KHADI_GLOBAL_semaphore_data_thread_init, 0, 0);

        for (Size i = 0; i < sbufElemin(khadi->data_cpus); i++) { // Data threads
            for (Size j = 0; j < khadi->data_cpus[i].thread_count; j++) {
                cpu_set_t cpu;
                CPU_ZERO(&cpu);
                CPU_SET(khadi->data_cpus[i].cpu, &cpu);

                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);

                pthread_t thread;
                pthread_create(&thread, &attr, khadi__ThreadDataFunction, NULL);
                sbufAdd(khadi->data_threads, thread);

                pthread_attr_destroy (&attr);
            }
        }

        { // Wait for threads to be created
            for (Size i = 0; i < sbufElemin(khadi->task_threads); i++) {
                sem_wait(&KHADI_GLOBAL_semaphore_task_threads_init);
            }

            for (Size i = 0; i < sbufElemin(khadi->data_threads); i++) {
                sem_wait(&KHADI_GLOBAL_semaphore_data_thread_init);
            }
        }
    }



    return true;
}

void khadiFinalize (Khadi_Config *khadi)
{
    for (Size i = 0; i < sbufElemin(khadi->task_threads); i++) {
        pthread_join(khadi->task_threads[i], NULL);
    }

    for (Size i = 0; i < sbufElemin(khadi->data_threads); i++) {
        pthread_join(khadi->data_threads[i], NULL);
    }

    return;
}

KHADI_INTERNAL
Khadi_Fiber khadiFiberAcquire (void)
{
    cothread_t co;
    ringLockedPull(KHADI_GLOBAL_fibers_ring, &co);
    return co;
}

KHADI_INTERNAL
void khadiFiberRelease (Khadi_Fiber fiber)
{
    ringLockedPush(KHADI_GLOBAL_fibers_ring, fiber);
}

KHADI_INTERNAL
Khadi_Fiber_Metadata* khadiFiberGetMetadata (Khadi_Fiber fiber)
{
    Khadi_Fiber_Metadata *fmp = mapGetRef(KHADI_GLOBAL_fibers_metadata_map,
                                          hashInteger((Uptr)fiber));

    return fmp;
}

KHADI_INTERNAL
B64 khadiFiberIsTaskFinished (Khadi_Fiber fiber)
{
    Khadi_Fiber_Metadata fm = mapLookup(KHADI_GLOBAL_fibers_metadata_map,
                                        hashInteger((Uptr)fiber));

    B64 result = !(fm.has_been_assigned_a_task);

    return result;
}

Khadi_Task* khadiTaskCreate (Khadi_Task_Function *func, void *arg)
{
    Khadi_Task *task = calloc(1, sizeof(*task));

    task->func = func;
    task->arg = arg;

    return task;
}

void khadiTaskDestroy (Khadi_Task *task)
{
    free(task);
}

KHADI_INTERNAL
void khadiTaskSubmit (Khadi_Task *task)
{
    queueLockedEnqueue(KHADI_GLOBAL_task_queue, &task->queue_entry);
}

KHADI_INTERNAL
Khadi_Task* khadiTaskAccept (void)
{
    Queue_Locked_Entry *qr;
    queueLockedDequeue(KHADI_GLOBAL_task_queue, qr);
    Khadi_Task *task = containerof(qr, Khadi_Task, queue_entry);

    return task;
}

KHADI_INTERNAL
void khadiTaskAssignFiber (Khadi_Task *task, Khadi_Fiber fiber)
{
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
    co_switch(task->assigned_fiber);
}

KHADI_INTERNAL
B64 khadiTaskIsReady (Khadi_Task *task)
{
    B64 result = false;
    if (task->child_counter == 0) {
        result = true;
    }

    return result;
}

KHADI_INTERNAL
void khadiTaskMarkDone(Khadi_Task *task)
{
    atomic_fetch_sub(task->parent_counter, 1);
}

KHADI_INTERNAL
void* khadi__ThreadTaskFunction (void *arg) {
    unused_variable(arg);

    KHADI_THREAD_LOCAL_cpu_id = sched_getcpu();
    KHADI_GLOBAL_thread_default_fiber_id = co_active();
    sem_post(&KHADI_GLOBAL_semaphore_task_threads_init);

    printf("Task CPU: %zu\n", khadiCurrentCPU());

    while (true) {
        Khadi_Task *task = khadiTaskAccept();
        if (!khadiTaskIsReady(task)) {
            khadiTaskSubmit(task);
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
        } else {
            khadiTaskSubmit(task);
        }
    }

    return NULL;
}

KHADI_INTERNAL
void* khadi__ThreadDataFunction (void* arg) {
    unused_variable(arg);

    KHADI_THREAD_LOCAL_cpu_id = sched_getcpu();
    sem_post(&KHADI_GLOBAL_semaphore_data_thread_init);

    printf("Data CPU: %zu\n", khadiCurrentCPU());
    // ...

    return NULL;
}
