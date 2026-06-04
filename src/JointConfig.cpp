#include <erct.h>

/* --- PDO 条目 (RxPDO + TxPDO) --- */
ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    /* RxPDO (主站 → 从站) */
    {0x6040, 0x00, 16}, /* Controlword        */
    {0x607a, 0x00, 32}, /* Target Position    */
    {0x60FF, 0x00, 32}, /* Target Velocity    */
    {0x6071, 0x00, 16}, /* Target Torque      */
    {0x60B2, 0x00, 16}, /* Torque Offset      */
    /* TxPDO (从站 → 主站) */
    {0x6041, 0x00, 16}, /* Statusword         */
    {0x6064, 0x00, 32}, /* Position Actual    */
    {0x606c, 0x00, 32}, /* Velocity Actual    */
    {0x6077, 0x00, 16}, /* Torque Actual      */
};

/* --- PDO 描述 --- */
ec_pdo_info_t slave_0_pdos[] = {
    {0x1600, 5, slave_0_pdo_entries + 0}, /* 第2 RxPDO Mapping */
    {0x1a00, 4, slave_0_pdo_entries + 5}, /* 第2 TxPDO Mapping */
};

/* --- 同步管理器配置 --- */
ec_sync_info_t slave_0_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},            /* SM0: 保留 */
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},             /* SM1: 保留 */
    {2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_ENABLE}, /* SM2: RxPDO */
    {3, EC_DIR_INPUT, 1, slave_0_pdos + 1, EC_WD_DISABLE}, /* SM3: TxPDO */
    {0xFF}                                                 /* 终止 */
};
