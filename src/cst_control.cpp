#include "cst_control.h"
#include "ecat_config.h"

#include <stdio.h>
#include <ecrt.h>

/* ============================================================
 * cst_control.cpp — CST 模式驱动控制实现
 *
 * 包含 CiA 402 驱动状态机和 CSP 控制逻辑。
 * 替换为其他模式（CSV/CST/PP/PV/PT）时，创建对应文件即可
 * ============================================================ */

//模式选择：前馈力矩模式/虚拟弹簧模式(只启用其中一个) 
#define TORQUE_FEEDFORWARD
//#define STRING_MODE

static int32_t holdPos = 0;
static int32_t error = 0;
static int32_t actPos = 0;

void ODwrite(ec_master_t *master,
             uint16_t slavePos,
             uint16_t index,
             uint8_t subIndex,
             uint8_t objectValue)
{
        uint8_t retVal = ecrt_master_sdo_download(master, slavePos,
                                                  index, subIndex, &objectValue, sizeof(objectValue), NULL);
        if (retVal)
                printf("OD write unsuccessful\n");
}

void initDrive(ec_master_t *master, uint16_t slavePos, uint8_t mode)
{
        /* 设置操作模式 (0x6060) */
        ODwrite(master, slavePos, 0x6060, 0x00, mode);
        /* 复位报警 */
        ODwrite(master, slavePos, 0x6040, 0x00, 0x80);
}

uint16_t driveStateMachine(uint16_t statusWord,
                           uint8_t *domain_pd,
                           unsigned int offset_cw,
                           unsigned int offset_target,
                           int32_t *targetTorque)
{
        uint16_t state = getDriveState(statusWord);
        uint16_t cw = 0;
        int32_t actVel = 0;
        int32_t offsetTorque = 0;
        actVel = EC_READ_S32(domain_pd + offset_actual_velocity); 
        offsetTorque = EC_READ_S32(domain_pd + offset_torque_offset);
        actPos = EC_READ_S32(domain_pd + offset_actual_position); 

        printf("error = %d\n",error);
        printf("actVel = %d\n",actVel);
        printf("offsetTorque = %d\n",offsetTorque);

        switch (state)
        {
        case STATE_FAULT:
                cw = CONTROL_WORD_FAULT_RESET;
                printf("Fault state, sending reset command\n");
                break;

        case STATE_SWITCH_ON_DISABLED:
                cw = CONTROL_WORD_SHUTDOWN;
                printf("Switch on disabled, sending shutdown command\n");
                break;

        case STATE_READY_TO_SWITCH_ON:
                cw = CONTROL_WORD_SWITCH_ON;
                printf("Ready to switch on, sending switch on command\n");
                break;

        case STATE_SWITCHED_ON:
                cw = CONTROL_WORD_ENABLE_OPERATION;
                holdPos = actPos;
                EC_WRITE_S16(domain_pd + offset_target, 0x00);
                printf("Switched on, sending enable operation command\n");
                break;

        case STATE_OPERATION_ENABLED:
                cw = CONTROL_WORD_ENABLE_OPERATION;
                printf("Operating...\n");
                break;

        default:
                cw = CONTROL_WORD_SHUTDOWN;
                printf("Unknown state (0x%04x), trying shutdown\n", state);
                break;
        }

        /* 写入控制字 */
        EC_WRITE_U16(domain_pd + offset_cw, cw);
        printf("holdPos=%d actPos=%d\n",holdPos,actPos);

        /* 在 Operation Enabled 状态下周期更新目标转矩 */
        if (state == STATE_OPERATION_ENABLED)
        {
                #ifdef STRING_MODE
                        error = holdPos - actPos;
                        if(error > 0)
                        *targetTorque = Kp * error;
                        else if(error < 0)
                        *targetTorque = Kp * error;
                        else
                        *targetTorque = 0;
                        EC_WRITE_S16(domain_pd + offset_target, *targetTorque);
                #endif

                
                #ifdef TORQUE_FEEDFORWARD
                        EC_WRITE_S16(domain_pd + offset_target, 0);
                        if(actVel>0)
                        EC_WRITE_S16(domain_pd + offset_torque_offset, 22);
                        else if(actVel<0)
                        EC_WRITE_S16(domain_pd + offset_torque_offset, -22);
                        else
                        EC_WRITE_S16(domain_pd + offset_torque_offset, 0);
                #endif

        }

        return state;
}
