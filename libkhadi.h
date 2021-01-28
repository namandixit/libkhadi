/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 */

#if !defined(LIBKHADI_H_INCLUDE_GUARD)

#include "nlib.h"

#define KHADI_DEFINE_THREAD_LOCAL_VARIABLE(type, var)  KHADI__DEFTLV(type, var)
#define KHADI_DECLARE_THREAD_LOCAL_VARIABLE(type, var) KHADI__DECLTLV(KHADI_EXPORTED, type, var)
#define KHADI_GET_THREAD_LOCAL_VARIABLE(var)           khadi__GetDeclaredTLS_ ## var()
#define KHADI_SET_THREAD_LOCAL_VARIABLE(var, val)      khadi__SetDeclaredTLS_ ## var(val)

typedef struct Khadi_Config Khadi_Config;

typedef _Atomic(U64) Khadi_Counter;

#define KHADI_TASK_FUNCTION(func_name) void func_name (void * arg)
typedef KHADI_TASK_FUNCTION(Khadi_Task_Function);
typedef struct Khadi_Task Khadi_Task;

#define KHADI_ACTION_FUNCTION(func_name) void func_name (void * arg)
typedef KHADI_ACTION_FUNCTION(Khadi_Action_Function);
typedef struct Khadi_Action Khadi_Action;

#define KHADI_LAUNCHER_FUNCTION(func_name) void func_name (void)
typedef KHADI_LAUNCHER_FUNCTION(Khadi_Launcher_Function);

Size khadiEnvGetCPUCount (void);
Size khadiEnvCurrentCPU  (void);

Khadi_Config* khadiConfigCreate       (void);
void          khadiConfigSetMainCPU   (Khadi_Config *k, Uint cpu);
void          khadiConfigAddTaskCPU   (Khadi_Config *k, Uint cpu);
void          khadiConfigAddActionCPU (Khadi_Config *k, Uint cpu, Uint thread_count, Khadi_Action_Function *function);
void          khadiConfigAddFibers    (Khadi_Config *k, Size stack_size, Size count);
B32           khadiConfigConstruct    (Khadi_Config *khadi);
void          khadiConfigDestruct     (Khadi_Config *khadi);

Khadi_Counter* khadiCounterCreate    (void);
void           khadiCounterDestroy   (Khadi_Counter *counter);
U64            khadiCounterIsEqualTo (Khadi_Counter *counter, U64 value);

Khadi_Task* khadiTaskCreate          (Khadi_Task_Function *func, void *arg);
void        khadiTaskDestroy         (Khadi_Task *task);
void        khadiTaskWaitOnCounter   (Khadi_Counter *counter);
void        khadiTaskSubmitAsync     (Khadi_Task *task, Khadi_Counter *counter);
void        khadiTaskSubmitAsyncMany (Khadi_Task **task, Size count, Khadi_Counter *counter);
void        khadiTaskSubmitSync      (Khadi_Task *task, Khadi_Counter *counter);
void        khadiTaskSubmitSyncMany  (Khadi_Task **task, Size count, Khadi_Counter *counter);
void        khadiTaskLaunch          (Khadi_Launcher_Function *initializer,
                                      Khadi_Launcher_Function *finalizer,
                                      Khadi_Task_Function *func, void *arg);
void        khadiTaskSuspend         (void);

Khadi_Action* khadiActionCreate          (void *command);
void          khadiActionDestroy         (Khadi_Action *action);
void          khadiActionSync            (Khadi_Counter *counter);
void          khadiActionSubmitAsync     (Khadi_Action *action, Khadi_Counter *counter);
void          khadiActionSubmitAsyncMany (Khadi_Action **action, Size count, Khadi_Counter *counter);
void          khadiActionSubmitSync      (Khadi_Action *action, Khadi_Counter *counter);
void          khadiActionSubmitSyncMany  (Khadi_Action **action, Size count, Khadi_Counter *counter);


/* Macro implementation stuff */
#if defined(COMPILER_GCC)
# define KHADI_INTERNAL internal_function __attribute__ ((noipa))
# define KHADI_EXPORTED __attribute__ ((noipa))
#elif defined(COMPILER_CLANG)
# define KHADI_INTERNAL internal_function __attribute__ ((optnone))
# define KHADI_EXPORTED __attribute__ ((optnone))
#else
# error libkhadi only support GCC and Clang for now
#endif

#define KHADI__DECLTLV(visible, type, var)                              \
    global_variable thread_local type KHADI__DECLARED_TLS_Var ## var;   \
    visible                                                             \
    type khadi__GetDeclaredTLS_ ## var (void) {                         \
        type val = KHADI__DECLARED_TLS_Var ## var;                      \
        __asm__ volatile("");                                           \
        return val;                                                     \
    }                                                                   \
    visible                                                             \
    void khadi__SetDeclaredTLS_ ## var (type val) {                     \
        KHADI__DECLARED_TLS_Var ## var = val;                           \
        __asm__ volatile("");                                           \
        return;                                                         \
    }                                                                   \
    _Static_assert(true, "To swallow the ending semicolon")

#define KHADI__DEFTLV(type, var)                        \
    type khadi__GetDeclaredTLS_ ## var (void);          \
    void khadi__SetDeclaredTLS_ ## var (type val);


#define LIBKHADI_H_INCLUDE_GUARD
#endif
