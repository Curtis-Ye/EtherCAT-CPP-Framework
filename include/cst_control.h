#pragma once

#include <stdint.h>
#include <ecrt.h>

#define Kp 0.01
#define Kd 0.001

/* ============================================================
 * csp_control.h — 应用层：CSP 模式驱动控制声明
 *
 * 实现 CiA 402 驱动状态机和 CSP（循环同步位置）控制逻辑。
 * 如需其他模式（CSV/CST/PP/PV/PT），可创建对应的头文件替换本模块
 * ============================================================ */

/* --- CiA 402 操作模式 --- */
#define PP 0x01  /* Profile Position        */
#define PV 0x03  /* Profile Velocity        */
#define PT 0x04  /* Profile Torque          */
#define CSP 0x08 /* Cyclic Sync Position    */
#define CSV 0x09 /* Cyclic Sync Velocity    */
#define CST 0x0A /* Cyclic Sync Torque      */

/* --- CiA 402 驱动状态（statusword 掩码后）--- */
#define STATE_FAULT 0x0008
#define STATE_SWITCH_ON_DISABLED 0x0040
#define STATE_READY_TO_SWITCH_ON 0x0021
#define STATE_SWITCHED_ON 0x0023
#define STATE_OPERATION_ENABLED 0x0027

/* --- CiA 402 控制字命令 --- */
#define CONTROL_WORD_SHUTDOWN 0x0006
#define CONTROL_WORD_SWITCH_ON 0x0007
#define CONTROL_WORD_ENABLE_OPERATION 0x000F
#define CONTROL_WORD_FAULT_RESET 0x0080

/* --- 驱动 SDO 写入 --- */
void ODwrite(ec_master_t *master,
             uint16_t slavePos,
             uint16_t index,
             uint8_t subIndex,
             uint8_t objectValue);

/* 初始化驱动：设置操作模式 + 复位报警 */
void initDrive(ec_master_t *master,
               uint16_t slavePos,
               uint8_t mode);

/* CSP 驱动状态机
 * 根据当前状态返回应写入的控制字，并在 Operation Enabled 时更新目标位置
 * 返回: 当前驱动状态 */
uint16_t driveStateMachine(uint16_t statusWord,
                           uint8_t *domain_pd,
                           unsigned int offset_cw,
                           unsigned int offset_target,
                           int32_t *targetTorque);
