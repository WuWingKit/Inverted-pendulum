# STM32 Inverted Pendulum Cart

[中文](./README.zh-CN.md) · [Demo video](./media/demonstration.mp4) · [Presentation](./presentation/project-presentation.pptx) · [Course report](./控制工程原理课程设计报告.md)

[![STM32F103](https://img.shields.io/badge/MCU-STM32F103RCT6-03234B?logo=stmicroelectronics)](https://www.st.com/en/microcontrollers-microprocessors/stm32f103rc.html)
[![Control](https://img.shields.io/badge/Control-PD_+_Speed_+_Position-0A8FDC)](#control-strategy)
[![C](https://img.shields.io/badge/Language-C-A8B9CC?logo=c)](https://www.iso.org/standard/82075.html)
[![Keil](https://img.shields.io/badge/IDE-Keil_MDK-394049)](https://www.keil.com/)
[![Loop](https://img.shields.io/badge/Control_loop-500_Hz-success)](#runtime-logic)
[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/Archive-CC_BY--NC--SA_4.0-lightgrey.svg)](./LICENSE-CONTENT.md)

A course-design prototype that uses an STM32F103RCT6, a WHEELTEC D24A four-wheel encoder chassis, and a WDD35D4 angular displacement sensor to investigate real-time inverted-pendulum stabilization.

![Physical prototype and system overview](./assets/system-overview.png)

## Project at a glance

| Item | Details |
|---|---|
| Project period | **June 2026** |
| Course | Control Engineering Principles — course design |
| Project lead | **Hu Rongjie (胡荣杰 / WuWingKit)** |
| Team | tulip627722, hzx, Lisa-TTT, zzz-rh |
| Platform | STM32F103RCT6 + D24A chassis + WDD35D4 angle sensor |
| Control mode | Manual upright placement; forward/backward balance only |
| Prototype cost | **Not recorded in the supplied materials — to be added** |

### Team contributions

- **Hu Rongjie — project lead/owner:** overall architecture, system integration, controller iteration, debugging, repository and final archive.
- **tulip627722:** motor control, PID tuning, and encoder driver.
- **hzx:** hardware pin mapping and documentation.
- **Lisa-TTT, zzz-rh:** gyroscope/angular-velocity sensor exploration.

The final archived controller uses the WDD35D4 angular displacement sensor and discrete angle-rate estimation. Team credits above preserve the roles recorded by the original repository.

## Commercialization and application analysis

An inverted-pendulum cart is primarily an education and control-algorithm validation platform. Its commercial value lies in laboratory teaching kits, controller-tuning demonstrations, embedded real-time training, and rapid comparison of PID, state-feedback, LQR, or observer-based methods. The same balance principles also transfer to self-balancing robots and unstable-platform control.

A productized teaching kit would benefit from a rigid modular frame, guarded moving parts, repeatable sensor mounting, guided tuning software, automatic experiment logging, replaceable components, and a documented safety envelope. The present prototype is a course-stage experimental platform and should not be presented as a transportation product or unattended balancing system.

## Control strategy

The controller combines several feedback terms:

```text
angle feedback + angular-rate damping
             - speed damping
             - position centering
             + nonlinear rescue / minimum-output compensation
             → four-wheel PWM
```

- Angle error is the dominant balancing term.
- A discrete derivative estimates angular rate and reacts to the falling direction.
- Encoder feedback damps cart speed and integrates position to reduce long-term drift.
- Minimum-output compensation helps overcome motor static friction.
- Rescue boost and drift pull-back assist recovery outside the small-angle region.
- Output is disabled when the safety-angle threshold is exceeded.

![Control strategy](./assets/control-strategy.png)

## Runtime logic

1. Initialize the LCD, keys, ADC, PWM, encoders, and timers.
2. Manually hold the pendulum close to upright.
3. Press `KEY2` to capture the WDD35D4 zero reference and enable balance control.
4. Every `2 ms` (`500 Hz`), TIM1 samples the angle, reads four encoders, updates angle rate/speed/position, computes the controller, and writes motor PWM.
5. `KEY3`, serial stop, or excessive angle disables the motors.

The project **does not implement automatic swing-up**. A presentation draft briefly described motion from hanging to upright, but the final firmware and demonstrated workflow require manual upright placement.

## Hardware and important interfaces

| Function | Implementation |
|---|---|
| MCU | STM32F103RCT6, 72 MHz |
| Chassis | WHEELTEC D24A four-wheel chassis with Hall encoders |
| Angle sensor | WDD35D4 on `PC4 / ADC1_CH14` |
| Motor PWM | TIM5 CH1–CH4 on `PA0–PA3`, 10 kHz |
| Encoder A/B/C | TIM8 / TIM2 / TIM3 hardware quadrature |
| Encoder D | Software decoding in TIM6 ISR at 50 kHz |
| Local UI | 1.44-inch 128×128 SPI LCD and keys |
| Debug | USART1, 115200 bps |

## Development progression

- Replaced a slow 50 ms software-polling path for encoder D with a 50 kHz TIM6 interrupt decoder.
- Added 50-sample zero calibration for the WDD35D4 sensor.
- Consolidated the final balance computation into a deterministic 2 ms TIM1 loop.
- Added motor dead-zone compensation after observing PWM output without physical motion.
- Reduced overshoot through gain/limit tuning and added drift detection with reverse pull-back.

![Test and tuning findings](./assets/test-and-tuning.png)

## Build and test

1. Open `USER/Tb6612demo.uvprojx` in Keil MDK 5 using ARM Compiler 5.
2. Build and flash through ST-Link or J-Link.
3. Confirm `BOOT0` is low and the LCD debug page appears.
4. Hold the pendulum near upright and press `KEY2`.
5. Apply only a small disturbance and keep clear of the moving chassis; press `KEY3` to stop.

Serial commands: `z`/`Z` zeroes and starts; `s`/`S` stops. Debug output includes angle, rate, filtered speed, position, bias, ADC, and PWM.

## Repository contents

- `HAREWER/BALANCE/`: final balance controller
- `HAREWER/ENCODER/`, `MOTO/`, `PWM/`: motion feedback and actuation
- `USER/main.c`: initialization, UI, commands, and 2 ms control ISR
- `DEVLOG.md`: development history
- `控制工程原理课程设计报告.md` and `倒立摆小车开发文档.pdf`: technical documentation
- `presentation/project-presentation.pptx`: final course presentation
- `media/demonstration.mp4`: physical prototype demonstration
- `assets/`: README images exported from the presentation

## Current limitations

- Manual upright placement is required; there is no automatic swing-up.
- The cart only controls forward/backward motion and does not steer.
- Small-angle recentering and long-duration drift rejection still require improvement.
- The controller is empirically tuned; no complete plant identification or formal stability proof is included.
- Keep the test area clear: the cart can accelerate unexpectedly during recovery.

## License

Project-authored documentation, presentation, images, video, and hardware-design material are shared under **CC BY-NC-SA 4.0**; see [LICENSE-CONTENT.md](./LICENSE-CONTENT.md). Source and third-party vendor components retain the terms stated in their respective files.

