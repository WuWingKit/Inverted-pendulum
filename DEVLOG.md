# 开发文档 — 倒立摆项目（D24A 霍尔编码器版）

---

## 项目概述

| 项目 | 说明 |
|------|------|
| **项目名称** | D24Ademo — 基于霍尔编码器的四轮电机控制系统 |
| **平台** | WHEELTEC D24A 四轮底盘 |
| **MCU** | STM32F103（中等容量，Flash 256KB / RAM 48KB） |
| **固件库** | STM32F10x Standard Peripheral Library V3.5.0 |
| **IDE** | Keil MDK (工程文件: `USER/Tb6612demo.uvprojx`) |
| **目标** | 最终实现倒立摆自平衡控制 |
| **当前阶段** | 四轮 PI 速度闭环控制 + LCD 调试显示 |
| **GitHub** | https://github.com/WuWingKit/Inverted-pendulum |

---

## 硬件资源分配

### 电机控制引脚

| 电机 | 方向引脚 | PWM 通道 | 定时器 |
|------|----------|----------|--------|
| A | PC13 (AIN1), PC14 (AIN2) | PA0 — TIM5_CH1 | TIM5 |
| B | PB12 (BIN1), PB13 (BIN2) | PA1 — TIM5_CH2 | TIM5 |
| C | PB0 (CIN1), PB1 (CIN2) | PA2 — TIM5_CH3 | TIM5 |
| D | PC1 (DIN1), PC2 (DIN2) | PA3 — TIM5_CH4 | TIM5 |

- PWM 频率：`72MHz / (7199+1) / (0+1) = 10kHz`
- 占空比范围：0 ~ 7200

### 编码器引脚

| 电机 | 编码器接口 | 引脚 | 模式 |
|------|-----------|------|------|
| A | TIM8 硬件编码器 | PC6 (A相), PC7 (B相) | 硬件正交解码 |
| B | TIM2 硬件编码器 | PA15 (A相), PB3 (B相) | 硬件正交解码（完全重映射，禁用 SWJ） |
| C | TIM3 硬件编码器 | PA6 (A相), PA7 (B相) | 硬件正交解码 |
| D | 软件编码器 | PB14 (A相), PB15 (B相) | 软件轮询正交解码 |

> ⚠️ **注意：** TIM2 使用了完全重映射（`GPIO_FullRemap_TIM2`），并禁用了 SWJ 调试接口（`GPIO_Remap_SWJ_Disable`）。这意味着 PA15 和 PB3 被用作编码器输入，**烧录时可能需要按住复位键或使用串口 ISP 下载**。如果调试或下载遇到问题，这是首先要检查的地方。

### 其他外设

| 外设 | 引脚 | 说明 |
|------|------|------|
| ADC 电池检测 | PA5 (ADC1_CH5) | 10 次采样平均，`Vbat = ADC值 × 3.3 × 11 / 4096` |
| USART 调试串口 | PA9 (TX), PA10 (RX) | 波特率 115200 |
| LCD | SPI 接口（具体引脚见 lcd_init.c） | 1.44 寸 128×128 彩色 LCD |

---

## 软件架构

```
main.c
├── SystemInit()              — 系统时钟初始化 (72MHz)
├── delay_init()              — 滴答定时器延时
├── Gpio_Init()               — 电机方向引脚初始化
├── uart_init(115200)         — 串口调试输出
├── adc_Init()                — ADC 电池电压检测
├── PWM_Int(7199, 0)          — 四路 PWM 初始化 (TIM5, 10kHz)
├── Encoder_Init_Tim8()       — 硬件编码器 A (TIM8)
├── Encoder_Init_Tim2()       — 硬件编码器 B (TIM2, 完全重映射)
├── Encoder_Init_Tim3()       — 硬件编码器 C (TIM3)
├── Encoder_Init_Soft()       — 软件编码器 D (PB14/PB15)
├── LCD_Init()                — LCD 初始化
│
└── while(1) 主循环 (50ms) ────
    ├── Get_adc_Average()     — 读取电池电压
    ├── Encoder_Soft_Poll()   — 轮询软件编码器 D
    ├── Read_Encoder()        — 读取四个编码器速度值
    ├── Velocity_A/B/C/D()    — PI 速度闭环计算
    ├── Set_PWM()             — 更新四路 PWM 占空比
    ├── LCD_Show_Debug()      — LCD 显示调试信息
    └── printf()              — 串口输出调试信息
```

### 目录结构

```
D24Ademo/
├── CORE/                     — CMSIS 核心文件
│   ├── core_cm3.c/h          — Cortex-M3 内核接口
│   └── startup_stm32f10x_md.s — 启动文件（中等容量）
├── HAREWER/                  — 硬件驱动层
│   ├── ADC/adc.c/h           — ADC 电池电压采样
│   ├── ENCODER/encoder.c/h   — 编码器（3路硬件 + 1路软件正交解码）
│   ├── GPIO/gpio.c/h         — 电机方向引脚初始化
│   ├── LCD/                  — 1.44寸 LCD 驱动
│   │   ├── lcd.c/h           — LCD 底层操作
│   │   ├── lcd_init.c/h      — LCD 初始化
│   │   └── lcdfont.h         — 字库
│   ├── MOTO/moto.c/h         — 电机方向控制 + PI 速度闭环
│   └── PWM/pwm.c/h           — TIM5 四路 PWM 输出
├── STM32F10x_FWLIB/          — STM32F10x 标准外设库 V3.5.0
│   ├── inc/                  — 头文件
│   └── src/                  — 源文件
├── SYSTEM/                   — 系统层
│   ├── delay/delay.c/h       — 延时函数
│   ├── sys/sys.c/h           — 系统初始化
│   └── usart/usart.c/h       — 串口驱动
├── USER/                     — 应用层
│   ├── main.c                — 主程序
│   ├── stm32f10x.h           — 芯片头文件
│   ├── stm32f10x_conf.h      — 外设库配置文件
│   ├── stm32f10x_it.c/h      — 中断服务函数
│   ├── system_stm32f10x.c/h  — 系统时钟配置
│   └── Tb6612demo.uvprojx    — Keil 工程文件
├── OBJ/                      — 链接脚本
│   └── Minibalance.sct       — 散列加载文件
├── .gitignore                — Git 忽略配置
├── CONTRIBUTING.md           — 团队协作指南
├── DEVLOG.md                 — 本文件（开发日志）
└── keilkill.bat              — Keil 编译产物清理脚本
```

---

## 当前实现状态

### ✅ 已完成

| 功能 | 状态 | 说明 |
|------|------|------|
| 系统时钟 72MHz | ✅ | 标准 HSE 8MHz × 9 PLL |
| GPIO 初始化 | ✅ | PB0/1/12/13, PC1/2/13/14 推挽输出 |
| PWM 输出 | ✅ | TIM5 四路 PWM，频率 10kHz，占空比 0~7200 |
| 编码器 A (TIM8) | ✅ | 硬件正交解码, PC6/PC7 |
| 编码器 B (TIM2) | ✅ | 硬件正交解码, 完全重映射 PA15/PB3 |
| 编码器 C (TIM3) | ✅ | 硬件正交解码, PA6/PA7 |
| 编码器 D (软件) | ✅ | 软件正交解码, PB14/PB15 轮询 |
| ADC 电压检测 | ✅ | PA5, 10次平均, 分压比 11:1 |
| PI 速度闭环 | ✅ | 增量式 PI, Kp=1.0, Ki=3.0, Kd=0 |
| LCD 调试显示 | ✅ | 显示四路速度/PWM、目标速度、电压、PID参数 |
| 串口调试输出 | ✅ | 115200bps, printf 输出实时数据 |
| Git 版本管理 | ✅ | 已配置 .gitignore，推送到 GitHub |

### ❌ 待开发

| 功能 | 优先级 | 说明 |
|------|--------|------|
| MPU6050 陀螺仪驱动 | 🔴 高 | 获取倾角/角速度，倒立摆核心传感器 |
| I2C 通信驱动 | 🔴 高 | MPU6050 依赖 I2C 接口 |
| 直立平衡 PID | 🔴 高 | 角度环 + 角速度环，倒立摆核心算法 |
| 速度环 PID | 🟡 中 | 位置控制，实现定点自平衡 |
| 转向控制 | 🟡 中 | 差速转向，实现遥控/自主导航 |
| 蓝牙/无线调试 | 🟢 低 | 远程调参，方便调试 |
| 上位机调参工具 | 🟢 低 | 实时观察数据曲线，在线调整 PID |

---

## C/D 编码器硬件调试指南

> **问题现象：** LCD 上 A/B 电机编码器读数正常，C/D 显示 0 或 -1
> **代码排查结论：** TIM3 硬件编码器和 D 电机软件编码器初始化代码均无逻辑错误，怀疑硬件连接问题

### 步骤 1：确认实际引脚映射（最重要）

| 编码器 | 代码中使用 | 确认事项 |
|--------|-----------|----------|
| C 电机 | PA6 (A相), PA7 (B相) | 万用表蜂鸣档：编码器 C 接口 A/B 线是否连通到 MCU PA6/PA7？ |
| D 电机 | PB10 (A相), PB11 (B相) | 万用表蜂鸣档：编码器 D 接口 A/B 线是否连通到 MCU PB10/PB11？ |

> ⚠️ **关键怀疑点：** 同事将 D 编码器从 PB14/PB15 改为 PB10/PB11 是基于"PB14 被 E3B 占用"的逻辑判断，但需要实际确认 PCB 上 PB10/PB11 是否有走线连接到 D 编码器接口。

### 步骤 2：示波器/逻辑分析仪测量

转动 C/D 电机，用示波器测量编码器信号引脚，应看到正交方波：

```
A相: ▔▔__▔▔__▔▔__▔▔__
B相: __▔▔__▔▔__▔▔__▔▔  (相位差90°)
```

- 如果**有波形**但读数为 0 → 代码配置问题（TIM3 时钟、引脚复用冲突等）
- 如果**无波形**（恒定高/低电平） → 硬件连接问题（线序错误、虚焊、引脚不对应）

### 步骤 3：软件诊断（临时添加调试代码）

在 `main.c` 的 while 循环中临时加入以下代码，通过串口查看原始引脚电平：

```c
// 临时诊断代码 - 读取 C/D 编码器原始引脚电平
u8 pa6 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6);
u8 pa7 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7);
u8 pb10 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10);
u8 pb11 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
printf("RAW: PA6=%d PA7=%d PB10=%d PB11=%d\r\n", pa6, pa7, pb10, pb11);
```

转动电机时，4 个值应该快速跳变。如果某引脚始终为 0 或 1 不变 → 该引脚无编码器信号。

### 步骤 4：引脚冲突复查

| 引脚 | 默认复用功能 | 与当前用途是否冲突 |
|------|-------------|-------------------|
| PA6 | SPI1_MISO, TIM3_CH1, ADC_IN6 | ✅ 无冲突（SPI1 未初始化，TIM3_CH1 正是编码器用途） |
| PA7 | SPI1_MOSI, TIM3_CH2, ADC_IN7 | ✅ 无冲突 |
| PB10 | I2C2_SCL, USART3_TX, **TIM2_CH3（remap后）** | ⚠️ TIM2_FullRemap 将 TIM2_CH3 重映射到 PB10，虽然配置为 GPIO 输入，但建议实际验证 |
| PB11 | I2C2_SDA, USART3_RX, **TIM2_CH4（remap后）** | ⚠️ 同上 |

> 如果确认 PB10/PB11 有编码器信号但软件读数异常，可尝试临时**禁用 TIM2 完全重映射**（注释 `GPIO_FullRemap_TIM2` 那行），看 D 编码器读数是否恢复，以此判断 TIM2 remap 是否干扰了 PB10/PB11 的 GPIO 输入。

---

## 已知问题

### 1. TIM4 硬件编码器未使用

- **文件：** `HAREWER/ENCODER/encoder.c`
- **问题：** `Encoder_Init_Tim4()` 函数已实现（PB6/PB7），但 `main.c` 中并未调用
- **现状：** 电机 D 使用软件编码器（PB14/PB15）代替
- **建议：** 如果软件编码器性能足够，可删除 TIM4 相关代码以减小代码体积；如果需要更高精度，可改回 TIM4 硬件编码器

### 2. SWJ 调试接口被禁用

- **文件：** `HAREWER/ENCODER/encoder.c:97`
- **问题：** TIM2 完全重映射后调用了 `GPIO_Remap_SWJ_Disable`，禁用了 JTAG/SWD
- **影响：** 无法使用 ST-Link/J-Link 正常调试和下载
- **临时方案：** 烧录时按住复位键，点击下载后松开；或使用串口 ISP 下载
- **根本修复：** 如果不需要使用 PB3，可仅重映射部分引脚而非完全禁用 SWJ

### 3. 四个 PI 控制器代码重复

- **文件：** `HAREWER/MOTO/moto.c`
- **问题：** `Velocity_A/B/C/D()` 四个函数逻辑完全相同，仅函数名不同
- **建议：** 重构为单函数 + 传参，或使用结构体数组管理

### 4. Kd 参数声明但未使用

- **文件：** `HAREWER/MOTO/moto.h:6`
- **问题：** `extern float Velcity_Kd;` 声明了微分系数但 PI 控制中未使用
- **建议：** 保留，后续升级为 PID 时会用到

---

## 开发日志

### 2026-06-16 — 调整长重摆杆平衡参数

- **改动者：** WuWingKit
- **类型：** 参数调整 / 硬件调试
- **改动文件：** `HAREWER/BALANCE/balance.c`, `HAREWER/BALANCE/balance.h`, `DEVLOG.md`
- **内容：**
  - 针对摆杆较长较重、偏移后下落较快的机械特性，将 `BALANCE_PWM_LIMIT` 从 `4200` 提高到 `5200`
  - 将角度环参数从 `Kp=3.4, Kd=13.0` 调整到 `Kp=4.2, Kd=18.0`
  - 将速度反馈 `Balance_Speed_Kp` 从 `0.55` 降到 `0.40`，位置反馈 `Balance_Position_Kp` 从 `0.002` 降到 `0.001`
  - 目标是增强前后追杆能力，减少速度/位置反馈在初期对追杆动作的抑制
- **验证：** 需重新烧录后人工扶正摆杆，按 KEY2 启动，观察是否能更快追上长重摆杆

### 2026-06-16 — 放宽倒立摆自动停机角度

- **改动者：** WuWingKit
- **类型：** 参数调整 / 硬件调试
- **改动文件：** `HAREWER/BALANCE/balance.h`, `DEVLOG.md`
- **内容：**
  - 根据实车调试需要，将 `BALANCE_SAFE_ANGLE_X100` 从 `1500` 调整为 `2500`
  - 自动停机角度范围由约 ±15° 放宽到约 ±25°，避免调参时过早关闭电机
- **验证：** 需重新烧录后确认轻微大角度偏移不会立即停机，同时超过可控范围仍能保护停机

### 2026-06-16 — 继续提高倒立摆修正速度

- **改动者：** WuWingKit
- **类型：** 参数调整 / 硬件调试
- **改动文件：** `HAREWER/BALANCE/balance.c`, `HAREWER/BALANCE/balance.h`, `DEVLOG.md`
- **内容：**
  - 根据实车测试“速度仍然偏慢”的现象，将 `BALANCE_PWM_LIMIT` 从 `3200` 提高到 `4200`
  - 将角度环参数从 `Kp=2.4, Kd=9.5` 调整到 `Kp=3.4, Kd=13.0`
  - 暂时保持 `Balance_Speed_Kp=0.55` 不变，优先增强前后追杆能力
- **验证：** 需重新烧录后观察前后追杆速度是否明显提升，若出现高频抖动再回退 `Kd`

### 2026-06-16 — 提高倒立摆前后修正速度

- **改动者：** WuWingKit
- **类型：** 参数调整 / 硬件调试
- **改动文件：** `HAREWER/BALANCE/balance.c`, `HAREWER/BALANCE/balance.h`, `DEVLOG.md`
- **内容：**
  - 在前后运动方向已确认正确后，将 `BALANCE_PWM_LIMIT` 从 `2500` 提高到 `3200`
  - 将角度环参数从 `Kp=1.8, Kd=7.0` 调整到 `Kp=2.4, Kd=9.5`
  - 目标是提高小车前后追杆速度和姿态响应速度，同时保留当前速度反馈抑制冲出
- **验证：** 需重新烧录后人工扶正摆杆，按 KEY2 启动，观察响应是否更快且没有恢复到猛冲状态

### 2026-06-16 — 修正倒立摆前后运动方向

- **改动者：** WuWingKit
- **类型：** 参数调整 / 硬件调试
- **改动文件：** `HAREWER/BALANCE/balance.h`, `DEVLOG.md`
- **内容：**
  - 根据实车测试“杆子前倾时小车后退”的现象，将 `BALANCE_OUTPUT_SIGN` 从 `-1` 调整为 `1`
  - 目标是让摆杆前倾时小车向前运动，摆杆后倾时小车向后运动
- **验证：** 需重新烧录后人工扶正摆杆，轻微前倾观察小车是否向前追杆

### 2026-06-16 — 降低倒立摆控制输出强度

- **改动者：** WuWingKit
- **类型：** 参数调整 / 硬件调试
- **改动文件：** `HAREWER/BALANCE/balance.c`, `HAREWER/BALANCE/balance.h`, `DEVLOG.md`
- **内容：**
  - 根据实车测试“冲得太快，无法微调”的现象，将 `BALANCE_PWM_LIMIT` 从 `6500` 降到 `2500`
  - 将角度环参数从 `Kp=5.0, Kd=18.0` 降到 `Kp=1.8, Kd=7.0`
  - 将速度反馈 `Balance_Speed_Kp` 从 `0.35` 提高到 `0.55`，帮助抑制小车持续冲出
- **验证：** 需重新烧录后人工扶正摆杆，按 KEY2 启动，观察小车是否能以更小动作前后修正

### 2026-06-16 — 翻转倒立摆平衡输出方向

- **改动者：** WuWingKit
- **类型：** 参数调整 / 硬件调试
- **改动文件：** `HAREWER/BALANCE/balance.h`, `DEVLOG.md`
- **内容：**
  - 根据硬件测试现象“平衡方向反”，将 `BALANCE_OUTPUT_SIGN` 从 `1` 调整为 `-1`
  - 用于反转角度控制到四轮同步 PWM 的输出方向
- **验证：** 需重新烧录后再次人工扶正摆杆，按 KEY2 启动，观察小车是否朝纠正摆杆倾倒的方向运动

### 2026-06-16 — 修复串口接收状态变量编译错误

- **改动者：** WuWingKit
- **类型：** Bug 修复
- **改动文件：** `SYSTEM/usart/usart.c`, `SYSTEM/usart/usart.h`, `DEVLOG.md`
- **内容：**
  - 将 `USART_RX_STA` 的声明和定义统一为 `volatile u16`
  - 修复 ARMCC 报错 `declaration is incompatible with "volatile u16 USART_RX_STA"`
- **验证：** 已根据 Keil 报错定位并完成静态检查；需重新 Rebuild 确认工程通过编译

### 2026-06-16 — 新增四轮小车倒立摆平衡控制基础框架

- **改动者：** WuWingKit
- **类型：** 新增功能 / 控制算法移植
- **改动文件：** `USER/main.c`, `USER/key.c`, `HAREWER/BALANCE/balance.c`, `HAREWER/BALANCE/balance.h`, `USER/Tb6612demo.uvprojx`, `DEVLOG.md`
- **内容：**
  - 参考 IP570 直线倒立摆源码的 5ms 控制节拍、角度 PD、位置/速度反馈、PWM 限幅和倾角保护思路
  - 面向四轮小车场景新增 `HAREWER/BALANCE` 控制模块，四个电机同步输出同一平衡 PWM
  - 不移植直线滑轨自动起摆流程，改为人工扶正摆杆后按 KEY2 调零并进入平衡控制
  - 将主循环从 50ms 调试循环改为 5ms 控制循环，LCD/串口仍按 50ms 刷新，电池电压按 100ms 刷新
  - KEY3 和串口 `s` 可停止平衡输出；串口 `z` 保留为调零并启动平衡的备用入口
  - 按键扫描改为非阻塞多帧消抖，避免控制循环被按键等待卡住
  - 新增 `BALANCE_OUTPUT_SIGN` 用于现场快速翻转角度控制输出方向
- **验证：** 已完成代码静态检查和 Keil 工程 XML 解析检查；需硬件烧录后从低占空比参数开始确认电机方向、角度极性和安全停机阈值

### 2026-06-16 — 增加 WDD35D4 按键调零

- **改动者：** WuWingKit
- **类型：** 新增功能 / 调试优化
- **改动文件：** `USER/main.c`, `USER/key.c`, `USER/key.h`, `USER/Tb6612demo.uvprojx`, `DEVLOG.md`
- **内容：**
  - 参考 `E:\WorkSpace\倒立摆\rfid` 工程的按键实现，新增 `USER/key.c` / `USER/key.h`
  - 避开已被 TIM5_CH1 电机 PWM 占用的 PA0，仅初始化 PC8/PC9 按键输入
  - 将 KEY2(PC8，低电平按下) 作为 WDD35D4 角位移传感器调零键，按下后把当前 `angle_adc` 写入 `zero_offset`
  - 保留串口 `z` 调零作为备用路径，同时启动提示增加 KEY2 调零说明
  - 将 `key.c` 加入 Keil 工程 USER 分组，确保工程编译包含按键驱动
- **验证：** 已完成代码静态检查；需在硬件上烧录后竖直放置杆体，按 KEY2，观察 LCD `Ang` 接近 0 且串口输出 `Zero set by KEY2!`

### 2026-06-14 — WDD35D4 角位移传感器集成，ADC 测试模式

- **改动者：** WuWingKit
- **类型：** 新增功能 / 文档更新
- **改动文件：** `HAREWER/ADC/adc.c`, `HAREWER/FILTER/filter.c`, `HAREWER/FILTER/filter.h`, `USER/main.c`, `引脚汇总.md`, `DEVLOG.md`
- **内容：**
  - ADC 初始化增加 PC4 (ADC1_CH14)，用于 WDD35D4 角位移传感器采样
  - 新增卡尔曼滤波器 + 一阶互补滤波器（从 WHEEL_ADC 项目迁移）
  - main.c 切换为传感器测试模式：电机全部禁用，仅读取 WDD35D4 角度数据
  - LCD 显示：原始 ADC 值、转换后角度值、电池电压
  - 串口输出角度数据 @ 20Hz
  - WDD35D4 接线：VCC→3.3V, GND→GND, 信号→PC4
  - 更新引脚汇总文档
- **验证：** 待硬件测试 — 上电后 LCD 应显示 ADC 原始值和角度，手动旋转传感器时数值变化

### 2026-06-05 — 添加中英文 README + 修复 DEVLOG 合并冲突 + 标准化开发日志

- **改动者：** WuWingKit
- **类型：** 文档更新
- **改动文件：** `README.md`, `README_CN.md`, `DEVLOG.md`
- **内容：**
  - 新增 `README.md`（英文主文档），包含硬件平台、引脚分配、项目结构、当前进度、协作流程
  - 新增 `README_CN.md`（中文文档），英文版通过链接指向中文版
  - 修复 `DEVLOG.md` 中遗留的 Git 合并冲突标记（`<<<<<<< HEAD` / `=======` / `>>>>>>>`），合并 hzx 与 tulip627722 的两条日志
  - 统一所有开发日志条目格式，修正不规范写法
- **验证：** 文档推送到 GitHub 后可正常渲染

### 2026-06-05 — B电机方向修正 + PID参数调优 + 目标速度降低

- **改动者：** tulip627722
- **类型：** Bug修复 / 参数调整
- **改动文件：** HAREWER/MOTO/moto.c, HAREWER/PWM/pwm.c, HAREWER/ENCODER/encoder.c, USER/main.c
- **内容：**
  - **B 电机方向修正：** 根据实际接线（BIN1=PB13, BIN2=PB12）修正 moto.c 和 pwm.c 中的引脚配置（交换 PB12/PB13）
  - **B 编码器读数取反：** encoder.c Read_Encoder case 2 中加负号，匹配硬件编码器接线方向
  - **PID 参数调优：** Kp 从 1.0 降至 0.5，Ki 从 3.0 降至 0.15，解决电机正反来回震荡问题，加减速更平滑
  - **目标速度降低：** TargetVelocity 从 500 降至 300，实现低速直行调试
- **验证：** 四轮同向稳定运转，PWM 平滑调节不再爬升至满速


### 2026-06-05 — 引脚汇总文档 + D编码器改用TIM6中断轮询

- **改动者：** hzx, tulip627722
- **类型：** 功能调整 / 文档更新
- **改动文件：** `HAREWER/ENCODER/encoder.c`, `HAREWER/ENCODER/encoder.h`, `USER/main.c`, `引脚汇总.md`
- **内容：**
  - 新建 `引脚汇总.md`，记录 STM32 实际硬件引脚映射表（PWM / 方向 / 编码器 / ADC / LCD）
  - 编码器 E4 (D电机) 实际使用 PA8 (A相) / PA4 (B相)，与代码中 PB10/PB11 不同，代码待适配
  - C 电机编码器 E3 实际接线 PA7 (A相) / PA6 (B相)，与代码中 PA6/PA7 的 A/B 相对应交换
  - B 电机方向引脚实际接线 BIN1=PB13 / BIN2=PB12
  - STBY 引脚直连 3.3V（常使能）
  - D 编码器从主循环 50ms 轮询改为 TIM6 定时器中断轮询（50kHz），解决高速时脉冲丢失导致读数一直为 0/-1 的问题
  - 50kHz 下 D 轮读数从 300 提升至 3000+，与其他轮（5000+）接近
  - TIM6 中断优先级设为 1，不影响其他外设中断
- **验证：** D 编码器可稳定读数，四轮速度闭环控制正常，小车可直线运行

### 2026-06-05 — 修复 TIM8 中断名 + C/D 编码器问题排查

- **改动者：** WuWingKit
- **类型：** Bug修复
- **改动文件：** `HAREWER/ENCODER/encoder.c`
- **内容：**
  - `TIM8_IRQHandler` → `TIM8_UP_IRQHandler`：HD 启动文件将 TIM8 拆分为 4 个独立中断向量（`TIM8_UP_IRQHandler`），旧名称与向量表不匹配，溢出时会导致 MCU 死循环
  - 修正 `Encoder_Init_Tim8()` 函数注释（误标为 TIM1）
  - 修正 `Encoder_Init_Tim3()` 函数注释（误标为 TIM2）
  - 排查 C/D 电机编码器读 0/-1 问题：TIM3 代码逻辑无 bug，软件编码器 PB10/PB11 配置无 bug，疑为硬件连接问题
- **验证：** 编译通过，待硬件调试确认 C/D 问题

### 2026-06-05 — LCD调试显示适配 + D编码器引脚调整 + RCT6启动文件更新

- **改动者：** tulip627722
- **类型：** 功能调整 / Bug修复
- **改动文件：** HAREWER/LCD/lcd_init.c, HAREWER/LCD/lcd_init.h, HAREWER/ENCODER/encoder.c, USER/main.c, USER/Tb6612demo.uvprojx, HAREWER/MOTO/moto.c, CORE/startup_stm32f10x_hd.s
- **内容：**
  - LCD 恢复PDF原始引脚（PB4~PB9），SCLK/PB4, MOSI/PB5, RES/PB6, DC/PB7, CS/PB8, BLK/PB9
  - D编码器从PB14/PB15改到PB10/PB11（PB14被E3B占用）
  - 启动文件从 _md.s 更换为 _hd.s（适配STM32F103RCT6高密度芯片）
  - 恢复LCD初始化序列为原始参数（修复白屏问题，LCD已正常显示调试数据）
  - PWM限幅从7200调整为7500
- **验证：** LCD显示正常，A轮速度读数稳定，B/C/D轮待进一步调试


### 2026-06-05 — 项目初始化，Git 仓库配置

- **改动者：** WuWingKit
- **类型：** 项目配置
- **改动文件：** `.gitignore`, `CONTRIBUTING.md`, `DEVLOG.md`
- **内容：**
  - 初始化 Git 仓库
  - 添加 `.gitignore`，忽略 Keil 编译产物（`*.axf`, `*.hex`, `*.o`, `*.d` 等）
  - 首次提交：81 个文件，42,767 行代码
  - 推送到 GitHub: `https://github.com/WuWingKit/Inverted-pendulum`
  - 编写 `CONTRIBUTING.md` 团队协作指南
  - 编写 `DEVLOG.md` 开发文档，梳理当前项目架构与状态
- **验证：** 推送成功，GitHub 仓库可正常访问

---

> **格式说明：** 每次改动按照以下模板追加到「开发日志」上方：
>
> ```markdown
> ### YYYY-MM-DD — 改动简述
>
> - **改动者：** 姓名
> - **类型：** 新增功能 / Bug修复 / 参数调整 / 文档更新
> - **改动文件：** `HAREWER/XXX/xxx.c`
> - **内容：** 详细描述
> - **验证：** 如何验证
> ```
