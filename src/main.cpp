/* ============================================================
 * main.cpp — EtherCAT 通用主站模板入口
 *
 * 核心配置 flag (在 include/csp_config.h 中定义):
 *   CONFIG_PDOS - 使能 PDO 配置
 *   DC          - 使能分布式时钟
 *
 * 调试 flag (按需取消注释):
 *   MEASURE_PERF    - 测量参考从站时钟时间戳差值
 *   MEASURE_TIMING  - 测量循环执行时间
 *   SET_CPU_AFFINITY - 绑定 CPU 核心
 *
 * 更换控制模式: 替换 cst_control 模块即可
 * ============================================================ */

#include <stdio.h>
#include <signal.h>
#include <inttypes.h>
#include "EtherCATMaster.h"
#include "Joint.h"
#include "RealtimeManager.h"
#include "config.h"

EtherCATMaster master0;
Joint joint1(master0, 0);
RealtimeManager manager0(master0);
std::vector<ec_pdo_entry_reg_t> regs;

void signal_handler(int sig)
{
        (void)sig;
        std::cout << "Releasing master..." << std::endl;
        ecrt_release_master(master0.getMaster_());
        pid_t pid = getpid();
        kill(pid, SIGKILL);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* ---- 1. 系统初始化 ---- */
#ifdef SET_CPU_AFFINITY
    manager0.set_cpu_affinity(4);
#endif
    manager0.set_realtime_priority();
    manager0.lock_memory();
    manager0.stack_prefault();
    signal(SIGINT, signal_handler);

    /* ---- 2. EtherCAT 主站初始化 ---- */
    if (master0.init() == false)
    {
        return -1;
    }

    joint1.setMode(CST);
    joint1.init();

    if (joint1.jointConfig() == false)
    {
        std::cout<<"jointConfig failed!"<<std::endl;
        return -1;
    }
        
    if (joint1.PDOConfig() == false)
    {
        std::cout<<"PDOConfig failed!"<<std::endl;
        return -1;
    }

    master0.createDomain();

    std::cout << "Activating master..." << std::endl;

    joint1.appendPdoRegs(regs);
    master0.regPDO2domain(regs);

    joint1.dcConfig();

#ifdef SYNC_REF_TO_MASTER
    master0.setMasterTime();
#endif

    if (master0.activate() == false)
        return -1;

    if (master0.getDomainData() == false)
        return -1;

    struct timespec cycleTime = {0, PERIOD_NS};

    /* ---- 3. 等待所有从站进入 OP 状态 ---- */
    {
        struct timespec wakeupTime;
        clock_gettime(CLOCK_MONOTONIC, &wakeupTime);

        while (1)
        {
            manager0.timespec_add(&wakeupTime, &wakeupTime, &cycleTime);
            manager0.nanoSleep(&wakeupTime);

            master0.receive();
            joint1.getSlaveState();

            if (joint1.slaveState_.operational)
            {
                std::cout << "All slaves have reached OP state" << std::endl;
                break;
            }

            master0.queueDomain();
            master0.syncDC();
            master0.send();
        }
    }

    /* ---- 4. 运动控制主循环 ---- */
struct timespec wakeupTime, sleepTime;

#ifdef MEASURE_PERF
    uint32_t t_cur, t_prev;
#endif

    sleepTime = cycleTime; //1ms
    clock_gettime(CLOCK_MONOTONIC, &wakeupTime);

    while (1)
    {
#ifdef MEASURE_TIMING
        struct timespec endTime, execTime;
        clock_gettime(CLOCK_MONOTONIC, &endTime);
        timespec_sub(&execTime, &endTime, &wakeupTime);
        printf("Execution time: %lu ns\n", execTime.tv_nsec);
#endif

        /* 精确周期睡眠 */
        manager0.timespec_add(&wakeupTime, &wakeupTime, &sleepTime);
        manager0.nanoSleep(&wakeupTime);
        /* 接收帧 & 处理域数据 */
        master0.receive();
        master0.processDomain();

#ifdef MEASURE_PERF
        ecrt_master_reference_clock_time(master, &t_cur);
#endif

        /* --- 驱动状态机 --- */
        joint1.enable();
        if(joint1.isOperationEnabled())
            joint1.setTargetTor(40);
        
        /* 队列 & 发送 */
        master0.queueDomain();
        master0.send();

        /* DC 时钟同步 */
        master0.syncDC();

#ifdef MEASURE_PERF
        printf("\nTimestamp diff: %" PRIu32 " ns\n\n", t_cur - t_prev);
        t_prev = t_cur;
#endif
    }

    return 0;
}
