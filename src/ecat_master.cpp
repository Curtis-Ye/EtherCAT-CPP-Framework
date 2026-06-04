#include "csp_config.h"
#include "ecat_config.h"
#include "ecat_master.h"

#include <stdio.h>
#include <time.h>

/* ============================================================
 * ecat_master.cpp — EtherCAT 主站操作实现
 *
 * 封装 IGH EtherCAT 主站的初始化、配置、激活和循环收发，
 * 对外暴露简洁的高层接口
 * ============================================================ */

ec_master_t *ecat_master_request(void)
{
    ec_master_t *m = ecrt_request_master(0);
    if (!m)
        printf("Requesting master failed\n");
    return m;
}

ec_slave_config_t *ecat_slave_config(ec_master_t *master)
{
    ec_slave_config_t *sc = slaveConfig(master, alias, position0,
                                        vendor_id, product_code);
    if (!sc)
        printf("Failed to get slave configuration\n");
    return sc;
}

int ecat_config_pdos(ec_slave_config_t *sc)
{
#ifdef CONFIG_PDOS
    if (ecrt_slave_config_pdos(sc, EC_END, slave_0_syncs)) {
        printf("Failed to configure slave PDOs\n");
        return -1;
    }
#endif
    (void)sc;  /* 未定义 CONFIG_PDOS 时消除警告 */
    return 0;
}

ec_domain_t *ecat_create_domain(ec_master_t *master)
{
    return ecrt_master_create_domain(master);
}

int ecat_reg_pdo_entries(ec_domain_t *domain)
{
    if (ecrt_domain_reg_pdo_entry_list(domain, domain1_regs)) {
        printf("PDO entry registration failed\n");
        return -1;
    }
    return 0;
}

void ecat_config_dc(ec_slave_config_t *sc)
{
#ifdef CONFIG_DC
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    ecrt_master_application_time(master, EC_NEWTIMEVAL2NANO(t));
    /* 使能 SYNC0, 周期 = PERIOD_NS, 偏移 = SHIFT0, 不使用 SYNC1 */
    ecrt_slave_config_dc(sc, 0x0300, PERIOD_NS, SHIFT0, 0, 0);
#endif
    (void)sc;  /* 未定义 CONFIG_DC 时消除警告 */
}

void ecat_sync_dc(ec_master_t *master)
{
#ifdef SYNC_REF_TO_MASTER
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    ecrt_master_application_time(master, TIMESPEC2NS(time));
    /* 参考从站时钟同步到主站(PC)时间 */
    ecrt_master_sync_reference_clock(master);
    /* 其他从站时钟同步到参考时钟 */
    ecrt_master_sync_slave_clocks(master);
#endif
    (void)master;  /* 未定义 SYNC_REF_TO_MASTER 时消除警告 */
}

int ecat_activate(ec_master_t *master)
{
    printf("Activating master...\n");
    if (ecrt_master_activate(master)) {
        printf("Master activation failed\n");
        return -1;
    }
    return 0;
}

uint8_t *ecat_domain_data(ec_domain_t *domain)
{
    return ecrt_domain_data(domain);
}

/* --- 循环数据交换 --- */

void ecat_cycle_receive(ec_master_t *master)
{
    ecrt_master_receive(master);
}

void ecat_cycle_process(ec_domain_t *domain)
{
    ecrt_domain_process(domain);
}

void ecat_cycle_queue_and_send(ec_master_t *master, ec_domain_t *domain)
{
    ecrt_domain_queue(domain);
    ecrt_master_send(master);
}
