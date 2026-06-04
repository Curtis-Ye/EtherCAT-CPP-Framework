#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <iostream>
#include "EtherCATMaster.h"
#include "config.h"

class RealtimeManager
{
public:
        RealtimeManager(EtherCATMaster &master);
        void set_cpu_affinity(int cpu);
        void set_realtime_priority();
        void lock_memory();
        void stack_prefault();
        void timespec_add(struct timespec *result,
                          struct timespec *time1,
                          struct timespec *time2);
        void timespec_sub(struct timespec *result,
                          struct timespec *time1,
                          struct timespec *time2);
        void nanoSleep(struct timespec *wakeupTime);

private:
        EtherCATMaster &ethercat_;
};
