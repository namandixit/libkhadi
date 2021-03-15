/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 */

#if !defined(LIBKHADI_H_INCLUDE_GUARD)

# ifdef __cplusplus
extern "C"
{
# endif

# include <stdint.h>
# include <stddef.h>
# include <stdbool.h>

/* Helper Macros */
#define KHADI_DEFINE_THREAD_LOCAL_VARIABLE(type, var)  KHADI__DEFTLV(type, var)
#define KHADI_DECLARE_THREAD_LOCAL_VARIABLE(type, var) KHADI__DECLTLV(KHADI_EXPORTED, type, var)
#define KHADI_GET_THREAD_LOCAL_VARIABLE(var)           khadi__GetDeclaredTLS_ ## var()
#define KHADI_SET_THREAD_LOCAL_VARIABLE(var, val)      khadi__SetDeclaredTLS_ ## var(val)

/* Types */
typedef _Atomic(uint64_t) Khadi_Counter;

typedef struct Khadi_Fiber_Configuration {
    size_t stack_size;
    void *stack;
} Khadi_Fiber_Configuration;

#define KHADI_THREAD_FUNCTION(func_name) void* func_name (void * arg)
typedef KHADI_THREAD_FUNCTION(Khadi_Thread_Function);

#define KHADI_CALLBACK_FUNCTION(func_name) void func_name (void * arg)
typedef KHADI_CALLBACK_FUNCTION(Khadi_Callback_Function);

#define KHADI_TASK_FUNCTION(func_name) void func_name (void * arg)
typedef KHADI_TASK_FUNCTION(Khadi_Task_Function);
typedef struct Khadi_Task Khadi_Task;

#define KHADI_ACTION_FUNCTION(func_name) void func_name (void * arg)
typedef KHADI_ACTION_FUNCTION(Khadi_Action_Function);
typedef struct Khadi_Action Khadi_Action;

#define KHADI_LAUNCHER_FUNCTION(func_name) void func_name (void)
typedef KHADI_LAUNCHER_FUNCTION(Khadi_Launcher_Function);

typedef struct {
    void *thread_userdata;
    void **current_task_userdata;
} Khadi_Task_Thread_Arguement;

typedef struct {
    Khadi_Action_Function* action_func;
    void *thread_userdata;
    void **current_action_userdata;
} Khadi_Action_Thread_Arguement;

/* Functions */

void khadiBegin (void);
void khadiEnd   (void);

bool khadiFibersCreate (Khadi_Fiber_Configuration *configs, size_t config_count);

void khadiCallbackSetTaskThread    (Khadi_Callback_Function *on_task_thread_begin,
                                    Khadi_Callback_Function *on_task_thread_end);
void khadiCallbackSetTaskFetch     (Khadi_Callback_Function *before_task_fetch,
                                    Khadi_Callback_Function *after_task_fetch);
void khadiCallbackSetTaskExecute   (Khadi_Callback_Function *before_task_execute,
                                    Khadi_Callback_Function *after_task_execute);
void khadiCallbackSetActionThread  (Khadi_Callback_Function *on_action_thread_begin,
                                    Khadi_Callback_Function *on_action_thread_end);
void khadiCallbackSetActionFetch   (Khadi_Callback_Function *before_action_fetch,
                                    Khadi_Callback_Function *after_action_fetch);
void khadiCallbackSetActionExecute (Khadi_Callback_Function *before_action_execute,
                                    Khadi_Callback_Function *after_action_execute);
void khadiCallbackSetFiberTask     (Khadi_Callback_Function *on_task_begin_in_fiber,
                                    Khadi_Callback_Function *on_task_end_in_fiber);


Khadi_Thread_Function* khadiTaskGetThreadFunction   (void);
Khadi_Thread_Function* khadiActionGetThreadFunction (void);

Khadi_Counter* khadiCounterCreate    (void);
void           khadiCounterDestroy   (Khadi_Counter *counter);
uint64_t       khadiCounterIsEqualTo (Khadi_Counter *counter, uint64_t value);

Khadi_Task*   khadiTaskCreate           (Khadi_Task_Function *func, void *arg);
void          khadiTaskDestroy          (Khadi_Task *task);
void          khadiTaskWaitOnCounter    (Khadi_Counter *counter);
void          khadiTaskSubmitAsync      (Khadi_Task *task, Khadi_Counter *counter);
void          khadiTaskSubmitAsyncMany  (Khadi_Task **task, size_t count, Khadi_Counter *counter);
void          khadiTaskSubmitSync       (Khadi_Task *task, Khadi_Counter *counter);
void          khadiTaskSubmitSyncMany   (Khadi_Task **task, size_t count, Khadi_Counter *counter);
void          khadiTaskLaunch           (Khadi_Launcher_Function *initializer,
                                         Khadi_Launcher_Function *finalizer,
                                         Khadi_Task_Function *func, void *arg);
void          khadiTaskSuspend          (void);
void*         khadiTaskGetUserdata      (Khadi_Task *task);

Khadi_Action* khadiActionCreate          (void *command);
void          khadiActionDestroy         (Khadi_Action *action);
void          khadiActionSync            (Khadi_Counter *counter);
void          khadiActionSubmitAsync     (Khadi_Action *action, Khadi_Counter *counter);
void          khadiActionSubmitAsyncMany (Khadi_Action **action, size_t count, Khadi_Counter *counter);
void          khadiActionSubmitSync      (Khadi_Action *action, Khadi_Counter *counter);
void          khadiActionSubmitSyncMany  (Khadi_Action **action, size_t count, Khadi_Counter *counter);
void*         khadiActionGetUserdata     (Khadi_Action *action);


/* Internal implementation details, put here due to the following being a macro */
# if defined(__clang__)
#  define KHADI_INTERNAL internal_function __attribute__ ((optnone))
#  define KHADI_EXPORTED __attribute__ ((optnone))
# elif defined(__GNUC__)
#  define KHADI_INTERNAL internal_function __attribute__ ((noipa))
#  define KHADI_EXPORTED __attribute__ ((noipa))
# else
#  error libkhadi only support GCC and Clang for now
# endif

# define KHADI__DECLTLV(visible, type, var)                             \
    global_variable thread_local type KHADI__DECLARED_TLS_Var_ ## var;  \
    visible                                                             \
    type khadi__GetDeclaredTLS_ ## var (void) {                         \
        type val = KHADI__DECLARED_TLS_Var_ ## var;                     \
        __asm__ volatile("");                                           \
        return val;                                                     \
    }                                                                   \
    visible                                                             \
    void khadi__SetDeclaredTLS_ ## var (type val) {                     \
        KHADI__DECLARED_TLS_Var_ ## var = val;                          \
        __asm__ volatile("");                                           \
        return;                                                         \
    }                                                                   \
    _Static_assert(true, "To swallow the ending semicolon")

# define KHADI__DEFTLV(type, var)                       \
    type khadi__GetDeclaredTLS_ ## var (void);          \
    void khadi__SetDeclaredTLS_ ## var (type val);

# ifdef __cplusplus
} // extern "C"
# endif

#define LIBKHADI_H_INCLUDE_GUARD
#endif
