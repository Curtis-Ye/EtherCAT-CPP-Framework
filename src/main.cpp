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

// ec_master_t *master;
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

    // master = ecat_master_request();
    // if (!master)
    //     return -1;
    if (master0.init() == false)
    {
        return -1;
    }

    // initDrive(master, 0, CST); // 初始化关节模组
    joint1.init(CST);

    // ec_slave_config_t *sc = ecat_slave_config(master);
    // if (!sc)
    //     return -1;
    if (joint1.jointConfig() == false)
        return -1;

    // if (ecat_config_pdos(sc))
    //     return -1;
    if (joint1.PDOConfig() == false)
        return -1;

    // ec_domain_t *domain = ecat_create_domain(master);
    master0.createDomain();

    // printf("Activating master...\n");
    std::cout << "Activating master..." << std::endl;

    // if (ecat_reg_pdo_entries(domain))
    //     return -1;
    joint1.appendPdoRegs(regs);
    master0.regPDO2domain(regs);

    // ecat_config_dc(sc);
    joint1.dcConfig();

#ifdef SYNC_REF_TO_MASTER
    // {
    //     struct timespec masterInitTime;
    //     clock_gettime(CLOCK_MONOTONIC, &masterInitTime);
    //     ecrt_master_application_time(master, TIMESPEC2NS(masterInitTime));
    // }
    master0.setMasterTime();
#endif

    // if (ecat_activate(master))
    //     return -1;
    if (master0.activate() == false)
        return -1;

    // uint8_t *domain_pd = ecat_domain_data(domain);
    // if (!domain_pd)
    //     return -1;
    if (master0.getDomainData() == false)
        return -1;

    struct timespec cycleTime = {0, PERIOD_NS};

    /* ---- 3. 等待所有从站进入 OP 状态 ---- */
    {
        struct timespec wakeupTime;
        clock_gettime(CLOCK_MONOTONIC, &wakeupTime);

        // ec_slave_config_state_t slaveState;

        while (1)
        {
            // timespec_add(&wakeupTime, &wakeupTime, &cycleTime);
            manager0.timespec_add(&wakeupTime, &wakeupTime, &cycleTime);
            // clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime, NULL);
            manager0.nanoSleep(&wakeupTime);

            // ecat_cycle_receive(master);
            // ecrt_slave_config_state(sc, &slaveState);
            master0.receive();
            joint1.getSlaveState();

            if (joint1.slaveState_.operational)
            {
                std::cout << "All slaves have reached OP state" << std::endl;
                break;
            }

            // ecrt_domain_queue(domain);
            master0.queueDomain();
            // ecat_sync_dc(master);
            master0.syncDC();
            // ecrt_master_send(master);
            master0.send();
        }
    }

    /* ---- 4. 运动控制主循环 ---- */
    struct timespec wakeupTime, sleepTime;

#ifdef MEASURE_PERF
    uint32_t t_cur, t_prev;
#endif

    sleepTime = cycleTime;
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
        // timespec_add(&wakeupTime, &wakeupTime, &sleepTime);
        manager0.timespec_add(&wakeupTime, &wakeupTime, &sleepTime);
        // clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime, NULL);
        manager0.nanoSleep(&wakeupTime);
        /* 接收帧 & 处理域数据 */
        // ecat_cycle_receive(master);
        // ecat_cycle_process(domain);
        master0.receive();
        master0.processDomain();

#ifdef MEASURE_PERF
        ecrt_master_reference_clock_time(master, &t_cur);
#endif

        /* --- CST 驱动状态机 --- */
        // uint16_t state = driveStateMachine(statusWord, domain_pd,
        //                                    offset_controlword,
        //                                    offset_target_torque,
        //                                    &targetTorque);

        joint1.enable();
        // printf("targetTorque=%d actTorque=%d state=0x%04x\n",
        //        targetTorque, actTorque, state);
        joint1.setTargetTor(40);
        /* 队列 & 发送 */
        // ecat_cycle_queue_and_send(master, domain);
        master0.queueDomain();
        master0.send();

        /* DC 时钟同步 */
        // ecat_sync_dc(master);
        master0.syncDC();

#ifdef MEASURE_PERF
        printf("\nTimestamp diff: %" PRIu32 " ns\n\n", t_cur - t_prev);
        t_prev = t_cur;
#endif
    }

    return 0;
}
