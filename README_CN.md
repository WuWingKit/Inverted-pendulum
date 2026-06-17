# 倒立摆小车工程 — D24A（霍尔编码器版）

[![Platform](https://img.shields.io/badge/platform-STM32F103RCT6-blue)](https://www.st.com/en/microcontrollers-microprocessors/stm32f103rc.html)
[![Library](https://img.shields.io/badge/library-STDPeriph%20V3.5.0-green)](https://www.st.com/en/embedded-software/stm32-standard-peripheral-libraries.html)
[![IDE](https://img.shields.io/badge/IDE-Keil%20MDK-orange)](https://www.keil.com/product/)
[![Language](https://img.shields.io/badge/language-C-lightgrey)]()

基于 STM32F103RCT6、WHEELTEC D24A 四轮编码器底盘和 WDD35D4 角位移传感器的小车倒立摆控制工程。

当前工程已经完成以下核心能力：

- 角位移传感器采样与角度换算
- KEY2 按键调零并启动平衡
- KEY3 按键停止平衡
- TIM1 `2ms` 周期闭环控制
- 四轮统一 PWM 前后运动控制
- 编码器速度与位置反馈
- LCD 本地调试显示
- 串口实时输出角度、角速度、速度、位置和偏置等信息

本项目当前目标不是自动起摆，而是由人工将摆杆扶到近似竖直位置后启动控制，使小车通过前后运动尽可能保持摆杆不倒。

> 📖 [English README](./README.md)

---

## 系统方案

本系统采用“角度主导、角速度辅助、速度阻尼、位置回中、保护停机”的控制思路：

1. 角位移传感器测量摆杆相对竖直位置的偏差。
2. 通过离散差分估算角速度，判断摆杆是在回正还是继续倒下。
3. 通过四轮编码器估算小车速度与位置，抑制越跑越快和单侧漂移。
4. 在 TIM1 中断中每 `2ms` 执行一次控制运算并更新 PWM。
5. 当摆杆偏角超过安全范围时，系统自动停机保护。

## 硬件平台

| 项目 | 详情 |
|------|------|
| MCU | STM32F103RCT6（高密度，256KB Flash，48KB RAM） |
| 底盘 | WHEELTEC D24A 四轮霍尔编码器底盘 |
| 角度传感器 | WDD35D4 角位移传感器 |
| 显示屏 | 1.44 寸 128×128 SPI 彩色 LCD |
| IDE | Keil MDK（工程文件：`USER/Tb6612demo.uvprojx`） |
| 固件库 | STM32F10x 标准外设库 V3.5.0 |

---

## 引脚分配

### PWM 输出（TIM5，10kHz）

| 电机 | PWM 引脚 |
|------|----------|
| A | PA0 (TIM5_CH1) |
| B | PA1 (TIM5_CH2) |
| C | PA2 (TIM5_CH3) |
| D | PA3 (TIM5_CH4) |

### 电机方向控制

| 电机 | IN1 | IN2 |
|------|-----|-----|
| A | PC13 | PC14 |
| B | PB13 | PB12 |
| C | PB0 | PB1 |
| D | PC1 | PC2 |

### 编码器输入

| 电机 | A 相 | B 相 | 解码方式 |
|------|------|------|----------|
| A | PC6 | PC7 | TIM8 硬件正交解码 |
| B | PA15 | PB3 | TIM2 硬件正交解码（完全重映射） |
| C | PA7 | PA6 | TIM3 硬件正交解码 |
| D | PA8 | PA4 | 软件解码，TIM6 中断 @ 50kHz |

### 其他外设

| 功能 | 引脚 |
|------|------|
| 电池 ADC | PA5 (ADC1_CH5) |
| 角位移传感器 | PC4 (ADC1_CH14) |
| 串口调试 | PA9 (TX), PA10 (RX) @ 115200 |
| LCD SPI | PB4 ~ PB9 |
| STBY | 接 3.3V（常使能） |

---

## 项目结构

```
D24Ademo/
├── CORE/                     CMSIS 核心文件 + 启动文件
├── HAREWER/                  硬件驱动层
│   ├── ADC/                  电池电压采样
│   ├── ENCODER/              编码器（3 路硬件 + 1 路软件）
│   ├── GPIO/                 电机方向引脚
│   ├── LCD/                  1.44 寸 LCD 驱动
│   ├── BALANCE/              倒立摆平衡控制器
│   ├── MOTO/                 电机方向控制
│   └── PWM/                  TIM5 四路 PWM
├── STM32F10x_FWLIB/          STM32F10x 标准外设库 V3.5.0
├── SYSTEM/                   系统层（delay、sys、usart）
├── USER/                     应用层
│   ├── main.c                主程序
│   └── Tb6612demo.uvprojx    Keil 工程文件
├── OBJ/                      链接脚本
├── CONTRIBUTING.md           团队协作指南
├── DEVLOG.md                 开发日志
├── 控制工程原理课程设计报告.md  课程设计报告
├── 引脚汇总.md                引脚参考表
└── README.md                 英文 README
```

---

## 当前进度

### ✅ 已完成
- 72MHz 系统时钟
- 四路 PWM 输出 @ 10kHz
- 四路编码器读取（3 硬件 + 1 软件）
- WDD35D4 角位移传感器采样与角度换算
- KEY2 调零并启动平衡
- KEY3 手动停机
- 基于 TIM1 的 2ms 定时中断闭环控制
- 角度 + 角速度 + 速度 + 位置联合控制
- 大角度救摆、漂移回拉、最小输出补偿
- 1.44 寸 LCD 实时调试显示
- 串口调试输出 @ 115200bps
- 电池电压监测
- Git + GitHub 版本管理（分支保护 + PR 审查）

### ❌ 待开发
- 更稳定的小角度长期回中
- 更系统的参数建模与整定
- 可选的辅助滤波 / 状态观测
- 转向控制
- 自动起摆（当前不要求）

---

## 当前运行逻辑

1. 上电后初始化 LCD、按键、ADC、PWM、编码器和 TIM1 控制中断。
2. 将摆杆手动扶到近似竖直位置。
3. 按下 `KEY2`，系统采样当前角度作为零点，并开始平衡。
4. TIM1 每 `2ms` 执行一次：
   - 读取角位移传感器
   - 读取四路编码器
   - 计算角度、角速度、速度和位置
   - 执行平衡控制算法
   - 输出四轮一致的 PWM
5. 主循环中处理按键、串口命令、LCD 刷新和调试信息输出。
6. 按下 `KEY3` 或偏角超出安全范围时，系统停止输出。

---

## 快速开始

### 环境要求
- Keil MDK 5（ARM Compiler 5）
- ST-Link 或 J-Link 调试器

### 编译 & 烧录
1. 用 Keil MDK 打开 `USER/Tb6612demo.uvprojx`
2. 点击 **Build**（F7）
3. 点击 **Download**（F8）

> 注意：
> 当前工程已经修正为“关闭 JTAG、保留 SWD”，正常情况下可继续通过 ST-Link 下载。
> 如果板子仍因旧程序影响而连接困难，可使用 `Connect under reset` 或按住复位键后再下载。

### 上电测试步骤

1. 确认 `BOOT0` 接地，正常从 Flash 启动。
2. 上电后观察 LCD 是否正常显示调试页面。
3. 将摆杆扶到近似竖直位置。
4. 按下 `KEY2` 完成调零并启动控制。
5. 轻微拨动摆杆，观察小车是否开始前后追杆。
6. 如需紧急停止，按下 `KEY3`。

### 串口调试信息

串口波特率为 `115200`，会输出如下核心字段：

- `Ang`：当前角度
- `Rate`：角速度估计
- `SF`：速度滤波值
- `Pos`：位置积分值
- `Bias`：角度偏置
- `PWM`：当前控制输出

这些数据是分析“为什么会抖动、为什么会单侧漂移、为什么大角度救得回来但小角度回不来”的重要依据。

### 串口命令

- `z` / `Z`：串口调零并启动平衡
- `s` / `S`：串口停止平衡输出

---

## 文档索引

- [开发日志](./DEVLOG.md)
- [课程设计报告](./控制工程原理课程设计报告.md)
- [引脚汇总](./引脚汇总.md)
- [协作规范](./CONTRIBUTING.md)

---

## 协作开发

详见 [CONTRIBUTING.md](./CONTRIBUTING.md)。

快速流程：
```bash
git clone https://github.com/WuWingKit/Inverted-pendulum.git
git checkout -b feature/你的功能
# 写代码 ...
git add -A && git commit -m "做了什么"
git push -u origin feature/你的功能
# → 打开 GitHub → 创建 Pull Request → 审查 → 合并
```

---

## 团队成员

- **WuWingKit** — 项目负责人
- **tulip627722** — 电机控制、PID 调参、编码器驱动
- **hzx** — 硬件引脚映射 & 文档
- **Lisa-TTT** — 陀螺仪 / 角速度传感器开发
- **zzz-rh** — 陀螺仪 / 角速度传感器开发
