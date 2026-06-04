#pragma once
#include <ecrt.h>
#include <vector>
#include <time.h>
#include "config.h"

class EtherCATMaster
{
public:
        bool init();
        bool activate();
        void receive();
        void send();
        void createDomain();
        void processDomain();
        void queueDomain();
        bool getDomainData();
        bool regPDO2domain(const std::vector<ec_pdo_entry_reg_t> &domain_regs);
        void syncDC();
        uint8_t *getDomainPD();
        ec_master_t *getMaster_();
        void setMasterTime();

private:
        uint8_t *domain_pd_;
        ec_master_t *master_;
        ec_domain_t *domain_;
};
