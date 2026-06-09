# Inverted Pendulum — D24A (Hall Encoder)

A four-wheel motor control project based on STM32F103RCT6, targeting self-balancing inverted pendulum control. Currently at the **PI velocity closed-loop + LCD debugging** stage.

> 📖 [中文文档 (Chinese README)](./README_CN.md)

---

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | STM32F103RCT6 (High Density, 256KB Flash, 48KB RAM) |
| Chassis | WHEELTEC D24A 4-wheel with hall encoder motors |
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
│   ├── MOTO/                 Motor direction + PI controller
│   └── PWM/                  TIM5 4-channel PWM
├── STM32F10x_FWLIB/          STM32F10x StdPeriph Library V3.5.0
├── SYSTEM/                   System layer (delay, sys, usart)
├── USER/                     Application code
│   ├── main.c                Main program
│   └── Tb6612demo.uvprojx    Keil project file
├── OBJ/                      Linker script
├── CONTRIBUTING.md           Team workflow guide (Chinese)
├── DEVLOG.md                 Development log (Chinese)
├── 引脚汇总.md                Pin reference table (Chinese)
└── README_CN.md              Chinese README
```

---

## Current Status

### ✅ Done
- 72MHz system clock
- 4-channel PWM @ 10kHz
- 4× encoder reading (3 hardware + 1 software)
- PI velocity closed-loop control (Kp=0.5, Ki=0.15)
- 1.44" LCD real-time debug display
- USART debug output @ 115200bps
- Battery voltage monitoring
- Git + GitHub with branch protection & PR workflow

### ❌ TODO
- MPU6050 gyroscope driver
- I2C communication
- Balance PID (angle + angular velocity)
- Steering control
- Wireless debug (Bluetooth)

---

## Control Parameters

| Parameter | Value |
|-----------|-------|
| Target Velocity | 300 |
| Kp | 0.5 |
| Ki | 0.15 |
| Kd | 0 (unused) |
| PWM Limit | ±7500 |
| Main Loop | 50ms |

---

## Getting Started

### Prerequisites
- Keil MDK 5 (ARM Compiler 5)
- ST-Link or J-Link debugger

### Build & Flash
1. Open `USER/Tb6612demo.uvprojx` in Keil MDK
2. Click **Build** (F7)
3. Click **Download** (F8)

> ⚠️ SWJ debug interface is disabled (TIM2 full remap). Hold the reset button while clicking Download, then release — or use serial ISP.

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
