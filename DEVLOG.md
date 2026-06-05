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
