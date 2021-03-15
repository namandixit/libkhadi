/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 */

#include "libkhadi.h"
#include "nlib.h"

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
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wsign-conversion"
# pragma clang diagnostic ignored "-Wmissing-noreturn"
# pragma clang diagnostic ignored "-Wcast-qual"
# pragma clang diagnostic ignored "-Wcast-align"
#endif
#include "libco/libco.c"
#if defined(COMPILER_CLANG)
# pragma clang diagnostic pop
#endif

typedef struct Khadi_Fiber {
    cothread_t cothread;
    Khadi_Task *assigned_task;
    B64 is_task_finished;
} Khadi_Fiber;

struct Khadi_Task {
    Khadi_Task_Function *func;
    void *arg;
    B64 is_launcher;
    Khadi_Launcher_Function *launcher_finalizer;
    Khadi_Fiber *assigned_fiber;
    Queue_Locked_Entry queue_entry;
    Khadi_Counter *parent_counter; // Counter which this task will decrement upon completion
    Khadi_Counter *child_counter; // Counter which this task's children will decrement upon completion

    void *userdata;
};

struct Khadi_Action {
    void *command;
    Queue_Locked_Entry queue_entry;
    Khadi_Counter *parent_counter; // Counter which this action will decrement upon completion
    void *userdata;
};

KHADI_INTERNAL void  khadiFiberFunction        (void);

global_variable Khadi_Fiber*          *KG__fibers_ring;
global_variable Queue_Locked_Entry    *KG__task_queue;
global_variable Queue_Locked_Entry    *KG__action_queue;

global_variable struct {
    Khadi_Callback_Function *on_task_thread_begin, *on_task_thread_end;
    Khadi_Callback_Function *before_task_fetch, *after_task_fetch;
    Khadi_Callback_Function *before_task_execute, *after_task_execute;

    Khadi_Callback_Function *on_action_thread_begin, *on_action_thread_end;
    Khadi_Callback_Function *before_action_fetch, *after_action_fetch;
    Khadi_Callback_Function *before_action_execute, *after_action_execute;

    Khadi_Callback_Function *on_task_begin_in_fiber, *on_task_end_in_fiber;
} KG__callbacks;

#define KHADI__CALLBACK(cbf,arg)                \
    do {                                        \
        if (KG__callbacks.cbf != NULL) {        \
            KG__callbacks.cbf(arg);             \
        } else {                                \
        }                                       \
    } while(0)

KHADI__DECL_TLS(Khadi_Fiber*, KGTLV__default_fiber);
KHADI__DECL_TLS(Khadi_Fiber*, KGTLV__current_fiber);

KHADI_EXPORTED
void khadiBegin (void)
{
    KG__task_queue = queueLockedCreate();
    KG__action_queue = queueLockedCreate();
}

KHADI_EXPORTED
void khadiEnd (void)
{
    return;
}

KHADI_EXPORTED
void khadiCallbackSetTaskThread (Khadi_Callback_Function *on_task_thread_begin,
                                 Khadi_Callback_Function *on_task_thread_end)
{
    KG__callbacks.on_task_thread_begin = on_task_thread_begin;
    KG__callbacks.on_task_thread_end = on_task_thread_end;
    return;
}

KHADI_EXPORTED
void khadiCallbackSetTaskFetch (Khadi_Callback_Function *before_task_fetch,
                                Khadi_Callback_Function *after_task_fetch)
{
    KG__callbacks.before_task_fetch = before_task_fetch;
    KG__callbacks.after_task_fetch = after_task_fetch;
    return;
}

KHADI_EXPORTED
void khadiCallbackSetTaskExecute (Khadi_Callback_Function *before_task_execute,
                                  Khadi_Callback_Function *after_task_execute)
{
    KG__callbacks.before_task_execute = before_task_execute;
    KG__callbacks.after_task_execute = after_task_execute;
    return;
}

KHADI_EXPORTED
void khadiCallbackSetActionThread (Khadi_Callback_Function *on_action_thread_begin,
                                   Khadi_Callback_Function *on_action_thread_end)
{
    KG__callbacks.on_action_thread_begin = on_action_thread_begin;
    KG__callbacks.on_action_thread_end = on_action_thread_end;
    return;
}

KHADI_EXPORTED
void khadiCallbackSetActionFetch (Khadi_Callback_Function *before_action_fetch,
                                  Khadi_Callback_Function *after_action_fetch)
{
    KG__callbacks.before_action_fetch = before_action_fetch;
    KG__callbacks.after_action_fetch = after_action_fetch;
    return;
}

KHADI_EXPORTED
void khadiCallbackSetActionExecute (Khadi_Callback_Function *before_action_execute,
                                    Khadi_Callback_Function *after_action_execute)
{
    KG__callbacks.before_action_execute = before_action_execute;
    KG__callbacks.after_action_execute = after_action_execute;
    return;
}

KHADI_EXPORTED
void khadiCallbackSetFiberTask (Khadi_Callback_Function *on_task_begin_in_fiber,
                                Khadi_Callback_Function *on_task_end_in_fiber)
{
    KG__callbacks.on_task_begin_in_fiber = on_task_begin_in_fiber;
    KG__callbacks.on_task_end_in_fiber = on_task_end_in_fiber;
    return;
}

KHADI_EXPORTED
Bool khadiFibersCreate (Khadi_Fiber_Configuration *configs, Size config_count)
{
    KG__fibers_ring = ringLockedCreate(cothread_t, config_count);

    for (Size i = 0; i < config_count; i++) {
        cothread_t co = co_derive(configs[i].stack,
                                  (Uint)configs[i].stack_size,
                                  khadiFiberFunction);
        Khadi_Fiber *fiber = malloc(sizeof(*fiber));
        *fiber = (Khadi_Fiber){.cothread = co};

        ringLockedPush(KG__fibers_ring, fiber);
    }

    return true;
}

KHADI_INTERNAL
Khadi_Fiber* khadiFiberGetCurrentlyRunning (void)
{
    Khadi_Fiber *current_fiber = KHADI_THREAD_LOCAL_GET(KGTLV__current_fiber);
    return current_fiber;
}

KHADI_INTERNAL
Khadi_Fiber* khadiFiberAcquire (void)
{
    Khadi_Fiber *fiber;
    ringLockedPull(KG__fibers_ring, &fiber);
    return fiber;
}

KHADI_INTERNAL
void khadiFiberRelease (Khadi_Fiber *fiber)
{
    fiber->is_task_finished = false;

    ringLockedPush(KG__fibers_ring, fiber);
}

KHADI_INTERNAL
B64 khadiFiberIsTaskFinished (Khadi_Fiber *fiber)
{
    B64 result = fiber->is_task_finished;

    return result;
}

KHADI_INTERNAL
void khadiFiberSetThreadCurrent (Khadi_Fiber *fiber)
{
    KHADI_THREAD_LOCAL_SET(KGTLV__current_fiber, fiber);
}

KHADI_INTERNAL
void khadiFiberSwitchToThreadDefault (void)
{
    Khadi_Fiber *default_fiber = KHADI_THREAD_LOCAL_GET(KGTLV__default_fiber);
    khadiFiberSetThreadCurrent(default_fiber);
    co_switch(default_fiber->cothread);
}

KHADI_INTERNAL
void khadiFiberResetThreadCurrent (void)
{
    KHADI_THREAD_LOCAL_SET(KGTLV__current_fiber,
                                    KHADI_THREAD_LOCAL_GET(KGTLV__default_fiber));
}

KHADI_INTERNAL
noreturn
void khadiFiberFunction (void)
{
    while (true) {
        Khadi_Fiber *fiber = khadiFiberGetCurrentlyRunning();
        Khadi_Task *task = fiber->assigned_task;

        KHADI__CALLBACK(on_task_begin_in_fiber, NULL);
        (task->func)(task->arg);
        KHADI__CALLBACK(on_task_end_in_fiber, NULL);

        fiber->is_task_finished = true;
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
    Bool equal = atomic_compare_exchange_strong(counter, &v2, value);
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
    Khadi_Fiber* fiber = khadiFiberGetCurrentlyRunning();
    Khadi_Task *task = fiber->assigned_task;

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
void khadiTaskAssignFiber (Khadi_Task *task, Khadi_Fiber *fiber)
{
    fiber->is_task_finished = false;
    fiber->assigned_task = task;

    task->assigned_fiber = fiber;
}

KHADI_INTERNAL
Khadi_Fiber* khadiTaskRetriveFiber (Khadi_Task *task)
{
    Khadi_Fiber *fiber = task->assigned_fiber;
    return fiber;
}

KHADI_INTERNAL
void khadiTaskExecute (Khadi_Task *task)
{
    khadiFiberSetThreadCurrent(task->assigned_fiber);
    co_switch(task->assigned_fiber->cothread);
    khadiFiberResetThreadCurrent();
}

KHADI_INTERNAL
B64 khadiTaskIsReady (Khadi_Task *task)
{
    B64 result = false;
    if (task->child_counter == NULL) {
        result = true;
    } else if (khadiCounterIsEqualTo(task->child_counter, 0)) {
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

KHADI_EXPORTED
void* khadiTaskGetUserdata (Khadi_Task *task)
{
    return task->userdata;
}

KHADI_INTERNAL
KHADI_THREAD_FUNCTION(khadiTaskThreadFunction) {
    Khadi_Task_Thread_Arguement *data = arg;

    KHADI_THREAD_LOCAL_SET(KGTLV__default_fiber, co_active());

    KHADI__CALLBACK(on_task_thread_begin, data);

    while (true) {
        // NOTE(naman): Use pthread_testcancel here in lockless code
        KHADI__CALLBACK(before_task_fetch, data);
        Khadi_Task *task = khadiTaskAccept();
        KHADI__CALLBACK(after_task_fetch, data);

        if (!khadiTaskIsReady(task)) {
            queueLockedEnqueue(KG__task_queue, &(task)->queue_entry);
            continue;
        }

        data->current_task_userdata = &(task->userdata);

        Khadi_Fiber *fiber = khadiTaskRetriveFiber(task);
        if (fiber == NULL) {
            fiber = khadiFiberAcquire();
            khadiTaskAssignFiber(task, fiber);
        }

        KHADI__CALLBACK(before_task_execute, data);
        khadiTaskExecute(task);
        KHADI__CALLBACK(after_task_execute, data);

        data->current_task_userdata = NULL;

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

    KHADI__CALLBACK(on_task_thread_end, data);

    return NULL;
}

KHADI_EXPORTED
Khadi_Thread_Function* khadiTaskGetThreadFunction (void)
{
    return &khadiTaskThreadFunction;
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
    Khadi_Fiber *fiber = khadiFiberGetCurrentlyRunning();
    Khadi_Task *task = fiber->assigned_task;

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

KHADI_EXPORTED
void* khadiActionGetUserdata (Khadi_Action *action)
{
    return action->userdata;
}

KHADI_INTERNAL
KHADI_THREAD_FUNCTION(khadiActionThreadFunction) {
    Khadi_Action_Thread_Arguement *data = arg;
    Khadi_Action_Function *function = data->action_func;

    claim(function != NULL);

    KHADI__CALLBACK(on_action_thread_begin, arg);

    while (true) {
        KHADI__CALLBACK(before_action_fetch, arg);
        Khadi_Action *action = khadiActionAccept();
        KHADI__CALLBACK(after_action_fetch, arg);

        data->current_action_userdata = &(action->userdata);

        KHADI__CALLBACK(before_action_execute, arg);
        function(action->command);
        KHADI__CALLBACK(after_action_execute, arg);

        data->current_action_userdata = NULL;

        khadiActionMarkDone(action);
    }

    KHADI__CALLBACK(on_action_thread_end, arg);

    return NULL;
}

KHADI_EXPORTED
Khadi_Thread_Function* khadiActionGetThreadFunction (void)
{
    return &khadiActionThreadFunction;
}
