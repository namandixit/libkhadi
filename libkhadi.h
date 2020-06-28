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

#define KHADI_THREAD_FUNCTION(func_name) void* func_name (void)
typedef KHADI_THREAD_FUNCTION(Khadi_Config_Thread_Function);

Khadi_Config* khadiCreate (void);

void khadiSetMainCPU    (Khadi_Config *k, Uint cpu);
void khadiAddTaskCPU    (Khadi_Config *k, Uint cpu);
void khadiAddDataCPU    (Khadi_Config *k, Uint cpu, Uint thread_count);
void khadiAddCoroutines (Khadi_Config *k, Size stack_size, Size count);

Size khadiGetCPUCount (void);
Size khadiCurrentCPU  (void);

B32 khadiInitialize (Khadi_Config *khadi,
                     Khadi_Config_Thread_Function *task_func, Khadi_Config_Thread_Function *data_func);
void khadiFinalize (Khadi_Config *khadi);


#define LIBKHADI_H_INCLUDE_GUARD
#endif
