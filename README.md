# EtherCAT-CPP-Framework

基于 [IGH EtherCAT Master](https://etherlab.org/en/ethercat/)（EtherLab）库的模块化 C++ 框架，用于 EtherCAT 机器人与运动控制应用。提供面向对象的 API，封装了 EtherCAT 主站管理、CiA 402 驱动控制以及 Linux 实时调度等功能。

## 功能特性

- **EtherCAT 主站抽象** — 对 IGH `ecrt_*` API 进行 C++ 封装，管理主站生命周期、域操作和过程数据 I/O。
- **CiA 402 驱动协议** — 完整的状态机实现（Fault → Switch On Disabled → Ready to Switch On → Switched On → Operation Enabled），支持多种运行模式。
- **运行模式** — 同时支持轮廓模式（Profile）和周期同步模式（Cyclic Sync）：
  - 轮廓位置模式（PP）、轮廓速度模式（PV）、轮廓转矩模式（PT）
  - 周期同步位置模式（CSP）、周期同步速度模式（CSV）、周期同步转矩模式（CST）
- **分布式时钟（DC）** — 基于 SYNC0 的时钟同步，可配置偏移时间，用于多轴协调控制。
- **实时配置** — SCHED_FIFO 优先级、CPU 核心绑定、内存锁定（`mlockall`）、栈预取，避免控制周期内发生缺页。
- **PDO 配置** — 灵活的 RxPDO/TxPDO 映射，支持控制字、状态字、位置、速度和转矩数据。

## 依赖环境

| 依赖 | 说明 |
|---|---|
| **IGH EtherCAT Master** | 安装于 `/usr/local/etherlab`，需要共享库 `libethercat.so` 和头文件 `ecrt.h`。 |
| **PREEMPT_RT 内核** | 推荐使用实时内核补丁，以保证确定性定时。 |
| **CMake** | 版本 3.5 或更高。 |
| **C++ 编译器** | GCC，支持 C++11 及以上标准。 |
| **Root 权限** | `mlockall`、`sched_setscheduler` 和 EtherCAT 主站操作需要 root 权限。 |

## 项目结构

```
EtherCAT-CPP-Framework/
├── CMakeLists.txt              # CMake 构建配置
├── LICENSE                     # Apache 2.0 许可证
├── README.md
├── include/
│   ├── config.h                # 编译时配置标志和常量
│   ├── EtherCATMaster.h        # EtherCAT 主站抽象类
│   ├── Joint.h                 # CiA 402 关节/电机类
│   ├── JointConfig.h           # PDO 映射和同步管理器配置
│   └── RealtimeManager.h       # 实时设置与定时工具类
├── src/
│   ├── main.cpp                # 应用程序入口与控制循环
│   ├── EtherCATMaster.cpp
│   ├── Joint.cpp
│   ├── JointConfig.cpp
│   └── RealtimeManager.cpp
└── build/                      # 编译输出目录
```

## 编译

```bash
cd EtherCAT-CPP-Framework
mkdir -p build && cd build
cmake ..
make
```

编译产物为单个可执行文件：`build/EC_PROJECT`。

> **注意：** EtherCAT 库路径在 [CMakeLists.txt](CMakeLists.txt#L7) 中硬编码为 `/usr/local/etherlab`。如果您的安装路径不同，请修改 `ETHERLAB_DIR` 变量。

## 快速开始

以 root 权限运行（实时调度和 EtherCAT 主站操作需要）：

```bash
sudo ./build/EC_PROJECT
```

程序执行流程：

1. 配置实时调度（SCHED_FIFO，最高优先级）。
2. 初始化 EtherCAT 主站 0。
3. 配置单个从站（vendor ID `0x5a65726f`，product code `0x00029252`）。
4. 配置 CST（周期同步转矩）模式的 PDO 映射。
5. 启用分布式时钟同步。
6. 等待从站进入 OP（Operational）状态。
7. 进入 1 kHz 控制循环，以转矩设定值 40 驱动电机。

按下 `Ctrl+C` 可优雅释放 EtherCAT 主站并退出。

## 配置说明

所有编译时配置位于 [include/config.h](include/config.h)：

| 配置项 | 说明 |
|---|---|
| `DC` | 启用分布式时钟支持 |
| `CONFIG_PDOS` | 启动时配置 PDO |
| `SHOW_PARAM` | 配置过程中打印参数信息 |
| `FREQUENCY` | 控制循环频率（Hz），默认：1000 |
| `PERIOD_NS` | 控制周期（纳秒），默认：1,000,000 ns = 1 ms |
| `SHIFT0` | SYNC0 事件距周期开始的偏移量，默认：`PERIOD_NS / 2` |
| `ENCODER_RES` | 电机每转编码器增量数，默认：524287（2^19 - 1） |
| `MAX_SAFE_STACK` | 栈预取大小，默认：8 KB |

### 调试开关

在 `config.h` 中取消注释即可启用：

| 开关 | 说明 |
|---|---|
| `MEASURE_PERF` | 打印参考从站时钟时间戳差值 |
| `MEASURE_TIMING` | 打印每个控制周期的执行时间 |
| `SET_CPU_AFFINITY` | 将进程绑定到 CPU 核心 4 |

## 架构概览

```
main.cpp
  │
  ├── RealtimeManager（实时管理器）
  │   ├── set_realtime_priority()   → 设置 SCHED_FIFO
  │   ├── set_cpu_affinity(cpu)     → CPU 核心绑定
  │   ├── lock_memory()             → mlockall 内存锁定
  │   ├── stack_prefault()          → 栈预取，防止缺页
  │   └── nanoSleep()               → clock_nanosleep（绝对时间、单调时钟）
  │
  ├── EtherCATMaster（EtherCAT 主站）
  │   ├── init() / activate()       → 主站生命周期管理
  │   ├── receive() / send()        → 帧收发
  │   ├── createDomain()            → 创建过程数据域
  │   ├── processDomain() / queueDomain()
  │   ├── regPDO2domain()           → PDO 注册到域
  │   └── syncDC()                  → 分布式时钟同步
  │
  └── Joint（CiA 402 关节/电机）
      ├── setMode()                 → 设置运行模式（PP/PV/PT/CSP/CSV/CST）
      ├── enable()                  → 状态机（Fault → … → OpEnabled）
      ├── setTargetPos/Vel/Tor()    → 位置/速度/转矩指令
      ├── getActualPos/Vel/Tor()    → 位置/速度/转矩反馈
      └── jointConfig() / PDOConfig() / dcConfig()
```

## API 参考

### EtherCATMaster

核心类，封装 IGH EtherCAT 主站 C API：

```cpp
EtherCATMaster master;
master.init();              // 申请主站 0
master.createDomain();      // 创建过程数据域
master.regPDO2domain(regs); // 注册 PDO 条目到域
master.activate();          // 激活主站

// 控制循环中：
master.receive();           // 接收 EtherCAT 帧
master.processDomain();     // 处理域数据（更新 PDO）
master.queueDomain();       // 将域数据加入发送队列
master.send();              // 发送 EtherCAT 帧
master.syncDC();            // 同步分布式时钟
```

### Joint

表示总线上的单个伺服驱动器，实现 CiA 402 状态机：

```cpp
Joint joint(master, 总线位置);        // 创建关节对象
joint.setMode(CST);                   // 设置运行模式
joint.init();                         // SDO 下发模式和控制字
joint.jointConfig();                  // 获取从站配置
joint.PDOConfig();                    // 应用 PDO 映射
joint.dcConfig();                     // 配置分布式时钟

// 状态机：
joint.enable();                       // 使能驱动，进入 Operation Enabled
joint.isOperationEnabled();           // 检查驱动是否就绪

// 指令与反馈：
joint.setTargetTor(40);               // 设置转矩指令（CST 模式）
joint.getActualPos();                 // 读取实际位置
joint.getActualVel();                 // 读取实际速度
joint.getActualTor();                 // 读取实际转矩
```

### RealtimeManager

处理 Linux 实时配置：

```cpp
RealtimeManager rt(master);
rt.set_cpu_affinity(4);       // 绑定到 CPU 核心 4
rt.set_realtime_priority();   // 设置 SCHED_FIFO 最高优先级
rt.lock_memory();             // 锁定内存，防止换页
rt.stack_prefault();          // 栈预取，避免缺页

// 控制循环中的精确定时睡眠：
rt.timespec_add(&wakeupTime, &wakeupTime, &cycleTime);
rt.nanoSleep(&wakeupTime);    // 基于绝对时间的单调时钟睡眠
```

## CiA 402 状态机

`Joint::enable()` 方法实现了标准的 CiA 402 驱动状态机：

```
  Fault（故障）─────────────────────────────────────────┐
    │ （Fault Reset: 控制字第 7 位置 1）                   │
    ▼                                                     │
  Switch On Disabled ──（Shutdown: 第 0,1,2 位）──→ Ready to Switch On
                                                             │
                                              （Switch On: 第 0,1,2,3 位）
                                                             │
                                                             ▼
                                                        Switched On
                                                             │
                                         （Enable Op: 第 0,1,2,3,4 位）
                                                             │
                                                             ▼
                                                     Operation Enabled
```

每个状态转换时都会检查从站 `status_word`，并设置相应的 `control_word` 位以推进状态。

## 添加更多关节

框架天然支持多关节。添加第二个轴的示例：

```cpp
Joint joint1(master, 0);  // 第一个从站，总线位置 0
Joint joint2(master, 1);  // 第二个从站，总线位置 1

// 分别配置每个关节...
joint2.setMode(CSP);
joint2.init();
joint2.jointConfig();
joint2.PDOConfig();

// 将所有关节的 PDO 注册到域
joint1.appendPdoRegs(regs);
joint2.appendPdoRegs(regs);
master.regPDO2domain(regs);
```

> **注意：** `JointConfig` 当前定义了一套固定的 PDO 映射。如需驱动不同类型的设备，请创建额外的 `JointConfig` 结构，或按从站参数化 PDO 条目。

## 许可证

本项目采用 Apache License, Version 2.0 许可证。详见 [LICENSE](LICENSE) 文件。

## 参考资料

- [IGH EtherCAT Master (EtherLab)](https://etherlab.org/en/ethercat/)
- [CiA 402 — CANopen 驱动协议](https://www.can-cia.org/can-knowledge/canopen/cia402/)
- [EtherCAT Technology Group](https://www.ethercat.org/)
