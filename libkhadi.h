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

typedef struct Khadi Khadi;

#define KHADI_THREAD_FUNCTION(func_name) void* func_name (void)
typedef KHADI_THREAD_FUNCTION(Khadi_Thread_Function);

void khadiSetMainCPU    (Khadi *k, Uint cpu);
void khadiAddTaskCPU    (Khadi *k, Uint cpu);
void khadiAddDataCPU    (Khadi *k, Uint cpu, Uint thread_count);
void khadiAddCoroutines (Khadi *k, Size stack_size, Size count);

Size khadiGetCPUCount (void);
Size khadiCurrentCPU  (void);

B32 khadiInitialize (Khadi *khadi,
                     Khadi_Thread_Function *task_func, Khadi_Thread_Function *data_func);
void khadiFinalize (Khadi *khadi);


#define LIBKHADI_H_INCLUDE_GUARD
#endif
