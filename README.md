# Inverted Pendulum Cart Project — D24A (Hall Encoder)

[![Platform](https://img.shields.io/badge/platform-STM32F103RCT6-blue)](https://www.st.com/en/microcontrollers-microprocessors/stm32f103rc.html)
[![Library](https://img.shields.io/badge/library-STDPeriph%20V3.5.0-green)](https://www.st.com/en/embedded-software/stm32-standard-peripheral-libraries.html)
[![IDE](https://img.shields.io/badge/IDE-Keil%20MDK-orange)](https://www.keil.com/product/)
[![Language](https://img.shields.io/badge/language-C-lightgrey)]()

A cart inverted pendulum project based on STM32F103RCT6, the WHEELTEC D24A four-wheel chassis, and the WDD35D4 angular displacement sensor.

The current project already includes:

- WDD35D4 angle sampling and angle conversion
- KEY2 zero calibration and balance start
- KEY3 manual stop
- TIM1 `2 ms` interrupt-driven balance loop
- shared four-wheel PWM output for forward/backward motion
- encoder-based speed and position feedback
- LCD local debug display
- serial debug output for angle, angular rate, speed, position and bias

This project does not currently implement auto swing-up. The pendulum is manually placed near upright, then the controller attempts to keep it from falling using only forward and backward cart motion.

> 📖 [中文文档 (Chinese README)](./README_CN.md)

---

## System Overview

The controller follows an engineering-oriented strategy:

1. angle feedback as the primary balancing term
2. angular-rate feedback to react to falling tendency
3. speed feedback as damping
4. position feedback to reduce long-term drift
5. nonlinear helpers such as minimum output, rescue boost and drift pull-back
6. automatic stop when the pendulum angle exceeds the safety threshold

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | STM32F103RCT6 (High Density, 256KB Flash, 48KB RAM) |
| Chassis | WHEELTEC D24A 4-wheel with hall encoder motors |
| Angle Sensor | WDD35D4 angular displacement sensor |
| Display | 1.44" 128×128 SPI color LCD |
| IDE | Keil MDK (project file: `USER/Tb6612demo.uvprojx`) |
| Library | STM32F10x Standard Peripheral Library V3.5.0 |

---

## Pin Map

### PWM Output (TIM5, 10kHz)

| Motor | PWM Pin |
|-------|---------|
| A | PA0 (TIM5_CH1) |
| B | PA1 (TIM5_CH2) |
| C | PA2 (TIM5_CH3) |
| D | PA3 (TIM5_CH4) |

### Motor Direction

| Motor | IN1 | IN2 |
|-------|-----|-----|
| A | PC13 | PC14 |
| B | PB13 | PB12 |
| C | PB0 | PB1 |
| D | PC1 | PC2 |

### Encoder Inputs

| Motor | Phase A | Phase B | Mode |
|-------|---------|---------|------|
| A | PC6 | PC7 | TIM8 hardware quadrature |
| B | PA15 | PB3 | TIM2 hardware quadrature (full remap) |
| C | PA7 | PA6 | TIM3 hardware quadrature |
| D | PA8 | PA4 | Software decode, TIM6 ISR @ 50kHz |

### Other Peripherals

| Function | Pin |
|----------|-----|
| Battery ADC | PA5 (ADC1_CH5) |
| Angle Sensor | PC4 (ADC1_CH14) |
| USART Debug | PA9 (TX), PA10 (RX) @ 115200 |
| LCD SPI | PB4~PB9 |
| Standby | Tied to 3.3V |

---

## Project Structure

```
D24Ademo/
├── CORE/                     CMSIS core + startup
├── HAREWER/                  Hardware drivers
│   ├── ADC/                  Battery voltage sampling
│   ├── ENCODER/              Encoder (3× hardware + 1× software)
│   ├── GPIO/                 Motor direction pins
│   ├── LCD/                  1.44" LCD driver
│   ├── BALANCE/              Balance controller
│   ├── MOTO/                 Motor direction control
│   └── PWM/                  TIM5 4-channel PWM
├── STM32F10x_FWLIB/          STM32F10x StdPeriph Library V3.5.0
├── SYSTEM/                   System layer (delay, sys, usart)
├── USER/                     Application code
│   ├── main.c                Main program
│   └── Tb6612demo.uvprojx    Keil project file
├── OBJ/                      Linker script
├── CONTRIBUTING.md           Team workflow guide (Chinese)
├── DEVLOG.md                 Development log (Chinese)
├── 控制工程原理课程设计报告.md  Course design report (Chinese)
├── 引脚汇总.md                Pin reference table (Chinese)
└── README_CN.md              Chinese README
```

---

## Current Status

### ✅ Done
- 72MHz system clock
- 4-channel PWM @ 10kHz
- 4× encoder reading (3 hardware + 1 software)
- WDD35D4 angle sampling and angle conversion
- KEY2 zero calibration and balance start
- KEY3 manual stop
- TIM1-based `2 ms` interrupt control loop
- angle + angular-rate + speed + position combined control
- rescue boost, drift pull-back and minimum-output compensation
- 1.44" LCD real-time debug display
- USART debug output @ 115200bps
- Battery voltage monitoring
- Git + GitHub with branch protection & PR workflow

### ❌ TODO
- more robust small-angle recentering
- cleaner model-based tuning
- optional filtering / state estimation
- Steering control
- Auto swing-up (not required in the current project scope)

---

## Runtime Logic

1. Power on and initialize LCD, keys, ADC, PWM, encoders and TIM1.
2. Manually hold the pendulum near the upright position.
3. Press `KEY2` to capture the current angle as zero and start balancing.
4. Every `2 ms`, TIM1 interrupt performs:
   - angle sensor sampling
   - encoder reading
   - angle / angular-rate / speed / position update
   - balance controller execution
   - four-wheel PWM update
5. The main loop handles keys, serial commands and display refresh.
6. Press `KEY3` or exceed the safety angle to stop output.

---

## Getting Started

### Prerequisites
- Keil MDK 5 (ARM Compiler 5)
- ST-Link or J-Link debugger

### Build & Flash
1. Open `USER/Tb6612demo.uvprojx` in Keil MDK
2. Click **Build** (F7)
3. Click **Download** (F8)

> Note:
> the project has been adjusted to disable JTAG while keeping SWD available.
> If an old firmware image still blocks normal access, use `Connect under reset` or hold reset while downloading.

### Quick Test Flow

1. Make sure `BOOT0` is tied to GND for normal Flash boot.
2. Confirm the LCD enters the debug page after power-on.
3. Hold the pendulum close to upright.
4. Press `KEY2` to zero and start balance.
5. Apply a small disturbance and observe whether the cart reacts forward/backward.
6. Press `KEY3` to stop if needed.

### Serial Debug Fields

UART baud rate is `115200`. The main debug fields are:

- `Ang`: current angle
- `Rate`: estimated angular rate
- `SF`: filtered speed
- `Pos`: integrated position term
- `Bias`: angle bias
- `PWM`: control output

These fields are important when analyzing oscillation, one-side drift, or insufficient rescue behavior.

### Serial Commands

- `z` / `Z`: zero and start balance
- `s` / `S`: stop balance output

---

## Document Index

- [Development Log](./DEVLOG.md)
- [Course Design Report](./控制工程原理课程设计报告.md)
- [Pin Summary](./引脚汇总.md)
- [Contribution Guide](./CONTRIBUTING.md)

---

## Collaboration

See [CONTRIBUTING.md](./CONTRIBUTING.md) for the full Git workflow (Chinese).

Quick start:
```bash
git clone https://github.com/WuWingKit/Inverted-pendulum.git
git checkout -b feature/your-feature
# ... write code ...
git add -A && git commit -m "what you did"
git push -u origin feature/your-feature
# → Open GitHub → Create Pull Request → Review → Merge
```

---

## Team

- **WuWingKit** — project owner
- **tulip627722** — motor control, PID tuning, encoder driver
- **hzx** — hardware pin mapping & documentation
- **Lisa-TTT** — gyroscope / angular velocity sensor development
- **zzz-rh** — gyroscope / angular velocity sensor development
