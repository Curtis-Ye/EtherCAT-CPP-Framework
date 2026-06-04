#include "Joint.h"
#include "config.h"
#include "JointConfig.h"

uint8_t control_word = 0x0080;

Joint::Joint(EtherCATMaster &master, uint16_t Pos)
    : ethercat_(master), position_(Pos)
{
}

void Joint::setMode(uint8_t mode)
{
        mode_ = mode;
}

bool Joint::init()
{
        if (ecrt_master_sdo_download(ethercat_.getMaster_(), position_, 0x6060, 0x00, &mode_, sizeof(mode_), NULL))
        {
                std::cout << "OD write unsuccessful" << std::endl;
                return false;
        }
        if (ecrt_master_sdo_download(ethercat_.getMaster_(), position_, 0x6040, 0x00, &control_word, sizeof(control_word), NULL))
        {
                std::cout << "OD write unsuccessful" << std::endl;
                return false;
        }
        return true;
}

bool Joint::jointConfig()
{
        ec_slave_config_t *temp = ecrt_master_slave_config(ethercat_.getMaster_(), alias_, position_, vendor_id_, product_code_);
        sc_ = temp;
        if (!sc_)
        {
                std::cout << "Failed to get slave configuration" << std::endl;
                return false;
        }
                return true;
}

bool Joint::PDOConfig()
{
#ifdef CONFIG_PDOS
        if (ecrt_slave_config_pdos(sc_, EC_END, slave_0_syncs))
        {
                std::cout << "Failed to configure slave PDOs" << std::endl;
                return false;
        }
        return true;
#endif
        (void)sc_; /* 未定义 CONFIG_PDOS 时消除警告 */
        return false;
}

void Joint::dcConfig()
{
        ecrt_slave_config_dc(sc_, 0x0300, PERIOD_NS, SHIFT0, 0, 0);
}

void Joint::getSlaveState()
{
        ecrt_slave_config_state(sc_, &slaveState_);
}

void Joint::appendPdoRegs(
    std::vector<ec_pdo_entry_reg_t> &regs)
{
        regs.push_back({alias_,
                        position_,
                        vendor_id_,
                        product_code_,
                        0x6040,
                        0x00,
                        &offset_controlword});
        regs.push_back({alias_,
                        position_,
                        vendor_id_,
                        product_code_,
                        0x607a,
                        0x00,
                        &offset_target_position});
        regs.push_back({alias_,
                        position_,
                        vendor_id_,
                        product_code_,
                        0x60FF,
                        0x00,
                        &offset_target_velocity});
        regs.push_back({alias_,
                        position_,
                        vendor_id_,
                        product_code_,
                        0x6071,
                        0x00,
                        &offset_target_torque});
        regs.push_back({alias_,
                        position_,
                        vendor_id_,
                        product_code_,
                        0x60B2,
                        0x00,
                        &offset_torque_offset});
        regs.push_back({alias_,
                        position_,
                        vendor_id_,
                        product_code_,
                        0x6041,
                        0x00,
                        &offset_statusword});
        regs.push_back({alias_,
                        position_,
                        vendor_id_,
                        product_code_,
                        0x6064,
                        0x00,
                        &offset_actual_position});
        regs.push_back({alias_,
                        position_,
                        vendor_id_,
                        product_code_,
                        0x606c,
                        0x00,
                        &offset_actual_velocity});
        regs.push_back({alias_,
                        position_,
                        vendor_id_,
                        product_code_,
                        0x6077,
                        0x00,
                        &offset_actual_torque});
        regs.push_back({}); /* 终止 */
}

void Joint::getActualPos()
{
        actualPos = EC_READ_S32(ethercat_.getDomainPD() + offset_actual_position);
}

void Joint::getActualVel()
{
        actualVel = EC_READ_S32(ethercat_.getDomainPD() + offset_actual_velocity);
}

void Joint::getActualTor()
{
        actualTorque = EC_READ_S16(ethercat_.getDomainPD() + offset_actual_torque);
}

void Joint::getStatusWord()
{
        status_word_ = EC_READ_U16(ethercat_.getDomainPD() + offset_statusword);
}

bool Joint::isFault()
{
        return (status_word_ & 0x0008) == 0x0008;
}

bool Joint::isSwitchOnDisabled()
{
        return (status_word_ & 0x0040) == 0x0040;
}

bool Joint::isReadyToSwitchOn()
{
        return (status_word_ & 0x0021) == 0x0021;
}

bool Joint::isSwitchedOn()
{
        return (status_word_ & 0x0023) == 0x0023;
}

bool Joint::isOperationEnabled()
{
        return (status_word_ & 0x0027) == 0x0027;
}

void Joint::setTargetPos(int32_t position)
{
        EC_WRITE_S32(ethercat_.getDomainPD() + offset_target_position, position);
#ifdef SHOW_PARAM
        std::cout << "TargetPos:" << position << std::endl;
#endif
}

void Joint::setTargetVel(int32_t velocity)
{
        EC_WRITE_S32(ethercat_.getDomainPD() + offset_target_velocity, velocity);
#ifdef SHOW_PARAM
        std::cout << "TargetVel:" << velocity << std::endl;
#endif
}

void Joint::setTargetTor(int32_t torque)
{
        EC_WRITE_S16(ethercat_.getDomainPD() + offset_target_torque, torque);
#ifdef SHOW_PARAM
        std::cout << "TargetTor:" << torque << std::endl;
#endif
}

void Joint::enable()
{
        getStatusWord();
        if (isFault())
        {
                control_word_ = 0x0080;
                std::cout << "Fault state, sending reset command" << std::endl;
        }
        else if (isOperationEnabled())       // 先检查最具体的状态
        {
                control_word_ = 0x000F;
                std::cout << "Operating..." << std::endl;
        }
        else if (isSwitchedOn())
        {
                control_word_ = 0x000F;
                setTargetTor(0x00);
                std::cout << "Switched on, sending enable operation command" << std::endl;
        }
        else if (isReadyToSwitchOn())
        {
                control_word_ = 0x0007;
                std::cout << "Ready to switch on, sending switch on command" << std::endl;
        }
        else if (isSwitchOnDisabled())
        {
                control_word_ = 0x0006;
                std::cout << "Switch on disabled, sending shutdown command" << std::endl;
        }
        else
        {
                control_word_ = 0x0006;
                std::cout << "Unknown state, trying shutdown" << std::endl;
        }

        EC_WRITE_U16(ethercat_.getDomainPD() + offset_controlword, control_word_);
}
