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

    struct Khadi_Config_Coroutine_Metadata {
        Size stack_size;
        Size count;
    } *coroutines;
    Size coroutine_count;
};

typedef struct Coroutine_Metadata {
    cothread_t id;
} Coroutine_Metadata;

global_variable sem_t               KHADI_GLOBAL_semaphore_task_threads_init;
global_variable sem_t               KHADI_GLOBAL_semaphore_data_thread_init;
global_variable cothread_t         *KHADI_GLOBAL_thread_default_coroutine_ids;
global_variable Coroutine_Metadata *KHADI_GLOBAL_coroutines_metadata_map;
global_variable cothread_t         *KHADI_GLOBAL_coroutines_queue;

global_variable thread_local int KHADI_THREAD_LOCAL_cpu_id;

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

void khadiAddCoroutines (Khadi_Config *k, Size stack_size, Size count)
{
    struct Khadi_Config_Coroutine_Metadata kcm = {0};
    kcm.stack_size = stack_size;
    kcm.count = count;
    sbufAdd(k->coroutines, kcm);
    k->coroutine_count++;
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

    Khadi_Config_Thread_Function *func = (Khadi_Config_Thread_Function*)arg;
    void *result = func();

    return result;
}

internal_function
void* khadi__DataFunction (void *arg) {
    KHADI_THREAD_LOCAL_cpu_id = sched_getcpu();
    sem_post(&KHADI_GLOBAL_semaphore_data_thread_init);

    Khadi_Config_Thread_Function *func = (Khadi_Config_Thread_Function*)arg;
    void *result = func();

    return result;
}

B32 khadiInitialize (Khadi_Config *khadi,
                     Khadi_Config_Thread_Function *task_func, Khadi_Config_Thread_Function *data_func)
{
    { // Create Coroutines
        KHADI_GLOBAL_coroutines_queue = queueLockedCreate(cothread_t, khadi->coroutine_count);
        for (Size i = 0; i < sbufElemin(khadi->coroutines); i++) {
            for (Size j = 0; j < khadi->coroutines[i].count; i++) {
                cothread_t co = co_create((Uint)khadi->coroutines[i].stack_size, NULL);
                Coroutine_Metadata com = {0};
                com.id = co;
                mapInsert(KHADI_GLOBAL_coroutines_metadata_map, hashInteger((Uptr)co), com);
                queueLockedEnqueue(KHADI_GLOBAL_coroutines_queue, co);
            }
        }
    }

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
        sem_init(&KHADI_GLOBAL_semaphore_task_threads_init, 0, 0);

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

Khadi_Coroutine khadiCoroutineAcquire (void)
{
    cothread_t co = queueLockedDequeue(KHADI_GLOBAL_coroutines_queue);
    return co;
}

void khadiCoroutineRelease (Khadi_Coroutine co)
{
    queueLockedEnqueue(KHADI_GLOBAL_coroutines_queue, co);
}
