# STM32-Based Real-Time Motor Control System

## Overview
This project implements a closed-loop DC motor speed control system using STM32F103 microcontroller.

## Features
- PWM-based motor speed control
- Hall sensor feedback using external interrupts
- Real-time speed monitoring
- Closed-loop proportional speed regulation
- USART-based user input

## Hardware Used
- STM32F103C6
- DC Motor
- Hall Sensor
- Motor Driver (L298N)
- Keil uVision

## Working Principle
The STM32 generates PWM using TIM3 to control motor speed. Hall sensor pulses are counted using EXTI interrupts to measure motor RPM. TIM1 periodically compares actual speed with desired speed and adjusts PWM duty cycle automatically.

## Build Output

Project successfully compiled in Keil uVision with 0 errors and 0 warnings.
