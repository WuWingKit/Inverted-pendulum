# 倒立摆 — D24A（霍尔编码器版）

基于 STM32F103RCT6 的四轮电机控制项目，目标是实现倒立摆自平衡控制。当前处于 **PI 速度闭环 + LCD 调试显示** 阶段。

> 📖 [English README](./README.md)

---

## 硬件平台

| 项目 | 详情 |
|------|------|
| MCU | STM32F103RCT6（高密度，256KB Flash，48KB RAM） |
| 底盘 | WHEELTEC D24A 四轮霍尔编码器底盘 |
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
│   ├── MOTO/                 电机方向 + PI 控制器
│   └── PWM/                  TIM5 四路 PWM
├── STM32F10x_FWLIB/          STM32F10x 标准外设库 V3.5.0
├── SYSTEM/                   系统层（delay、sys、usart）
├── USER/                     应用层
│   ├── main.c                主程序
│   └── Tb6612demo.uvprojx    Keil 工程文件
├── OBJ/                      链接脚本
├── CONTRIBUTING.md           团队协作指南
├── DEVLOG.md                 开发日志
├── 引脚汇总.md                引脚参考表
└── README.md                 英文 README
```

---

## 当前进度

### ✅ 已完成
- 72MHz 系统时钟
- 四路 PWM 输出 @ 10kHz
- 四路编码器读取（3 硬件 + 1 软件）
- PI 速度闭环控制（Kp=0.5, Ki=0.15）
- 1.44 寸 LCD 实时调试显示
- 串口调试输出 @ 115200bps
- 电池电压监测
- Git + GitHub 版本管理（分支保护 + PR 审查）

### ❌ 待开发
- MPU6050 陀螺仪驱动
- I2C 通信
- 直立平衡 PID（角度环 + 角速度环）
- 转向控制
- 蓝牙无线调试

---

## 控制参数

| 参数 | 值 |
|------|-----|
| 目标速度 | 300 |
| Kp | 0.5 |
| Ki | 0.15 |
| Kd | 0（未使用） |
| PWM 限幅 | ±7500 |
| 主循环周期 | 50ms |

---

## 快速开始

### 环境要求
- Keil MDK 5（ARM Compiler 5）
- ST-Link 或 J-Link 调试器

### 编译 & 烧录
1. 用 Keil MDK 打开 `USER/Tb6612demo.uvprojx`
2. 点击 **Build**（F7）
3. 点击 **Download**（F8）

> ⚠️ SWJ 调试接口已禁用（TIM2 完全重映射），烧录时需按住复位键，点击下载后松开；或使用串口 ISP 下载。

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
