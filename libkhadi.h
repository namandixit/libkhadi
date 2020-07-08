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

typedef cothread_t                  Khadi_Fiber;
typedef struct Khadi_Fiber_Metadata Khadi_Fiber_Metadata;

typedef _Atomic(U64) Khadi_Counter;

#define KHADI_TASK_FUNCTION(func_name) void* func_name (void *)
typedef KHADI_TASK_FUNCTION(Khadi_Task_Function);
typedef struct Khadi_Task Khadi_Task;

#define KHADI_THREAD_FUNCTION(func_name) void* func_name (void)
typedef KHADI_THREAD_FUNCTION(Khadi_Config_Thread_Function);

Khadi_Config* khadiCreate (void);

void khadiSetMainCPU    (Khadi_Config *k, Uint cpu);
void khadiAddTaskCPU    (Khadi_Config *k, Uint cpu);
void khadiAddDataCPU    (Khadi_Config *k, Uint cpu, Uint thread_count);
void khadiAddFibers     (Khadi_Config *k, Size stack_size, Size count);

Size khadiGetCPUCount (void);
Size khadiCurrentCPU  (void);

B32  khadiInitialize (Khadi_Config *khadi,
                      Khadi_Config_Thread_Function *task_func, Khadi_Config_Thread_Function *data_func);
void khadiFinalize   (Khadi_Config *khadi);

Khadi_Fiber khadiFiberAcquire (void);
void        khadiFiberRelease (Khadi_Fiber fiber);
#define     khadiFiberReleaseSafe(fiber) (khadiFiberRelease(fiber), (fiber) = NULL)

Khadi_Fiber_Metadata* khadiFiberGetMetadata (Khadi_Fiber fiber);


B64 khadiFiberTaskFinished (Khadi_Fiber fiber);

Khadi_Task* khadiTaskCreate  (Khadi_Task_Function *func, void *arg);
void        khadiTaskDestroy (Khadi_Task *task);

void        khadiTaskSubmit (Khadi_Task *task);
Khadi_Task* khadiTaskAccept (void);

void        khadiTaskAssignFiber  (Khadi_Task *task, Khadi_Fiber fiber);
Khadi_Fiber khadiTaskRetriveFiber (Khadi_Task *task);

void khadiTaskExecute (Khadi_Task *task);

B64 khadiTaskIsReady (Khadi_Task *task);
void khadiTaskMarkDone(Khadi_Task *task);

#define LIBKHADI_H_INCLUDE_GUARD
#endif
