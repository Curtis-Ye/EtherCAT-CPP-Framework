#include <unistd.h>
#include <vector>
#include <iostream>
#include "EtherCATMaster.h"

/* --- CiA 402 操作模式 --- */
#define PP 0x01  /* Profile Position        */
#define PV 0x03  /* Profile Velocity        */
#define PT 0x04  /* Profile Torque          */
#define CSP 0x08 /* Cyclic Sync Position    */
#define CSV 0x09 /* Cyclic Sync Velocity    */
#define CST 0x0A /* Cyclic Sync Torque      */

class Joint
{
public:
    uint16_t control_word_ = 0;
    uint16_t status_word_;
    int32_t targetPos;
    int32_t actualPos;
    int32_t targetVel;
    int32_t actualVel;
    int32_t targetTorque;
    int32_t actualTorque;

    ec_slave_config_state_t slaveState_;

    Joint(EtherCATMaster &master, uint16_t Pos);
    bool init(uint8_t mode);
    bool jointConfig();
    bool PDOConfig();
    void dcConfig();
    void getSlaveState();
    void appendPdoRegs(
        std::vector<ec_pdo_entry_reg_t> &regs);
    void getActualPos();
    void getActualVel();
    void getActualTor();
    void getStatusWord();
    bool isFault();
    bool isSwitchOnDisabled();
    bool isReadyToSwitchOn();
    bool isSwitchedOn();
    bool isOperationEnabled();
    void enable();
    void setTargetPos(int32_t position);
    void setTargetVel(int32_t velocity);
    void setTargetTor(int32_t torque);

private:
    EtherCATMaster &ethercat_;
    ec_slave_config_t *sc_;

    uint16_t alias_ = 0;                 /* 从站别名        */
    uint16_t position_ = 0;              /* 从站在总线上的位置 */
    uint32_t vendor_id_ = 0x5a65726f;    /* 零售商 ID       */
    uint32_t product_code_ = 0x00029252; /* 制造商 ID       */

    uint offset_controlword;
    uint offset_statusword;
    uint offset_target_position;
    uint offset_actual_position;
    uint offset_target_velocity;
    uint offset_actual_velocity;
    uint offset_target_torque;
    uint offset_actual_torque;
    uint offset_torque_offset;
};
