#pragma once

/* --- 调试开关 (按需启用) --- */
/* #define MEASURE_PERF */
/* #define MEASURE_TIMING */
/* #define SET_CPU_AFFINITY */

// #define MEASURE_TIMING
// #define MEASURE_PERF

#define DC

#ifdef DC
#define SYNC_REF_TO_MASTER
#define CONFIG_DC
#endif
#define CONFIG_PDOS
#define SHOW_PARAM

#define NSEC_PER_SEC (1000000000L)
#define FREQUENCY 1000
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY) /* 运动循环周期, 1ms */

#ifdef CONFIG_DC
#define SHIFT0 (PERIOD_NS / 2) /* SYNC0 事件在周期中点触发 */
#endif

#define TIMESPEC2NS(T) \
        ((uint64_t)(T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)

#define EC_NEWTIMEVAL2NANO(TV) \
        (((TV).tv_sec - 946684800ULL) * 1000000000ULL + (TV).tv_nsec)

#define ENCODER_RES 524287        /* 电机一圈对应的编码器增量 (2^19-1) */
#define MAX_SAFE_STACK (8 * 1024) /* 安全栈大小 */

#define SHIFT0 (PERIOD_NS / 2)
