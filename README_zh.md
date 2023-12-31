# PD无线充电专用单片机 CH641

[EN](README.md) | 中文

### 概述

CH641系列是基于青稞RISC-V2A内核设计的PD无线充电专用微控制器。该系列具有宽压、单线调
试、低功耗、外围精简等特点。CH641内置PD PHY、BC接口、差分输入电流采样和交流小信号放大解码
器，支持USB PD 及Type C 快充功能、BC1.2 及DCP 等多种HV 充电协议，提供了DMA 控制器、10 位模
数转换ADC、多组定时器、USART串口、I2C等丰富的外设资源，提供过压保护和过温保护。



### 产品特点

- 青稞32位RISC-V2A内核
- 快速可编程中断控制器+硬件中断堆栈
- 支持2级中断嵌套
- 支持系统主频48MHz
- 2KB SRAM，16KB CodeFlash，1920B BootLoader
- 64B系统非易失配置信息存储区
- 64B用户自定义信息存储区
- 系统供电VHV额定电压：5V～12V
- 低功耗模式：睡眠、待机
- 内置出厂调校的24MHz的RC振荡器
- 内置低频RC振荡器
- 上电复位、可编程电压监测器
- 7个DMA通道，支持环形缓冲区管理
- 支持TIM1/ADC/USART/I2C
- 模拟输入范围：GND～3.3V
- 15路外部信号信号+1路内部信号通道
- 支持外部延迟触发
- 1个16位高级定时器，提供死区控制和紧急
  刹车，提供用于电机控制的PWM互补输出
- 1个16位的精简通用定时器
- 1个看门狗定时器（窗口型）
- 系统时基定时器：32位计数器
- 一组多引脚映射的USART串口
- 1个I2C从机接口
- 支持DRP、Sink和Source应用
- 3个CC引脚，部分CC引脚内置Rd
- 支持BC1.2及多种HV DCP充电协议
- 内置6位DAC，支持多种电压输出及上下拉
- 差分输入电流采样ISP/ISN
- 交流小信号放大解码器QII
- 2组GPIO端口，25个I/O口
- 外部中断
- 4个高压驱动引脚，5个低压强驱动引脚
- OVP过压保护和OTP过温保护
- 安全特性：64位芯片唯一ID
- 调试模式：串行单线调试接口
- 封装形式：QFN、QSOP
