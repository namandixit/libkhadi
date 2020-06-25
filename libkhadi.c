/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 */

#include "libkhadi.h"

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

struct Khadi {
    Uint main_cpu;

    Byte _pad1[4];

    Uint *task_cpus;

    struct Khadi_Data_CPUs {
        Uint cpu;
        Uint thread_count;
    } *data_cpus;

    pthread_t *task_threads;
    pthread_t *data_threads;

    struct Khadi_Coroutine_Metadata {
        Size stack_size;
        Size count;
    } *coroutines;
};

global_variable sem_t       KHADI_GLOBAL_semaphore_task_threads_init;
global_variable sem_t       KHADI_GLOBAL_semaphore_data_thread_init;
global_variable cothread_t *KHADI_GLOBAL_thread_default_coroutine_ids;

global_variable thread_local int KHADI_THREAD_LOCAL_cpu_id;

Khadi* khadiCreate (void)
{
    Khadi *k = calloc(1, sizeof(*k));
    return k;
}

void khadiSetMainCPU (Khadi *k, Uint cpu) { k->main_cpu = cpu; }
void khadiAddTaskCPU (Khadi *k, Uint cpu) { sbufAdd(k->task_cpus, cpu); }
void khadiAddDataCPU (Khadi *k, Uint cpu, Uint thread_count)
{
    struct Khadi_Data_CPUs kdc = {0};
    kdc.cpu = cpu;
    kdc.thread_count = thread_count;
    sbufAdd(k->data_cpus, kdc);
}

void khadiAddCoroutines (Khadi *k, Size stack_size, Size count)
{
    struct Khadi_Coroutine_Metadata kcm = {0};
    kcm.stack_size = stack_size;
    kcm.count = count;
    sbufAdd(k->coroutines, kcm);
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

internal_function
void* khadi__TaskFunction (void *arg) {
    KHADI_THREAD_LOCAL_cpu_id = sched_getcpu();
    KHADI_GLOBAL_thread_default_coroutine_ids[KHADI_THREAD_LOCAL_cpu_id] = co_active();
    sem_post(&KHADI_GLOBAL_semaphore_task_threads_init);

    Khadi_Thread_Function *func = (Khadi_Thread_Function*)arg;
    void *result = func();

    return result;
}

internal_function
void* khadi__DataFunction (void *arg) {
    KHADI_THREAD_LOCAL_cpu_id = sched_getcpu();
    sem_post(&KHADI_GLOBAL_semaphore_data_thread_init);

    Khadi_Thread_Function *func = (Khadi_Thread_Function*)arg;
    void *result = func();

    return result;
}

B32 khadiInitialize (Khadi *khadi,
                     Khadi_Thread_Function *task_func, Khadi_Thread_Function *data_func)
{
    sem_init(&KHADI_GLOBAL_semaphore_task_threads_init, 0, 0);
    sem_init(&KHADI_GLOBAL_semaphore_data_thread_init, 0, 0);

    Size cpu_count = khadiGetCPUCount();
    KHADI_GLOBAL_thread_default_coroutine_ids = calloc(cpu_count,
                                                       sizeof(KHADI_GLOBAL_thread_default_coroutine_ids));

    { // Pin main thread to Core #0
        pthread_t mthread = pthread_self();
        cpu_set_t cpu;
        CPU_ZERO(&cpu);
        CPU_SET(0, &cpu);
        pthread_setaffinity_np(mthread, sizeof(cpu), &cpu);
    }

    { // Create Task and Data threads
        for (Size i = 0; i < sbufElemin(khadi->task_cpus); i++) {
            cpu_set_t cpu;
            CPU_ZERO(&cpu);
            CPU_SET(khadi->task_cpus[i], &cpu);

            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);

            pthread_t thread;
            pthread_create(&thread, &attr, khadi__TaskFunction, (void*)task_func);
            sbufAdd(khadi->task_threads, thread);

            pthread_attr_destroy (&attr);
        }

        for (Size i = 0; i < sbufElemin(khadi->data_cpus); i++) { // Data threads
            for (Size j = 0; j < khadi->data_cpus[i].thread_count; j++) {
                cpu_set_t cpu;
                CPU_ZERO(&cpu);
                CPU_SET(khadi->data_cpus[i].cpu, &cpu);

                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);

                pthread_t thread;
                pthread_create(&thread, &attr, khadi__DataFunction, (void*)data_func);
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

void khadiFinalize (Khadi *khadi)
{
    for (Size i = 0; i < sbufElemin(khadi->task_threads); i++) {
        pthread_join(khadi->task_threads[i], NULL);
    }

    for (Size i = 0; i < sbufElemin(khadi->data_threads); i++) {
        pthread_join(khadi->data_threads[i], NULL);
    }

    return;
}
