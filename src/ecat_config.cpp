#include "ecat_config.h"

/* ============================================================
 * ecat_config.cpp — EtherCAT 从站硬件配置数据定义
 *
 * 定义从站身份、PDO 映射、同步管理器和域注册的全部数据。
 * 更换从站/电机型号时，主要修改此文件中的数据表。
 * ============================================================ */

/* --- 从站身份 --- */
uint16_t alias       = 0;           /* 从站别名        */
uint16_t position0   = 0;           /* 从站在总线上的位置 */
uint32_t vendor_id   = 0x5a65726f;  /* 零售商 ID       */
uint32_t product_code = 0x00029252; /* 制造商 ID       */

/* --- PDO 条目 (RxPDO + TxPDO) --- */
ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    /* RxPDO (主站 → 从站) */
    {0x6040, 0x00, 16},  /* Controlword        */
    {0x607a, 0x00, 32},  /* Target Position    */
    {0x60FF, 0x00, 32},  /* Target Velocity    */
    {0x6071, 0x00, 16},  /* Target Torque      */
    {0x60B2, 0x00, 16},  /* Torque Offset      */
    /* TxPDO (从站 → 主站) */
    {0x6041, 0x00, 16},  /* Statusword         */
    {0x6064, 0x00, 32},  /* Position Actual    */
    {0x606c, 0x00, 32},  /* Velocity Actual    */
    {0x6077, 0x00, 16},  /* Torque Actual      */
};

/* --- PDO 描述 --- */
ec_pdo_info_t slave_0_pdos[] = {
    {0x1600, 5, slave_0_pdo_entries + 0},  /* 第2 RxPDO Mapping */
    {0x1a00, 4, slave_0_pdo_entries + 5},  /* 第2 TxPDO Mapping */
};

/* --- 同步管理器配置 --- */
ec_sync_info_t slave_0_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},               /* SM0: 保留 */
    {1, EC_DIR_INPUT,  0, NULL, EC_WD_DISABLE},               /* SM1: 保留 */
    {2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_ENABLE},    /* SM2: RxPDO */
    {3, EC_DIR_INPUT,  1, slave_0_pdos + 1, EC_WD_DISABLE},   /* SM3: TxPDO */
    {0xFF}                                                      /* 终止 */
};

/* --- PDO 偏移变量（域注册后填充）--- */
uint offset_controlword;
uint offset_statusword;
uint offset_target_position;
uint offset_actual_position;
uint offset_target_velocity;
uint offset_actual_velocity;
uint offset_target_torque;
uint offset_actual_torque;
uint offset_torque_offset;

/* --- 域注册表（将对象字典映射到偏移变量）--- */
ec_pdo_entry_reg_t domain1_regs[] = {
    {0, 0, vendor_id, product_code, 0x6040, 0x00, &offset_controlword},
    {0, 0, vendor_id, product_code, 0x607a, 0x00, &offset_target_position},
    {0, 0, vendor_id, product_code, 0x60FF, 0x00, &offset_target_velocity},
    {0, 0, vendor_id, product_code, 0x6071, 0x00, &offset_target_torque},

    {0, 0, vendor_id, product_code, 0x60B2, 0x00, &offset_torque_offset},

    {0, 0, vendor_id, product_code, 0x6041, 0x00, &offset_statusword},
    {0, 0, vendor_id, product_code, 0x6064, 0x00, &offset_actual_position},
    {0, 0, vendor_id, product_code, 0x606c, 0x00, &offset_actual_velocity},
    {0, 0, vendor_id, product_code, 0x6077, 0x00, &offset_actual_torque},
    {}  /* 终止 */
};

/* --- 驱动状态解析 --- */
uint16_t getDriveState(uint16_t statusWord)
{
    return statusWord & 0x006F;  /* 低7位为 CiA 402 状态 */
}

/* --- 从站配置封装 --- */
ec_slave_config_t *slaveConfig(ec_master_t *master,
                               uint16_t alias,
                               uint16_t position,
                               uint32_t vendor_id,
                               uint32_t product_code)
{
    return ecrt_master_slave_config(master, alias, position,
                                    vendor_id, product_code);
}
