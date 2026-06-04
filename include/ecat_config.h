#pragma once

#include <ecrt.h>

/* ============================================================
 * ecat_config.h — EtherCAT 从站硬件配置声明
 *
 * 定义从站身份信息、PDO 映射表、同步管理器配置、
 * 域注册等纯硬件数据。本模块与具体控制模式无关，
 * 更换从站硬件时只需修改对应的 .cpp 文件
 * ============================================================ */

/* --- 从站身份 --- */
extern uint16_t alias;
extern uint16_t position0;
extern uint32_t vendor_id;
extern uint32_t product_code;

/* --- PDO 条目定义 --- */
extern ec_pdo_entry_info_t slave_0_pdo_entries[];

/* --- PDO 信息 --- */
extern ec_pdo_info_t slave_0_pdos[];

/* --- 同步管理器配置 --- */
extern ec_sync_info_t slave_0_syncs[];

/* --- 域注册：PDO 偏移变量 --- */
extern uint offset_controlword;
extern uint offset_statusword;
extern uint offset_target_position;
extern uint offset_actual_position;
extern uint offset_target_velocity;
extern uint offset_actual_velocity;
extern uint offset_target_torque;
extern uint offset_actual_torque;
extern uint offset_torque_offset;

/* --- 域注册表 --- */
extern ec_pdo_entry_reg_t domain1_regs[];

/* --- 从站配置封装 --- */

/* 从状态字中提取驱动状态（低7位掩码） */
uint16_t getDriveState(uint16_t statusWord);

/* 创建从站配置对象 */
ec_slave_config_t *slaveConfig(ec_master_t *master,
                               uint16_t alias,
                               uint16_t position,
                               uint32_t vendor_id,
                               uint32_t product_code);
