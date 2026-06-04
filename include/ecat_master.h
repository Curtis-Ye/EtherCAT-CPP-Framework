#pragma once

#include <ecrt.h>

/* ============================================================
 * ecat_master.h — EtherCAT 主站操作声明
 *
 * 封装 IGH EtherCAT 主站 API，提供高层次的初始化、
 * 配置、激活和循环数据交换接口。
 * 本模块与具体控制模式无关，所有 EtherCAT 项目通用
 * ============================================================ */

/* 请求主站实例 (master index 0) */
ec_master_t *ecat_master_request(void);

/* 创建从站配置对象 */
ec_slave_config_t *ecat_slave_config(ec_master_t *master);

/* 配置从站 PDO */
int ecat_config_pdos(ec_slave_config_t *sc);

/* 创建过程数据域 */
ec_domain_t *ecat_create_domain(ec_master_t *master);

/* 注册 PDO 条目到域 */
int ecat_reg_pdo_entries(ec_domain_t *domain);

/* 配置分布式时钟 (DC) */
void ecat_config_dc(ec_slave_config_t *sc);

/* DC 同步：参考时钟 → 主站，从站时钟 → 参考时钟 */
void ecat_sync_dc(ec_master_t *master);

/* 激活主站 */
int ecat_activate(ec_master_t *master);

/* 获取域过程数据指针 */
uint8_t *ecat_domain_data(ec_domain_t *domain);

/* --- 循环数据交换 --- */

/* 接收帧 */
void ecat_cycle_receive(ec_master_t *master);

/* 处理域数据 */
void ecat_cycle_process(ec_domain_t *domain);

/* 队列 + 发送 */
void ecat_cycle_queue_and_send(ec_master_t *master,
                               ec_domain_t *domain);
