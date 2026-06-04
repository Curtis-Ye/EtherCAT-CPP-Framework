#include "csp_config.h"
#include "csp_util.h"

#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>

/* ============================================================
 * csp_util.cpp — 系统 & 时间工具实现
 * ============================================================ */

/* --- 全局变量定义 --- */
uint32_t interval_ = (uint32_t)(1000000000.0 / FREQUENCY);

/* --- 时间运算 --- */

void timespec_add(struct timespec *result,
                  struct timespec *time1,
                  struct timespec *time2)
{
    if ((time1->tv_nsec + time2->tv_nsec) >= NSEC_PER_SEC) {
        result->tv_sec  = time1->tv_sec + time2->tv_sec + 1;
        result->tv_nsec = time1->tv_nsec + time2->tv_nsec - NSEC_PER_SEC;
    } else {
        result->tv_sec  = time1->tv_sec + time2->tv_sec;
        result->tv_nsec = time1->tv_nsec + time2->tv_nsec;
    }
}

void timespec_sub(struct timespec *result,
                  struct timespec *time1,
                  struct timespec *time2)
{
    if ((time1->tv_nsec - time2->tv_nsec) < 0) {
        result->tv_sec  = time1->tv_sec - time2->tv_sec - 1;
        result->tv_nsec = NSEC_PER_SEC - (time2->tv_nsec - time1->tv_nsec);
    } else {
        result->tv_sec  = time1->tv_sec - time2->tv_sec;
        result->tv_nsec = time1->tv_nsec - time2->tv_nsec;
    }
}

/* --- 系统配置 --- */

void set_cpu_affinity(int cpu)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);

    if (sched_setaffinity(0, sizeof(set), &set)) {
        printf("Setting CPU affinity failed!\n");
    }
}

void set_realtime_priority(void)
{
    struct sched_param param = {};
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    printf("Using priority %i.\n", param.sched_priority);

    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("sched_setscheduler failed\n");
    }
}

void lock_memory(void)
{
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        printf("mlockall failed\n");
    }
}

void stack_prefault(void)
{
    unsigned char dummy[MAX_SAFE_STACK];
    memset(dummy, 0, MAX_SAFE_STACK);
}

void signal_handler(int sig)
{
    (void)sig;
    printf("\nReleasing master...\n");
    ecrt_release_master(master);
    pid_t pid = getpid();
    kill(pid, SIGKILL);
}
