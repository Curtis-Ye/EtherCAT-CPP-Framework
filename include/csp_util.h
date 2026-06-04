#pragma once

#include <time.h>

/* ============================================================
 * csp_util.h — 系统 & 时间工具声明
 *
 * 提供实时系统配置（CPU亲和性、调度策略、内存锁定）
 * 以及高精度时间运算工具，任何实时 EtherCAT 项目通用
 * ============================================================ */

/* --- 时间运算 --- */

/* result = time1 + time2 */
void timespec_add(struct timespec *result,
                  struct timespec *time1,
                  struct timespec *time2);

/* result = time1 - time2 */
void timespec_sub(struct timespec *result,
                  struct timespec *time1,
                  struct timespec *time2);

/* --- 系统配置 --- */

/* 将进程绑定到指定 CPU 核心 */
void set_cpu_affinity(int cpu);

/* 设置实时调度策略 (SCHED_FIFO), 使用最高优先级 */
void set_realtime_priority(void);

/* 锁定进程内存，防止 page fault 和 swap */
void lock_memory(void);

/* 预分配栈空间，配合 mlockall(MCL_FUTURE) 使用 */
void stack_prefault(void);

/* SIGINT 信号处理：释放主站并退出 */
void signal_handler(int sig);
