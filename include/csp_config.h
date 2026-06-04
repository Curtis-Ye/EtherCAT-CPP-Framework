#pragma once

#include <stdint.h>
#include <time.h>
#include <ecrt.h>

/* ============================================================
 * csp_config.h — 通用编译常量 & 共享宏定义
 *
 * 所有模块通过包含本文件获得统一的编译配置。
 * 修改下方的 #define 即可切换功能，也可通过编译器 -D 选项覆盖
 * ============================================================ */

/* ======== 用户配置 (按需注释/取消注释) ======== */
#ifndef CONFIG_PDOS
#define CONFIG_PDOS    /* 使能 PDO 配置 */
#endif
#ifndef DC
#define DC             /* 使能分布式时钟 */
#endif

/* ======== 衍生配置 (由上述 flag 自动推导) ======== */
#ifdef DC
#define SYNC_REF_TO_MASTER
#define CONFIG_DC
#endif

/* --- 周期时间常量 --- */
#define NSEC_PER_SEC (1000000000L)
#define FREQUENCY 1000
#define PERIOD_NS   (NSEC_PER_SEC / FREQUENCY) /* 运动循环周期, 1ms */

#ifdef CONFIG_DC
#define SHIFT0 (PERIOD_NS / 2) /* SYNC0 事件在周期中点触发 */
#endif

/* --- 时间转换宏 --- */
#define TIMESPEC2NS(T) \
    ((uint64_t)(T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)

#define EC_NEWTIMEVAL2NANO(TV) \
    (((TV).tv_sec - 946684800ULL) * 1000000000ULL + (TV).tv_nsec)

/* --- 硬件参数 --- */
#define ENCODER_RES   524287      /* 电机一圈对应的编码器增量 (2^19-1) */
#define MAX_SAFE_STACK (8 * 1024) /* 安全栈大小 */

/* --- 全局主站指针，各模块通过 extern 访问 --- */
extern ec_master_t *master;

/* --- 周期间隔（纳秒）--- */
extern uint32_t interval_;
