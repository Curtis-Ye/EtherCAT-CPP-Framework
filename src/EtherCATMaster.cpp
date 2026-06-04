#include "EtherCATMaster.h"

bool EtherCATMaster::init()
{
        master_ = ecrt_request_master(0);
        return master_ != nullptr;
}

bool EtherCATMaster::activate()
{
        if (ecrt_master_activate(master_))
                return false;
        return true;
}

void EtherCATMaster::receive()
{
        ecrt_master_receive(master_);
}

void EtherCATMaster::send()
{
        ecrt_master_send(master_);
}

void EtherCATMaster::createDomain()
{
        domain_ = ecrt_master_create_domain(master_);
}

void EtherCATMaster::processDomain()
{
        ecrt_domain_process(domain_);
}

void EtherCATMaster::queueDomain()
{
        ecrt_domain_queue(domain_);
}

bool EtherCATMaster::getDomainData()
{
        domain_pd_ = ecrt_domain_data(domain_);
        if (!domain_pd_)
                return false;
        return true;
}

bool EtherCATMaster::regPDO2domain(const std::vector<ec_pdo_entry_reg_t> &domain_regs)
{
        if (ecrt_domain_reg_pdo_entry_list(domain_, domain_regs.data()))
                return false;
        return true;
}

void EtherCATMaster::syncDC()
{
#ifdef SYNC_REF_TO_MASTER
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        ecrt_master_application_time(master_, TIMESPEC2NS(time));
        /* 参考从站时钟同步到主站(PC)时间 */
        ecrt_master_sync_reference_clock(master_);
        /* 其他从站时钟同步到参考时钟 */
        ecrt_master_sync_slave_clocks(master_);
#endif
        (void)master_; /* 未定义 SYNC_REF_TO_MASTER 时消除警告 */
}

uint8_t *EtherCATMaster::getDomainPD()
{
        return domain_pd_;
}

ec_master_t *EtherCATMaster::getMaster_()
{
        return master_;
}

void EtherCATMaster::setMasterTime()
{
        struct timespec masterInitTime;
        clock_gettime(CLOCK_MONOTONIC, &masterInitTime);
        ecrt_master_application_time(master_, TIMESPEC2NS(masterInitTime));
}
