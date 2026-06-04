#include "RealtimeManager.h"

RealtimeManager::RealtimeManager(EtherCATMaster &master)
    : ethercat_(master)
{
}

void RealtimeManager::set_cpu_affinity(int cpu)
{
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpu, &set);

        if (sched_setaffinity(0, sizeof(set), &set))
        {
                std::cout << "Setting CPU affinity failed!" << std::endl;
        }
}

void RealtimeManager::set_realtime_priority()
{
        struct sched_param param = {};
        param.sched_priority = sched_get_priority_max(SCHED_FIFO);
        // printf("Using priority %i.\n", param.sched_priority);
        std::cout << "Using priority:" << param.sched_priority << std::endl;
        if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
        {
                perror("sched_setscheduler failed\n");
        }
}

void RealtimeManager::lock_memory()
{
        if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
        {
                std::cout << "mlockall failed" << std::endl;
        }
}

void RealtimeManager::stack_prefault()
{
        unsigned char dummy[MAX_SAFE_STACK];
        memset(dummy, 0, MAX_SAFE_STACK);
}

void RealtimeManager::timespec_add(struct timespec *result,
                                   struct timespec *time1,
                                   struct timespec *time2)
{
        if ((time1->tv_nsec + time2->tv_nsec) >= NSEC_PER_SEC)
        {
                result->tv_sec = time1->tv_sec + time2->tv_sec + 1;
                result->tv_nsec = time1->tv_nsec + time2->tv_nsec - NSEC_PER_SEC;
        }
        else
        {
                result->tv_sec = time1->tv_sec + time2->tv_sec;
                result->tv_nsec = time1->tv_nsec + time2->tv_nsec;
        }
}

void RealtimeManager::timespec_sub(struct timespec *result,
                                   struct timespec *time1,
                                   struct timespec *time2)
{
        if ((time1->tv_nsec - time2->tv_nsec) < 0)
        {
                result->tv_sec = time1->tv_sec - time2->tv_sec - 1;
                result->tv_nsec = NSEC_PER_SEC - (time2->tv_nsec - time1->tv_nsec);
        }
        else
        {
                result->tv_sec = time1->tv_sec - time2->tv_sec;
                result->tv_nsec = time1->tv_nsec - time2->tv_nsec;
        }
}

void RealtimeManager::nanoSleep(struct timespec *wakeupTime)
{
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, wakeupTime, NULL);
}
