/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 */

#if !defined(LIBKHADI_H_INCLUDE_GUARD)

#include "nlib/nlib.h"

#include <sched.h>
#include <pthread.h>
#include <semaphore.h>

#if defined(COMPILER_CLANG)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wstrict-prototypes"
#endif

#define LIBCO_MP
#include "libco/libco.h"

#if defined(COMPILER_CLANG)
# pragma clang diagnostic pop
#endif

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

Khadi_Counter* khadiCounterCreate  (void);
void           khadiCounterDestroy (Khadi_Counter *counter);
Khadi_Counter  khadiCounterGet     (Khadi_Counter *counter);

Khadi_Task* khadiTaskCreate          (Khadi_Task_Function *func, void *arg);
void        khadiTaskDestroy         (Khadi_Task *task);
void        khadiTaskSync            (Khadi_Counter *counter);
void        khadiTaskSubmitAsync     (Khadi_Task *task, Khadi_Counter *counter);
void        khadiTaskSubmitAsyncMany (Khadi_Task **task, Size count, Khadi_Counter *counter);
void        khadiTaskSubmitSync      (Khadi_Task *task, Khadi_Counter *counter);
void        khadiTaskSubmitSyncMany  (Khadi_Task **task, Size count, Khadi_Counter *counter);
void        khadiTaskLaunch          (Khadi_Launcher_Function *initializer,
                                      Khadi_Launcher_Function *finalizer,
                                      Khadi_Task_Function *func, void *arg);

Khadi_Action* khadiActionCreate          (void *command);
void          khadiActionDestroy         (Khadi_Action *action);
void          khadiActionSync            (Khadi_Counter *counter);
void          khadiActionSubmitAsync     (Khadi_Action *action, Khadi_Counter *counter);
void          khadiActionSubmitAsyncMany (Khadi_Action **action, Size count, Khadi_Counter *counter);
void          khadiActionSubmitSync      (Khadi_Action *action, Khadi_Counter *counter);
void          khadiActionSubmitSyncMany  (Khadi_Action **action, Size count, Khadi_Counter *counter);

#define LIBKHADI_H_INCLUDE_GUARD
#endif
