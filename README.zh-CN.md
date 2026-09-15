# 基于 STM32 的倒立摆小车

![倒立摆小车实体成品](./assets/physical-prototype.jpg)

[English](./README.md) · [演示视频](./media/demonstration.mp4) · [终期汇报](./presentation/project-presentation.pptx) · [课程报告](./控制工程原理课程设计报告.md)

[![STM32F103](https://img.shields.io/badge/MCU-STM32F103RCT6-03234B?logo=stmicroelectronics)](https://www.st.com/en/microcontrollers-microprocessors/stm32f103rc.html)
[![Control](https://img.shields.io/badge/Control-PD_+_Speed_+_Position-0A8FDC)](#控制策略)
[![C](https://img.shields.io/badge/Language-C-A8B9CC?logo=c)](https://www.iso.org/standard/82075.html)
[![Keil](https://img.shields.io/badge/IDE-Keil_MDK-394049)](https://www.keil.com/)
[![Loop](https://img.shields.io/badge/Control_loop-500_Hz-success)](#运行流程)
[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/Archive-CC_BY--NC--SA_4.0-lightgrey.svg)](./LICENSE-CONTENT.md)

一套基于 STM32F103RCT6、WHEELTEC D24A 四轮编码器底盘与 WDD35D4 角位移传感器的倒立摆控制课程设计原型。上图是从终期汇报中提取的完成版实体照片。

## 项目基本信息

| 项目 | 内容 |
|---|---|
| 项目时间 | **2026 年 6 月** |
| 课程 | 控制工程原理课程设计 |
| 项目组长 | **胡荣杰（WuWingKit）** |
| 团队成员 | tulip627722、hzx、Lisa-TTT、zzz-rh |
| 硬件平台 | STM32F103RCT6 + D24A 底盘 + WDD35D4 角位移传感器 |
| 控制方式 | 人工扶正启动，仅控制小车前后平衡 |
| 原型成本 | **现有资料未记录，待后续补充** |

### 团队分工

- **胡荣杰——项目组长/负责人：**总体架构、系统集成、控制器迭代、调试、仓库与最终归档；
- **tulip627722：**电机控制、PID 整定、编码器驱动；
- **hzx：**硬件引脚整理与文档；
- **Lisa-TTT、zzz-rh：**陀螺仪/角速度传感器方向探索。

最终归档控制器实际使用 WDD35D4 角位移传感器与离散差分角速度估计；以上署名保留原仓库记录的分工。

## 商业化与应用分析

倒立摆小车的主要价值是控制算法教学与验证，可用于实验室教学套件、参数整定演示、嵌入式实时控制训练，以及 PID、状态反馈、LQR 或观测器方法的快速对比。其平衡原理也可迁移到两轮自平衡机器人和不稳定平台控制。

若产品化为教学套件，应增加刚性模块化结构、运动防护、可重复安装的传感器、参数整定软件、自动实验记录、易更换部件与明确安全边界。当前成果是课程阶段实验平台，不是交通工具或无人值守平衡设备。

## 控制策略

```text
角度反馈 + 角速度阻尼
        - 速度阻尼
        - 位置回中
        + 非线性救摆 / 最小输出补偿
        → 四轮 PWM
```

- 角度误差是主要平衡项；
- 通过离散差分估计角速度并判断倾倒方向；
- 编码器速度反馈提供阻尼，位置积分抑制长期漂移；
- 最小输出补偿克服电机静摩擦死区；
- 大角度救摆和漂移回拉辅助恢复；
- 超过安全角度阈值后关闭输出。

![控制策略](./assets/control-strategy.png)

## 运行流程

1. 初始化 LCD、按键、ADC、PWM、编码器和定时器；
2. 人工将摆杆扶到近似竖直位置；
3. 按 `KEY2` 采集 WDD35D4 零点并启动平衡；
4. TIM1 每 `2 ms`（`500 Hz`）采集角度、读取四路编码器、更新角速度/速度/位置、计算控制器并写入电机 PWM；
5. 按 `KEY3`、发送串口停止命令或偏角过大时关闭电机。

项目**没有实现自动起摆**。汇报草稿中曾出现“从自然下垂到直立”的表述，但最终源码和实际演示流程都要求人工扶正后启动。

## 硬件与软件总体架构

![系统组成](./assets/system-overview.png)

```text
WDD35D4 角度传感器 ─ ADC ─┐
四路车轮编码器 ─── 定时器 ├─→ TIM1 控制中断，每 2 ms 执行
                            │      ├─ 角度与角速度估计
按键 / 串口命令 ───────────┘      ├─ 速度与位置反馈
                                   ├─ Balance_Update()
                                   └─ 安全保护与输出限幅
                                              ↓
                                      PWM + 方向驱动
                                              ↓
                                         四路直流电机

主循环          → 按键、命令、LCD 刷新和串口调试
TIM6 50 kHz 中断 → D 路编码器软件正交解码
```

### 固件分层

| 层级 | 主要路径 | 作用 |
|---|---|---|
| 应用层 | `USER/main.c`、`USER/key.c` | 上电初始化、调零/启动/停止、LCD、串口调试及 TIM1 控制中断 |
| 控制器 | `HAREWER/BALANCE/` | 角速度估计、速度滤波、位置积分、救摆、漂移修正、死区补偿与输出限幅 |
| 反馈采集 | `HAREWER/ADC/`、`ENCODER/`、`FILTER/` | 角度/电池 ADC、四路编码器和信号滤波 |
| 执行层 | `HAREWER/PWM/`、`MOTO/`、`GPIO/` | 四路 PWM 和电机方向控制 |
| 人机交互 | `HAREWER/LCD/`、`SYSTEM/usart/` | 本地状态显示与 115200 bps 串口调试 |
| 平台层 | `CORE/`、`STM32F10x_FWLIB/`、`SYSTEM/` | CMSIS、启动文件、标准外设库、时钟和延时 |

目录名 `HAREWER` 是原工程遗留拼写，为避免破坏 Keil 工程引用路径而保留。

## 硬件与关键接口

| 功能 | 实现 |
|---|---|
| MCU | STM32F103RCT6，72 MHz |
| 底盘 | WHEELTEC D24A 四轮霍尔编码器底盘 |
| 角度传感器 | WDD35D4，`PC4 / ADC1_CH14` |
| 电机 PWM | TIM5 CH1–CH4，`PA0–PA3`，10 kHz |
| A/B/C 编码器 | TIM8 / TIM2 / TIM3 硬件正交解码 |
| D 编码器 | TIM6 50 kHz 中断软件解码 |
| 本地交互 | 1.44 寸 128×128 SPI LCD 与按键 |
| 调试串口 | USART1，115200 bps |

## 迭代过程

- 将 D 编码器由 50 ms 软件轮询改为 TIM6 50 kHz 中断解码；
- 为 WDD35D4 增加 50 次采样零点标定；
- 将最终平衡运算统一到确定性的 TIM1 2 ms 控制周期；
- 针对“有 PWM 但车不动”加入电机死区补偿；
- 通过增益/限幅调整降低过冲，并加入漂移识别与反向回拉。

![实验调试与结果分析](./assets/test-and-tuning.png)

## 下载、编译与测试

### 1. 下载源码

```bash
git clone https://github.com/WuWingKit/Inverted-pendulum.git
cd Inverted-pendulum
```

没有安装 Git 时，可在 GitHub 选择 **Code → Download ZIP** 并解压。

### 2. 编译与烧录

1. 安装 Keil MDK 5、ARM Compiler 5 和 ST-Link 或 J-Link 驱动；
2. 打开 `USER/Tb6612demo.uvprojx`；
3. 按 `F7` 编译现有 Target，在连接电机前应先消除全部编译错误；
4. 通过 SWD 连接下载器，按 `F8` 烧录；
5. 确认 `BOOT0` 为低电平，复位后 LCD 出现调试界面。

### 3. 首次受控测试

1. 先架空底盘，分别检查电机方向和四路编码器符号；
2. 查看 WDD35D4 原始 ADC，确认摆杆倾斜时角度变化方向正确；
3. 将小车放到空旷地面，人工扶正摆杆并按 `KEY2` 采集零点、启动控制；
4. 只施加小幅扰动。若电机方向错误或小车持续加速，立即按 `KEY3`；
5. 修改参数前先查看串口字段：`Ang`、`Rate`、`SF`、`Pos`、`Bias` 和 `PWM`。

串口命令：`z`/`Z` 调零并启动，`s`/`S` 停止。调试输出包括角度、角速度、滤波速度、位置、偏置、ADC 与 PWM。

## 仓库内容

- `USER/Tb6612demo.uvprojx`：Keil MDK 工程入口
- `HAREWER/BALANCE/`：最终平衡控制器
- `HAREWER/ENCODER/`、`MOTO/`、`PWM/`：运动反馈与执行
- `USER/main.c`：初始化、交互、命令与 2 ms 控制中断
- `DEVLOG.md`：开发日志
- `控制工程原理课程设计报告.md`、`倒立摆小车开发文档.pdf`：技术文档
- `presentation/project-presentation.pptx`：终期课程汇报
- `media/demonstration.mp4`：实体原型演示
- `assets/`：从 PPT 导出的 README 配图

## 当前限制

- 需要人工扶正，没有自动起摆；
- 只控制前后运动，不支持转向；
- 小角度长期回中与漂移抑制仍需改进；
- 参数主要依靠实验整定，没有完整对象辨识或形式化稳定性证明；
- 救摆时小车可能突然加速，测试区域必须保持净空。

## 协议

项目原创文档、汇报、图片、视频和硬件设计资料采用 **CC BY-NC-SA 4.0**；详见 [LICENSE-CONTENT.md](./LICENSE-CONTENT.md)。源码及第三方厂商组件仍遵循各自文件中注明的许可条款。
