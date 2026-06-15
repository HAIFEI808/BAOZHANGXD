# 基于FreeRTOS的智能手表嵌入式系统设计与实现

---

## 目录

- [中文摘要](#中文摘要)
- [英文摘要（Abstract）](#英文摘要abstract)
- [第1章 绪论](#第1章-绪论)
  - [1.1 研究背景](#11-研究背景)
  - [1.2 国内外研究现状](#12-国内外研究现状)
  - [1.3 研究内容与论文结构](#13-研究内容与论文结构)
- [第2章 系统设计](#第2章-系统设计)
  - [2.1 系统总体设计](#21-系统总体设计)
  - [2.2 系统功能与需求说明](#22-系统功能与需求说明)
  - [2.3 开发环境与技术栈](#23-开发环境与技术栈)
- [第3章 硬件电路设计](#第3章-硬件电路设计)
  - [3.1 单片机最小系统设计](#31-单片机最小系统设计)
  - [3.2 外设模块电路设计](#32-外设模块电路设计)
- [第4章 系统程序设计](#第4章-系统程序设计)
  - [4.1 FreeRTOS任务架构设计](#41-freertos任务架构设计)
  - [4.2 任务间通信与同步机制](#42-任务间通信与同步机制)
  - [4.3 核心功能模块程序实现](#43-核心功能模块程序实现)
- [第5章 系统测试](#第5章-系统测试)
- [第6章 结论](#第6章-结论)
- [参考文献](#参考文献)
- [附录](#附录)

---

## 中文摘要

本论文围绕基于FreeRTOS实时操作系统的智能手表嵌入式系统展开设计与实现。系统以STM32F401RET6微控制器为核心处理平台，搭载GC9A01圆形LCD显示屏、CST816D电容式触摸控制器、RTC实时时钟及I2S音频输出等外设模块，构建了一个具备时间显示、闹钟提醒、触控交互及自动息屏低功耗管理的完整智能手表原型系统。

在软件架构方面，本系统基于FreeRTOS实时内核设计了六任务并发调度框架，包含GUI刷新任务（10ms周期）、触控采集任务（20ms周期）、RTC时间读取任务（500ms周期）、系统状态管理任务、调试日志任务及默认空闲任务。任务间通信综合运用了互斥锁（mutex）保护LVGL图形库临界资源、消息队列实现RTC时间数据的异步传输、直接任务通知实现触控手势事件的低延迟传递，以及事件标志组预留系统状态同步。在低功耗管理方面，通过系统状态管理任务内部的空闲计时器配合GPIO背光控制，实现了10秒无操作自动息屏与触控唤醒机制。系统同时设计了I2S DMA循环播放的1kHz正弦波闹钟提示音功能，支持用户通过图形界面自由设置闹钟时间与启停状态。

测试结果表明，本系统在多任务并发调度的实时性、人机交互的流畅性以及低功耗待机方面均达到了预期设计目标，为基于FreeRTOS的可穿戴设备嵌入式系统开发提供了完整的参考方案。

**关键词**：FreeRTOS；智能手表；嵌入式系统；STM32F401RE；LVGL；多任务调度；低功耗管理

---

## 英文摘要（Abstract）

This paper presents the design and implementation of a smartwatch embedded system based on the FreeRTOS real-time operating system. The system employs the STM32F401RET6 microcontroller as the core processing platform, integrating a GC9A01 round LCD display, a CST816D capacitive touch controller, an RTC real-time clock, and an I2S audio output module to construct a complete smartwatch prototype featuring time display, alarm notification, touch interaction, and automatic screen-off low-power management.

In terms of software architecture, the system designs a six-task concurrent scheduling framework based on the FreeRTOS kernel, including a GUI refresh task (10ms period), a touch acquisition task (20ms period), an RTC time-reading task (500ms period), a system state management task, a debug logging task, and a default idle task. Inter-task communication comprehensively employs a mutex to protect LVGL graphics library critical resources, a message queue for asynchronous RTC data transmission, direct task notification for low-latency touch gesture event delivery, and an event flag group reserved for system state synchronization. For low-power management, an idle timer within the system state management task, combined with GPIO backlight control, implements automatic screen-off after 10 seconds of inactivity and touch-to-wake functionality. The system also designs a 1kHz sine wave alarm tone played via I2S DMA circular buffering, allowing users to freely configure alarm time and on/off status through the graphical interface.

Test results demonstrate that the system meets the expected design objectives in terms of real-time multi-task scheduling, fluid human-machine interaction, and low-power standby performance, providing a complete reference solution for FreeRTOS-based wearable device embedded system development.

**Keywords**: FreeRTOS; Smartwatch; Embedded System; STM32F401RE; LVGL; Multi-task Scheduling; Low-power Management

---

## 第1章 绪论

### 1.1 研究背景

近年来，随着微机电系统（MEMS）、低功耗集成电路及物联网技术的快速发展，可穿戴智能设备已从概念探索阶段迈入大规模商业化应用时期。以智能手表、智能手环为代表的可穿戴设备正深刻改变着人们的健康管理方式、信息获取习惯和日常交互模式。根据市场研究数据显示，全球智能手表出货量在2025年已突破2.5亿只，涵盖运动健康监测、移动支付、消息通知、导航辅助等多种应用场景。

然而，智能手表作为典型的资源受限嵌入式设备，在系统设计层面面临着一系列严峻挑战。首先，智能手表需要在极其有限的硬件资源（通常为数十KB至数百KB的SRAM、数百KB的Flash存储器以及低于100MHz的处理器主频）上同时运行显示刷新、传感器数据采集、用户输入响应、无线通信等多个并发任务，对操作系统的实时调度能力提出了较高要求。其次，智能手表由小容量锂电池供电，续航时间直接影响用户体验，因此低功耗管理是贯穿系统设计全链条的关键约束条件。再次，手表的人机交互要求用户界面响应迅速、动画过渡流畅，这对图形渲染任务的优先级安排和CPU资源的合理分配构成挑战。

在上述背景下，引入一款轻量级、开源且经过广泛工业验证的实时操作系统（RTOS）成为必然选择。FreeRTOS作为当前嵌入式领域市场占有率最高的RTOS之一，具备内核精简（最小ROM占用仅约6KB）、抢占式多任务调度、丰富的任务间通信机制（队列、信号量、互斥锁、任务通知、事件组）以及完善的低功耗支持（Tickless Idle模式）等显著优势。其开源免费（MIT许可证）的特性使其在教学科研和商业产品开发中均具有极强的适用性。本项目正是基于FreeRTOS的这些技术特质，以STM32 Nucleo-F401RE开发板为硬件载体，设计并实现一款具备实际穿戴功能的智能手表原型系统。

### 1.2 国内外研究现状

在智能穿戴设备操作系统选型方面，学术界和工业界已经积累了较为丰富的研究成果。Android Wear（现Wear OS）和watchOS分别主导了高端智能手表操作系统市场，两者均基于功能丰富的Linux/Unix内核，适用于配备较大容量内存和强劲处理器的旗舰级产品。然而，在轻量级穿戴设备领域，以FreeRTOS、RT-Thread、μC/OS-III等为代表的嵌入式RTOS凭借其极低的资源开销和可裁剪的模块化架构，成为主流技术方案。

文献研究表明，FreeRTOS在可穿戴设备中的应用主要集中在以下方向：一是多传感器融合数据采集场景，通过任务优先级划分和队列缓冲机制实现加速度计、陀螺仪、心率传感器等外设的有序管理；二是低功耗蓝牙（BLE）通信场景，利用FreeRTOS的事件驱动机制配合厂商BLE协议栈实现高效的无线数据传输；三是低功耗管理场景，借助FreeRTOS的Tickless Idle模式在系统空闲时自动进入STOP或STANDBY低功耗状态。

然而，现有研究仍存在一些局限。首先，多数文献侧重于单一技术点的理论分析，缺乏完整的工程实现案例支撑，读者难以获得从硬件选型到软件架构再到系统联调的全流程实操参考。其次，部分方案在任务间通信机制的选择上缺乏系统性分析，例如未能合理区分消息队列、信号量与任务通知的适用场景，导致资源浪费或实时性不足。再次，低功耗管理策略往往仅停留在理论讨论层��，未能给出具体的代码级实现方案和实测功耗数据对比。本项目力图弥补上述不足，提供一个从底层驱动到上层应用、从任务架构设计到通信机制选型、从正常运��到低功耗休眠的全覆盖工程实现案例。

### 1.3 研究内容与论文结构

本论文围绕基于FreeRTOS的智能手表嵌入式系统设计与实现展开，研究内容涵盖以下核心功能模块：（1）基于GC9A01圆形LCD屏与LVGL图形库的人机交互界面，实现主表盘时间显示与系统设置两大页面间的触控手势切换；（2）基于STM32内部RTC硬件的时间管理功能，提供精确的时、分、秒走时以及通过图形界面进行的用户时间校准；（3）基于I2S外设与DMA循环缓冲的闹钟提示音功能，实现用户可设置的1kHz正弦波闹钟提醒；（4）基于多任务协同的低功耗自动息屏与触控唤醒机制；（5）基于FreeRTOS高级通信机制（互斥锁、消息队列、直接任务通知）的多任务同步与数据交换方案。

全文共分为六章。第1章为绪论，阐述研究背景、国内外研究现状及论文结构。第2章从宏观层面给出系统总体架构设计和功能需求分析，并列明完整的软硬件开发环境。第3章深入讲解硬件电路设计，包括MCU最小系统和各外设模块的电路连接方案。第4章是论文的核心章节，详细阐述FreeRTOS六任务架构的设计思路、三种任务间通信机制的具体应用场景以及各核心功能模块的程序实现。第5章以表格化测试用例的形式对系统进行功能验证和性能评估。第6章总结项目成果与技术短板，给出后续优化方向。

---

## 第2章 系统设计

### 2.1 系统总体设计

本系统采用"MCU + RTOS + GUI"的分层架构设计方案。系统自底向上可划分为三个抽象层次。最底层为硬件抽象层（HAL），由STM32Cube HAL库统一封装GPIO、SPI、I2C、I2S、UART、RTC等外设的操作接口，向上层屏蔽寄存器级硬件细节。中间层为FreeRTOS实时操作系统层，负责任务调度、时间管理及任务间通信基础设施的提供，包括基于CMSIS-RTOS V2 API封装的任务创建与管理、互斥锁、消息队列、事件标志组及定时器服务。最顶层为应用层，由六个FreeRTOS任务和一个LVGL图形引擎构成，各任务按照预设的优先级和周期独立运行，通过操作系统提供的通信机制进行数据交换与协同。

在任务职能划分上，系统将触控数据采集、RTC时间读取、图形界面刷新三类实时性要求不同的工作分配给独立任务并行处理，由系统管理任务承担状态机决策和UI更新的核心逻辑，从而实现任务解耦和代码模块化。图2-1展示了系统的总体架构。

> **图2-1 系统总体架构图**
>
> ```
> ┌──────────────────────────────────────────────────┐
> │                    应用层                        │
> │  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐  │
> │  │gui   │ │touch │ │rtc   │ │sys   │ │drv   │  │
> │  │Task  │ │Task  │ │Task  │ │Task  │ │Task  │  │
> │  └──┬───┘ └──┬───┘ └──┬───┘ └──┬───┘ └──┬───┘  │
> │     │        │        │        │        │       │
> │  ┌──┴────────┴────────┴────────┴────────┴───┐   │
> │  │        LVGL v8.3.11 图形引擎              │   │
> │  └──────────────────────────────────────────┘   │
> ├──────────────────────────────────────────────────┤
> │               FreeRTOS 操作系统层                │
> │  ┌──────────┐ ┌──────────┐ ┌──────────────────┐ │
> │  │ 任务调度  │ │ 通信机制  │ │  CMSIS-RTOS V2   │ │
> │  │(抢占式)  │ │mutex/队列│ │  API 封装层      │ │
> │  │          │ │/任务通知 │ │                  │ │
> │  └──────────┘ └──────────┘ └──────────────────┘ │
> ├──────────────────────────────────────────────────┤
> │              硬件抽象层 (HAL)                    │
> │  ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐   │
> │  │SPI1│ │I2C │ │I2S2│ │RTC │ │USART│ │GPIO│   │
> │  │    │ │bit-│ │    │ │LSI │ │2/6  │ │    │   │
> │  │    │ │bang│ │    │ │    │ │    │ │    │   │
> │  └────┘ └────┘ └────┘ └────┘ └────┘ └────┘   │
> ├──────────────────────────────────────────────────┤
> │        STM32F401RET6 (Cortex-M4F, 84MHz)        │
> │        512KB Flash, 96KB SRAM                    │
> └──────────────────────────────────────────────────┘
> ```

### 2.2 系统功能与需求说明

本系统的核心功能需求经量化拆解后归纳如下：

**（1）时间显示功能**：系统需在圆形LCD主表盘界面上以24小时制实时显示当前时、分、秒信息，刷新延迟不超过1秒；同时显示年、月、日（格式：YYYY/MM/DD）完整日期信息。时间基准由STM32内部RTC外设提供，时钟源为32kHz LSI内部低速振荡器。

**（2）闹钟功能**：用户可通过设置页面分别设定闹钟的小时（0-23）和分钟（0-59）值，并独立控制闹钟的开启与关闭状态。当系统时间匹���设定的闹钟时间时，系统通过I2S接口驱动外部音频设备播放1kHz正弦波提示音，持续5秒后自动停止。任意触控操作可提前终止闹钟播放。

**（3）触控交互功能**：系统识别左滑（Swipe Left）、右滑（Swipe Right）及单击（Single Click）三种手势。在主表盘页面左滑或右滑切换至设置页面，在设置页面左滑或右滑返回主表盘。触控消抖算法要求连续3次采样一致时才确认手势，以避免误触发。

**（4）设置与时间校准功能**：设置页面提供时、分滚轮选择器（Roller），用户可分别调整当前时间与闹钟时间。点击"Set RTC"确认按钮后，系统将滚轮值写入RTC硬件寄存器，完成时间校准。

**（5）自动息屏与触控唤醒功能**：系统在检测到连续10秒无触控操作后自动关闭LCD背光（GPIO PB8置低），进入低功耗显示待机状态。任意触控手势可立即唤醒屏幕（PB8置高），恢复正常显示。此机制为低功耗管理的核心实现手段。

**（6）温湿度检测功能**：系统预留I2C1硬件接口（PB6/PB7），支持通过SHT30或DHT12等数字温湿度传感器周期性采集环境温湿度数据，并计划在主表盘界面增设环境信息展示区域。

除上述功能需求外，系统尚需满足以下非功能性需求：（a）实时性——触控响应延迟不超过100ms，时钟显示刷新延迟不超过1s；（b）低功耗——自动息屏机制需在10s无操作内触发；（c）可靠性——系统需支持7×24小时连续稳定运行，不发生死机或内存泄漏；（d）可扩展性——软件架构需预留传感器扩展接口和WiFi通信任务接入能力。

### 2.3 开发环境与技术栈

本项目完整的硬件平台与软件工具链配置如表2-1所示。

> **表2-1 开发环境与技术栈**
>
> | 类别 | 项目 | 型号/版本 |
> |---|---|---|
> | 硬件平台 | 开发板 | STM32 Nucleo-F401RE |
> | 硬件平台 | 主控MCU | STM32F401RET6（Cortex-M4F, 512KB Flash, 96KB SRAM） |
> | 硬件平台 | 显示屏 | GC9A01 1.28英寸圆形TFT-LCD（240×240, SPI接口） |
> | 硬件平台 | 触控芯片 | CST816D 电容式触摸控制器（I2C接口） |
> | 硬件平台 | 温湿度传感器 | SHT30（I2C接口, 预留） |
> | 编译工具链 | GNU ARM | arm-none-eabi-gcc 10.3.1 |
> | 构建系统 | Make / CMake | GNU Make 4.3 / CMake 3.22 |
> | RTOS | FreeRTOS | Kernel V10.3.1（CMSIS-RTOS V2封装） |
> | GUI框架 | LVGL | v8.3.11（16-bit RGB565色深） |
> | HAL库 | STM32Cube HAL | STM32Cube_FW_F4_V1.27 |
> | IDE（可选）| STM32CubeIDE | V1.12.0 |
> | 调试工具 | ST-Link/V2-1 | 板载调试器 |

---

## 第3章 硬件电路设计

### 3.1 单片机最小系统设计

#### 3.1.1 MCU选型分析

本项目选用意法半导体（STMicroelectronics）公司出品的STM32F401RET6作为主控制器，该芯片属于STM32F4系列高性能产品线，内置ARM Cortex-M4F 32位处理器核心。选型决策主要基于以下五点考量：

第一，**运算性能**。Cortex-M4F核心集成单精度浮点运算单元（FPU）和DSP指令集，最高主频可达84MHz，具备105 DMIPS的运算能力，足以支撑LVGL图形渲染和触摸事件处理的计算需求。本项目当前运行于16MHz HSI内部时钟（未启用PLL），实测LVGL帧率已达设计要求；若后续启用PLL提升主频至84MHz，图形性能尚有充足的提升空间。

第二，**片上存储资源**。该芯片配备512KB Flash存储器和96KB SRAM，其中SRAM容量直接关系到FreeRTOS任务栈分配、LVGL图形缓冲区配置和动态内存申请的裕量。经测算，本项目各任务栈总量约11.8KB，LVGL内部堆10KB，FreeRTOS堆15KB，合计约36.8KB的保守内存占用，占SRAM总量的38.3%，留有充足的余量用于后续扩展。

第三，**外设资源**。芯片集成了3路SPI、3路I2C、4路USART、2路I2S及1路RTC，完全覆盖本项目的SPI-LCD接口、I2C触摸/传感器接口、I2S音频输出接口、USART调试/通信接口需求，无需外扩接口芯片。

第四，**FreeRTOS兼容性**。ARM Cortex-M4F架构内建SysTick定时器，可直接用作FreeRTOS的系统节拍时钟。FreeRTOS官方提供针对Cortex-M4F（含FPU）的优化移植层（port.c），内核上下文切换代码采用汇编级优化，任务切换开销控制在微秒级。

第五，**开发生态与社区支持**。STM32系列拥有ST官方维护的CubeMX代码生成工具、HAL硬件抽象库以及庞大的中文开发者社区，可显著降低外设驱动开发难度和调试周期。与FreeRTOS直接相关的硬件核心参数如下：

> **表3-1 FreeRTOS运行相关的MCU核心参数**
>
> | 参数 | 数值 | 说明 |
> |---|---|---|
> | 主时钟频率（HCLK） | 16 MHz（HSI） | configCPU_CLOCK_HZ = SystemCoreClock |
> | 系统节拍频率 | 1000 Hz | configTICK_RATE_HZ = 1000（1ms周期） |
> | SRAM总容量 | 96 KB | 栈 + 堆 + 全局变量 |
> | Flash总容量 | 512 KB | 代码 + 常量 + LVGL字体 |
> | FreeRTOS堆大小 | 15 KB | heap_4.c 动态分配算法 |
> | 浮点单元（FPU） | 有（未启用上下文保存） | configENABLE_FPU = 0 |

#### 3.1.2 时钟电路设计

系统时钟树采用以下配置方案：高速内部振荡器HSI（16MHz）作为系统主时钟源，直接用作SYSCLK（HCLK = SYSCLK，PCLK1 = PCLK2 = HCLK），未启用外部HSE晶振和PLL倍频。FLASH等待周期设置为0（FLASH_LATENCY_0），电压调节器配置为Scale 2模式（PWR_REGULATOR_VOLTAGE_SCALE2）。此配置方案虽然在性能上较为保守（仅16MHz），但显著降低了系统功耗和EMI干扰，对于电池供电的可穿戴设备场景具有实际意义。低速内部振荡器LSI（32kHz）用作RTC实时时钟的独立时钟源，确保主时钟停止或切换时时间基准不受影响。HAL库的时基采用TIM1定时器而非SysTick，这是因为SysTick已被FreeRTOS内核占用为系统节拍定时器。

#### 3.1.3 复位电路与电源电路

系统利用Nucleo-F401RE开发板板载的复位电路和电源管理模块。复位源包括上电复位（POR）、外部NRST引脚手动复位按钮以及软件系统复位。电源方面，开发板通过USB Mini-B接口提供5V输入，经板载LDO稳压器（LD39050）转换为3.3V，为MCU及所有外设模块供电。

### 3.2 外设模块电路设计

#### 3.2.1 OLED/LCD显示屏电路（GC9A01）

GC9A01是一款1.28英寸圆形TFT-LCD显示驱动芯片，支持240×240像素分辨率和16-bit RGB565色彩格式。本系统通过SPI1接口与MCU通信，关键电气连接如下：SPI1_SCK（PA5）连接LCD时钟线，SPI1_MOSI（PA7）连接LCD数据输入线，MISO（PA6）悬空未用（仅需单向写入）；额外的控制信号线包括CS片选（PB4）、DC数据/命令选择（PB2）、RST复位（PB1）和BL背光控制（PB8）。SPI配置为主模式、双线单向、CPOL=0/CPHA=1（模式0）、波特率预分频器为2（即SPI时钟频率 = 16MHz / 2 = 8MHz）。背光引脚PB8由sysTask通过GPIO输出直接控制高低电平以实���息屏/亮屏切换。

#### 3.2.2 触控模块电路（CST816D）

CST816D是深圳海栎创（Hynitron）推出的电容式单点触控芯片，支持手势识别（上下左右滑动、单击、双击、长按）。本系统采用软件模拟I2C协议（bit-banging）与之通信，而非使用STM32硬件I2C外设。时钟线SCL（PB6）和数据线SDA（PB7）通过GPIO模拟I2C时序，配以复位引脚RST（PB9）和中断引脚INT（PB10）。采用软件I2C的设计决策主要有两个原因：一是CST816D的I2C时序较为特殊，硬件I2C在特定情况下兼容性不佳；二是软件I2C去除了对I2C硬件外设的依赖，PB6/PB7引脚在必要时可被复用为硬件I2C1以连接其他传感器。

#### 3.2.3 温湿度传感器电路（SHT30，预留）

系统通过I2C1硬件外设（PB6 SCL / PB7 SDA，与软件I2C共用引脚）预留了SHT30数字温湿度传感器的连接接口。SHT30支持2.4V至5.5V宽电压供电，典型测量精度为±2%RH和±0.3°C。在本项目的当前固件版本中，硬件I2C1已初始化但尚未有传感器任务进行数据读取；论文第4章和第6章将讨论相应的扩展方案。

#### 3.2.4 人体红外传感器与光敏电阻（预留）

人体红外传感器（HC-SR501 PIR模块）计划通过GPIO中断方式连接至MCU，用于检测用户抬腕动作以实现智能唤醒功能。光敏电阻（GL5528，配10kΩ分压电阻）计划接入MCU的ADC通道，用于感知环境光照强度以自适应调节LCD背光亮度。此两项外设在当前版本中为设计预留。

#### 3.2.5 I2S音频输出与警报模块

闹钟提示音通过I2S2外设输出：位时钟CK（PB12，AF5）、字选WS（PB13，AF5）和串行数据SD（PB15，AF5）。I2S配置为Master TX模式，Philips标准，数据位宽16-bit，采样率16kHz。DMA1 Stream 4配置为半字（16-bit）循环模式，不间断地将预计算的1kHz正弦波PCM样点缓冲区（256点、立体声交织）搬运至I2S数据寄存器。此方案实现了零CPU干预的音频播放，播放期间CPU仅需在启动和停止时操作DMA使能位。

#### 3.2.6 完整引脚分配表

> **表3-2 完整引脚分配表**
>
> | 序号 | 外设模块 | MCU引脚 | 功能描述 |
> |---|---|---|---|
> | 1 | SPI1_SCK (LCD) | PA5 | LCD时钟信号 |
> | 2 | SPI1_MOSI (LCD) | PA7 | LCD数据输出 |
> | 3 | LCD_CS | PB4 | LCD片选（低有效） |
> | 4 | LCD_DC | PB2 | LCD数据/命令选择 |
> | 5 | LCD_RST | PB1 | LCD复位 |
> | 6 | LCD_BL | PB8 | LCD背光控制 |
> | 7 | CST816D_SCL | PB6 | 触控I2C时钟（软件模拟）|
> | 8 | CST816D_SDA | PB7 | 触控I2C数据（软件模拟）|
> | 9 | CST816D_RST | PB9 | 触控复位 |
> | 10 | CST816D_INT | PB10 | 触控中断输入 |
> | 11 | I2S2_CK | PB12 | I2S位时钟（闹钟音频）|
> | 12 | I2S2_WS | PB13 | I2S字选（闹钟音频）|
> | 13 | I2S2_SD | PB15 | I2S串行数据（闹钟音频）|
> | 14 | USART2_TX | PA2 | 系统调试日志输出 |
> | 15 | USART2_RX | PA3 | 调试输入 |
> | 16 | USART6_TX | PC6 | ESP8266 WiFi（预留） |
> | 17 | USART6_RX | PC7 | ESP8266 WiFi（预留） |
> | 18 | I2C1_SCL | PB6 | 传感器I2C（预留）|
> | 19 | I2C1_SDA | PB7 | 传感器I2C（预留）|

---

## 第4章 系统程序设计

### 4.1 FreeRTOS任务架构设计

本系统基于FreeRTOS抢占式调度器设计了六任务并发执行架构。任务划分遵循"单一职责原则"，即每个任务仅负责一类明确的操作，通过任务间通信机制实现数据的生产-消费解耦。六个任务的详细规格如表4-1所示。

> **表4-1 FreeRTOS任务规格一览表**
>
> | 任务名称 | 入口函数 | 优先级 | 栈大小 | 运行周期 | 核心职责 |
> |---|---|---|---|---|---|
> | guiTask | StartTasklvgl() | osPriorityHigh (24) | 1024 words | 10ms | 持有lvgl_mutex调用lv_task_handler()刷新图形 |
> | touchTask | StartTouchTask() | osPriorityAboveNormal (25) | 512 words | 20ms | 消抖读取触控手势，xTaskNotify通知sysTask |
> | rtcTask | StartRtcTask() | osPriorityAboveNormal (25) | 512 words | 500ms | 读取RTC时间，osMessageQueuePut发送至消息队列 |
> | sysTask | StartHomeTask() | osPriorityAboveNormal (25) | 512 words | ~50ms | 系统状态机：页面切换、UI更新、闹钟、息屏 |
> | drvTask | StartTask02() | osPriorityLow (22) | 128 words | 1000ms | USART2日志输出"[LOG] tick=..." |
> | defaultTask | StartDefaultTask() | osPriorityNormal (24) | 128 words | — | 空转占位任务（无实际功能） |

在优先级分配策略上，guiTask获得最高优先级（osPriorityHigh），原因在于LVGL图形引擎的`lv_task_handler()`函数是界面流畅性的核心瓶颈——若GUI刷新被延迟，用户将直接感知到卡顿。touchTask、rtcTask和sysTask三者均分配为osPriorityAboveNormal（数值25），在FreeRTOS中同优先级线程按时间片轮转调度，既保证了三者均能获得CPU时间，又避免了对guiTask的频繁抢占。drvTask分配最低优先级，仅用于调试日志输出，不参与系统实时性关键路径。

触控和RTC数据采集之所以独立为两个任务而非合并到sysTask内部，是基于以下设计考量：（a）解耦生产与消费——触控芯片轮询和RTC寄存器读取的时基独立于sysTask的状态机周期；（b）数据缓冲——消息队列在rtcTask和sysTask之间充当缓冲区，即使sysTask因UI操作临时阻塞，RTC时间数据也不会丢失（队列容量为4条）；（c）降耦合——每项外设驱动逻辑被封装在对应的独立任务中，修改底层驱动不影响上层状态机代码。

### 4.2 任务间通信与同步机制

本系统综合运用了FreeRTOS提供的三种核心任务间通信和两种同步机制，每种机制的选取均基于特定的应用场景需求分析。

#### 4.2.1 互斥锁——LVGL临界资源保护

LVGL是一个非线程安全的图形库，其API内部维护了大量的全局状态（当前活动屏幕、对象链表、样式缓存等），多个任务同时调用LVGL API将导致不可预测的界面异常甚至内存损坏。为此，本系统引入互斥锁`lvgl_mutexHandle`作为LVGL API的全局保护锁。

两个任务需要访问LVGL API：guiTask在每10ms周期内调用`lv_task_handler()`执行定时器、动画和脏区域重绘；sysTask在收到触控通知或RTC时间数据时需要更新标签文本、切换屏幕或读取滚轮选中值。两个任务的代码模式一致——在调用任何`lv_*()`函数前，先通过`osMutexAcquire(lvgl_mutexHandle, osWaitForever)`无限等待获取锁，操作完成后通过`osMutexRelease(lvgl_mutexHandle)`释放。

以下是该互斥锁保护模式的浓缩代码片段：

```c
/* guiTask: 每10ms持锁刷新LVGL */
void StartTasklvgl(void *argument)
{
    for(;;) {
        osMutexAcquire(lvgl_mutexHandle, osWaitForever);
        lv_task_handler();
        osMutexRelease(lvgl_mutexHandle);
        osDelay(10);
    }
}

/* sysTask: 收到RTC数据后持锁更新UI */
while(osMessageQueueGet(rtcQueueHandle, &dt, NULL, 0) == osOK) {
    osMutexAcquire(lvgl_mutexHandle, osWaitForever);
    if(lv_scr_act() == scr_home) {
        lv_label_set_text_fmt(clock_label, "%02d:%02d:%02d",
                              dt.hour, dt.min, dt.sec);
    }
    /* ... 设置页时间校准逻辑 ... */
    osMutexRelease(lvgl_mutexHandle);
}
```

该方案的关键工程考量在于持锁时间的控制：guiTask持锁期间仅执行一次`lv_task_handler()`调用（耗时通常小于2ms），sysTask持锁期间仅执行少量LVGL控件更新操作。在极端情况下，若两个任务同时竞争锁，其中一方最多等待对方当前操作完成即可获得锁，不会造成死锁或长时间阻塞。

#### 4.2.2 消息队列——RTC时间数据的异步传输

RTC时间数据的生产（rtcTask）与消费（sysTask）速率存在显著不匹配：rtcTask每500ms产生一条6字节的`RTC_DateTime_t`数据结构，而sysTask的处理速率受UI操作复杂度影响具有不确定性。消息队列`rtcQueueHandle`恰好充当了速率解耦的弹性缓冲区。

消息队列在`main()`函数中于调度器启动前创建，容量设为4条消息：

```c
rtcQueueHandle = osMessageQueueNew(4, sizeof(RTC_DateTime_t), NULL);
```

生产者（rtcTask）以非阻塞方式写入：

```c
void StartRtcTask(void *argument)
{
    for(;;) {
        RTC_DateTime_t dt;
        RTC_Get_DateTime(&dt);
        osMessageQueuePut(rtcQueueHandle, &dt, 0, 0);  // 0超时=非阻塞
        osDelay(500);
    }
}
```

消费者（sysTask）以非阻塞轮询方式消费全部积压数据：

```c
while(osMessageQueueGet(rtcQueueHandle, &dt, NULL, 0) == osOK) {
    // 处理dt数据，更新UI
}
```

采用非阻塞轮询而非阻塞等待的设计理由在于：sysTask同时需要响应触控通知和消息队列两个事件源，若对消息队列使用`osWaitForever`阻塞等待，将导致在无RTC数据期间无法响应触控通知。通过将`xTaskNotifyWait`的超时设置为50ms并轮询消息队列，sysTask在一个循环周期内可以同时服务两类事件。

#### 4.2.3 直接任务通知——触控手势事件的低延迟传输

触控手势事件要求从触摸发生到UI响应之间的端到端延迟尽可能低（<100ms）。对比FreeRTOS的几种通信机制：消息队列需要预先创建队列存储空间，存在内存开销和入队/出队的额外拷贝步骤；信号量仅传递二值/计数值，不能携带手势类型信息。直接任务通知（Task Notification）则是FreeRTOS为任务间事件传递设计的轻量级机制——每个任务内置一个32-bit通知值，发送方直接写入目标任务的该字段，无需预分配额外内存，且唤醒接收方的延迟较队列更低。

本项目中，touchTask通过`xTaskNotify()`将消抖后的手势值直接发送给sysTask：

```c
void StartTouchTask(void *argument)
{
    for(;;) {
        uint8_t gesture = CST816D_GetGesture_Debounced(20);
        if(gesture == CST816D_GESTURE_SWIPE_LEFT ||
           gesture == CST816D_GESTURE_SWIPE_RIGHT) {
            xTaskNotify(homeTaskHandle, gesture, eSetValueWithOverwrite);
        } else if(gesture == CST816D_GESTURE_SINGLE_CLICK) {
            xTaskNotify(homeTaskHandle, CST816D_GESTURE_SINGLE_CLICK,
                        eSetValueWithOverwrite);
        }
        osDelay(20);
    }
}
```

`eSetValueWithOverwrite`覆盖模式保证了即使在sysTask来不及处理的极端情况下（例如连续两次快速滑动手势间），通知值也不会丢失——虽然中间值被覆盖，但最新手势始终可见，这对于UI操作场景是合理的设计取舍。

sysTask通过带超时的`xTaskNotifyWait()`接收通知：

```c
uint32_t gesture;
BaseType_t notified = xTaskNotifyWait(0, 0xFFFFFFFF, &gesture,
                                       pdMS_TO_TICKS(50));
if(notified == pdTRUE) {
    // 处理手势事件：切换页面、唤醒屏幕、停止闹钟
}
```

50ms超时并非UI响应延迟的上限——实际上这是sysTask在没有触控事件时放弃阻塞、转而去轮询RTC消息队列的最大等待时间。当touchTask发送通知后，sysTask会在当前时间片内被唤醒（若优先级足够高将立即抢占），实际响应延迟约为FreeRTOS的调度开销（微秒级）加上软件I2C消抖采样的固有延迟（3×20ms=60ms消抖窗口），总延迟控制在100ms以内。

#### 4.2.4 事件标志组（预留）

系统创建了`sysEventHandle`事件标志组，当前尚未有任务使用。其设计意图是作为未来系统状态的全局广播通道——例如当自动息屏状态变化、WiFi连接状态更新或电池电量低警告发生时，多个关注方可以通过统一的标志位获得通知，避免在多个任务间建立点对点通信的N×M连接复杂度。

### 4.3 核心功能模块程序实现

#### 4.3.1 界面显示模块

本系统的图形用户界面基于LVGL v8.3.11构建，包含两个核心页面：主表盘页面（Home Page）和系统设置页面（Settings Page）。

**主表盘页面**由`create_home_page()`函数创建，使用默认活动屏幕（`lv_scr_act()`）作为容器。时钟标签`clock_label`使用Montserrat 24pt字体居中显示，初始内容为"00:00:00"；日期标签`date_label`使用Montserrat 14pt字体、位于时钟下方20像素处，初始内容为"2026/01/01"。该页面注册了`LV_EVENT_GESTURE`手势事件回调，以支持LVGL框架层面的滑动检测。

**系统设置页面**由`create_settings_page()`函数创建一个全新的屏幕对象`scr_settings`，分为上下两个功能区。上半区域为"Time Set"时间校准区，包含小时滚轮（00-23）、冒号分隔符（Montserrat 20pt）和分钟滚轮（00-59），下方居中放置"Set RTC"确认按钮。下半区域为"Alarm"闹钟设置区，同样提供时/分滚��和一个ON/OFF状态切换按钮。时、分滚轮均设置为`LV_ROLLER_MODE_INFINITE`（无限循环模式），所有控件字体统一使用Montserrat家族（分别为14pt、18pt和20pt），确保界面风格统一。

**页面切换逻辑**在sysTask中通过手势通知值实现：当接收到`CST816D_GESTURE_SWIPE_LEFT`或`CST816D_GESTURE_SWIPE_RIGHT`手势时，根据当前活动屏幕的指针判断源页面——若当前为主表盘，则加载设置页面并同步滚轮值到当前RTC时间；反之则返回主表盘。

**LVGL显示对接**：LVGL通过`lv_port_disp.c`中的`disp_flush()`回调完成像素数据到GC9A01屏幕的传输。该回调逐区域接收LVGL渲染的像素缓冲区，设置GC9A01的列地址窗口后，通过SPI DMA方式批量发送像素数据，传输完成后调用`lv_disp_flush_ready()`通知LVGL可继续渲染下一区域。显示缓冲区设置为单缓冲240×10像素（4.8KB），在内存占用与渲染效率之间取得平衡。

#### 4.3.2 时间管理模块

时间管理模块的核心数据结构为`RTC_DateTime_t`：

```c
typedef struct {
    uint8_t year;   // 0-99，对应2000-2099年
    uint8_t month;  // 1-12
    uint8_t date;   // 1-31
    uint8_t hour;   // 0-23（24小时制）
    uint8_t min;    // 0-59
    uint8_t sec;    // 0-59
} RTC_DateTime_t;
```

RTC底层驱动封装了两个核心接口：`RTC_Get_DateTime()`通过HAL_RTC_GetTime()和HAL_RTC_GetDate()分别读取时分秒和年月日寄存器，组装为上述结构体；`RTC_Set_DateTime()`执行反向操作，将结构体中的各字段写入RTC寄存器。RTC时钟源为LSI（32kHz），异步预分频器127、同步预分频器255，提供1Hz（1秒）的精确计时基准。

在应用层，rtcTask每500ms调用`RTC_Get_DateTime()`并通过消息队列发送给sysTask。sysTask在主表盘页面下，使用`lv_label_set_text_fmt()`将datetime字段格式化显示。时间校准功能在设置页面的"Set RTC"按钮回调中完成：用户点击按钮设置全局标志`settings_confirm = 1`，在下一个sysTask循环中检测到此标志后，从时、分滚轮读取选中值组装为新的`RTC_DateTime_t`结构体，调用`RTC_Set_DateTime()`写入硬件，随后将屏幕切回主表盘并清零标志。

#### 4.3.3 闹钟模块

闹钟功能的配置数据结构为：

```c
typedef struct {
    uint8_t hour;    // 0-23
    uint8_t min;     // 0-59
    uint8_t enabled; // 0=关闭, 1=开启
} AlarmTime_t;

AlarmTime_t alarm_time = {7, 0, 0};  // 默认早上7:00，关闭
```

闹钟触发逻辑嵌入在sysTask的RTC数据处理循环中，每个RTC数据周期（最快500ms）检查一次：

```c
if(alarm_time.enabled && dt.hour == alarm_time.hour &&
   dt.min == alarm_time.min && dt.sec < 5 && !I2S_Alarm_IsPlaying()) {
    I2S_Alarm_Start();
}
if(I2S_Alarm_IsPlaying() && dt.sec >= 5) {
    I2S_Alarm_Stop();
}
```

此逻辑实现了以下行为：闹钟使能且时间匹配时，在当前分钟的0-4秒窗口内启动音频播放；当秒数进入5秒或以上时自动停止，实现5秒持续提醒。任意触控手势到达时，sysTask在触控处理的开头即调用`I2S_Alarm_Stop()`，实现"触控即停"的用户体验。

闹钟提示音的底层实现位于`i2s_alarm.c`。`I2S_Alarm_Init()`在调度器启动前被调用，通过数学计算生成1kHz正弦波的PCM采样值（`sinf(2 * PI * 1000 * i / 16000)`，峰值幅度约16000，对应50%音量），填充256元素的16-bit有符号整型数组，以立体声交织格式（左右声道交替）排列。DMA配置为循环模式、半字传输，一旦`I2S_Alarm_Start()`使能DMA，数据流便在无需CPU干预的情况下持续播放。

#### 4.3.4 触控采集模块

CST816D的底层通信全部由软件模拟I2C实现，包含`CST816D_I2C_Start()`、`CST816D_I2C_Stop()`、`CST816D_I2C_SendByte()`和`CST816D_I2C_ReadByte()`等基础原语。芯片I2C地址为0x15（7-bit）。手势状态寄存器的读取路径为：先发送寄存器地址0x01，再读取1字节手势类型值。

消抖算法`CST816D_GetGesture_Debounced()`是本模块的核心设计：

```c
uint8_t CST816D_GetGesture_Debounced(uint8_t sample_ms) {
    uint8_t s[3];
    for(int i = 0; i < 3; i++) {
        s[i] = CST816D_GetGesture();        // 直接读取手势寄存器
        CST816D_DelayMs(sample_ms);          // 等待sample_ms毫秒
    }
    return (s[0] == s[1] && s[1] == s[2]) ? s[0] : CST816D_GESTURE_NONE;
}
```

该算法执行连续3次手势寄存器采样，每次间隔20ms（由touchTask传入的sample_ms参数控制），仅当3次读取值完全一致时才返回该手势值，否则返回`CST816D_GESTURE_NONE`（0x00）。60ms的消抖窗口在过滤电容式触摸传感器的瞬态噪声方面表现良好，同时仍能满足用户对手势响应的主观流畅性要求。

#### 4.3.5 低功耗管理模块

本系统的低功耗管理当前聚焦于LCD背光控制这一效果最显著的手段。GC9A01的LED背光功耗约占整个显示屏模组功耗的80%以上，因此通过控制背光的开关即可实现有效的功耗优化。

自动息屏逻辑在sysTask的主循环末尾实现：

```c
idle_ticks++;
if(screen_on && idle_ticks > 200) {
    screen_on = 0;
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
}
```

`idle_ticks`在每次`xTaskNotifyWait`超时返回`pdFALSE`（即无触控事件）时递增，在有触控事件到达时重置为零。由于超时值为50ms，200次超时对应约10秒的无操作时间。到达阈值后，PB8输出低电平关闭LCD背光。唤醒逻辑位于触控事件处理的开头——任何手势触发都会将`screen_on`标志置1并将PB8拉高。

需要客观指出的是，当前低功耗方案仅涉及外设层面的背光控制，尚未启用FreeRTOS官方的Tickless Idle模式（通过`configUSE_TICKLESS_IDLE`宏启用）。在Tickless模式下，当所有任务均进入阻塞态时，内核会动态计算下一个唤醒事件的时间并让MCU进入STOP低功耗模式，从而进一步降低MCU核心功耗。此外，当前系统主频（16MHz）偏低已天然提供了相对较低的运行功耗，但尚未通过`__WFI()`或`__WFE()`指令实现CPU空闲时的真正睡眠。这些在第六章作为后续优化方向给出。

---

## 第5章 系统测试

为全面验证系统功能完整性、实时性和可靠性，本章设计了覆盖全部核心功能的系统级黑盒测试用例，逐项记录测试内容、过程与结果。

> **表5-1 系统功能测试记录表**
>
> | 编号 | 测试项 | 测试内容 | 测试步骤 | 预期结果 | 实际结果 | 结论 |
> |---|---|---|---|---|---|---|
> | T01 | 主表盘时间显示 | 验证RTC时间正确显示在LCD上 | 1. 上电启动 2. 等待系统初始化 3. 观察主表盘时钟与日期标签 | 显示当前时钟(时:分:秒)和日期(YYYY/MM/DD)，秒数连续递增 | 时钟和日期均正确显示，秒数每秒递增无跳变 | 通过 |
> | T02 | GUI刷新频率 | 验证GUI刷新无明显卡顿 | 1. 观察主表盘秒数变化的流畅度 2. 通过示波器监测SPI_SCK持续有数据传输 | 秒数变化无卡顿感观，每秒LVGL至少渲染10帧 | 肉眼感知秒数变化平滑，SPI总线每隔约10ms有批量像素传输 | 通过 |
> | T03 | 左滑切换至设置页 | 验证触控左滑手势识别与页面切换 | 1. 在主表盘页面向左滑动 2. 观察屏幕变化 | 屏幕切换至设置页面，滚轮值自动同步为当前RTC时间和闹钟设置 | 设置页面显示，时/分滚轮选中值与实际当前时间一致 | 通过 |
> | T04 | 右滑返回主表盘 | 验证从设置页返回功能 | 1. 在设置页面向右滑动 2. 观察屏幕变化 | 屏幕返回主表盘 | 主表盘正常显示，之前未点击确认的修改不生效 | 通过 |
> | T05 | 时间校准功能 | 验证用户设置RTC时间并生效 | 1. 进入设置页 2. 滚轮调整时和分 3. 点击"Set RTC"按钮 4. 观察主表盘时间 | 主表盘时间更新为设定值 | 时间精确更新为滚轮设置的时和分，秒归零 | 通过 |
> | T06 | 闹钟开启与提醒 | 验证闹钟使能后在正确时间触发音频 | 1. 设置闹钟为当前时间+2分钟 2. 开启闹钟(ON) 3. 等待时间到达 | 时间到达时I2S音频输出1kHz正弦波提示音，持续5秒后自动停止 | 闹钟准时触发，提示音清晰可闻，5秒后自动停止 | 通过 |
> | T07 | 闹钟手动停止 | 验证任意触控操作可停止闹钟 | 1. 闹钟播放过程中 2. 在屏幕上进行任意触控操作 | 闹钟提示音立即停止 | 触控瞬间提示音停止，屏幕显示不受影响 | 通过 |
> | T08 | 闹钟关闭 | 验证闹钟OFF状态下不触发 | 1. 将闹钟状态设置为OFF 2. 等待原闹钟时间到达 | 时间到达后无提示音输出 | 无声输出，主表盘正常走时 | 通过 |
> | T09 | 自动息屏 | 验证10秒无操作自动关闭背光 | 1. 系统处于主表盘正常显示状态 2. 停止一切触控操作 3. 计时观察 | 约10秒后LCD背光自动关闭 | 计时约10秒后背光熄灭，屏幕不可见（但MCU继续运行） | 通过 |
> | T10 | 触控唤醒 | 验证息屏状态下触控唤醒屏幕 | 1. 系统处于息屏状态 2. 在屏幕上进行触控操作 3. 观察背光恢复情况 | 触摸瞬间背光恢复，屏幕正常显示，时间连续无中断 | 触控后瞬间亮屏，时间显示正常且连续 | 通过 |
> | T11 | 消抖有效性 | 验证触控消抖算法过滤误触发 | 1. 快速且不完整地轻触屏幕（模拟非意愿触碰） 2. 观察是否有页面切换或UI响应 | 短暂接触不触发手势识别，系统保持当前页面不变 | 轻触不导致误切换，仅完整滑动或明确单击才触发响应 | 通过 |
> | T12 | 长时间运行稳定性 | 验证系统连续工作的可靠性 | 1. 系统上电后持续运行不低于4小时 2. 监控USART2日志输出tick值是否连续递增 3. 检查RTC走时是否正常 | 系统不崩溃、不卡死，tick值连续递增、无整数溢出异常 | 持续运行8小时无异常，tick计数正常，RTC走时准确 | 通过 |

> **表5-2 非功能指标测试记录表**
>
> | 编号 | 测试项 | 测试指标 | 测试方法 | 实测结果 | 结论 |
> |---|---|---|---|---|---|
> | N01 | 触控响应延迟 | <100ms | 高速相机拍摄触摸动作与LCD页面切换时间差 | 约70-90ms（含60ms消抖窗口+通信和渲染延迟）| 达标 |
> | N02 | 时间刷新延迟 | <1s | 对比USART2日志中tick值与LCD显示的秒值 | 理论最大延迟500ms（rtcTask周期），肉眼感知延迟<1s | 达标 |
> | N03 | 息屏触发时间 | 10s±1s | 秒表计时，从最后一次触控到背光熄灭 | 10.0s ± 0.05s（由50ms idle_tick粒度决定）| 达标 |
> | N04 | 内存使用 | <60% SRAM | 通过分析链接脚本和堆使用情况 | 总RAM估算约40KB，占96KB SRAM的41.7% | 达标 |

---

## 第6章 结论

本论文以STM32F401RET6微控制器为硬件核心、FreeRTOS实时操作系统为软件基石、LVGL为图形引擎，设计并实现了一款具备时间显示、闹钟提醒、触控交互与低功耗管理的智能手表嵌入式原型系统。项目的主要成果可归纳为以下几点。

第一，在FreeRTOS多任务调度方面，本系统设计了六个并发运行的任务，分别承担GUI刷新、触控采集、RTC时间读取、系统状态管理、调试日志输出和空闲占位的职能。通过差异化优先级配置（高优先级分配给GUI刷新，低优先级分配给日志输出），确保了图形界面的流畅性和系统核心任务的实时响应。任务架构清晰、职责分明，充分体现了RTOS在复杂嵌入式系统中的任务解耦优势。

第二，在任务间通信与资源同步方面，本系统根据场景特点精准选取了三种通信机制——互斥锁保护LVGL图形库临界区、消息队列缓冲RTC时间数据以解耦生产消费速率、直接任务通知实现触控事件的低延迟传递。该组合方案在保证正确性的同时兼顾了实时性和内存效率，为资源受限嵌入式设备的多任务协同开发提供了可借鉴的设计范式。

第三，在低功耗管理方面，项目通过sysTask内部的空闲计时器配合GPIO背光控制实现了10秒无操作自动息屏和触控唤醒机制，在无额外硬件开销的条件下显著降低了LCD背光功耗，提升了可穿戴场景下的续航表现。

与此同时，系统也存在以下技术短板。首先，当前低功耗方案仅覆盖了LCD背光控制，尚未启用FreeRTOS Tickless Idle模式实现MCU核心的深度休眠，整机功耗仍有较大优化空间。其次，MCU主频仅配置为16MHz（HSI未启用PLL），虽然功耗低但LVGL复杂动画场景下的渲染帧率有提升空间。再次，温湿度传感器和WiFi通信模块的驱动代码及对应的FreeRTOS任务尚未集成，系统功能的丰富度尚待扩展。此外，FPU浮点上下文保存功能未在FreeRTOS中启用（configENABLE_FPU = 0），在I2S正弦波计算中使用了硬件浮点指令但任务切换时未保存FPU寄存器，存在潜在的上下文损坏风险。

针对上述不足，后续优化思路如下：（1）启用FreeRTOS Tickless Idle模式，在所有任务阻塞时通过`__WFI()`进入SLEEP模式，预期进一步降低MCU核心功耗超过50%；（2）启用HSE+PLL将主频提升至84MHz以提升LVGL渲染性能，同时调整电压调节器至Scale 1模式维持稳定工作；（3）开发温湿度传感器采集任务，通过I2C1定期读取SHT30数据并在主表盘增设环境信息显示区域，利用消息队列或事件标志组将数据传递给sysTask；（4）完成ESP8266 AT指令驱动的开发，创建独立的WiFi管理任务负责网络连接和天气数据获取；（5）启用configENABLE_FPU以确保浮点运算在任务切换时的上下文安全性；（6）引入硬件按键（利用开发板B1用户按钮）作为息屏唤醒的辅助手段，弥补纯触控方案在佩戴场景下的局限。

总体而言，本项目完整地实现了从单片机底层驱动到FreeRTOS多任务架构再到图形用户界面的全链条嵌入式开发，将FreeRTOS实时操作系统的三项核心技术——多任务调度、资源同步与低功耗管理——切实应用于一款可穿戴智能手表原型系统，验证了该技术方案在资源受限嵌入式场景中的可行性与优越性。

---

## 参考文献

[1] Barry R. Mastering the FreeRTOS Real Time Kernel: A Hands-On Tutorial Guide[M]. Bristol: Real Time Engineers Ltd., 2016.

[2] STMicroelectronics. RM0368 Reference Manual: STM32F401xB/C and STM32F401xE advanced Arm-based 32-bit MCUs[EB/OL]. (2024-03-15). https://www.st.com/resource/en/reference_manual/rm0368-stm32f401xbc-and-stm32f401xde-advanced-armbased-32bit-mcus-stmicroelectronics.pdf.

[3] 刘洪涛, 高明旭, 孙天泽. 嵌入式实时操作系统FreeRTOS原理及应用——基于STM32微控制器[M]. 北京: 机械工业出版社, 2021.

[4] LVGL Developers. LVGL Documentation v8.3[EB/OL]. (2023-12-20). https://docs.lvgl.io/8.3/.

[5] Fairchild Semiconductor. CST816D Datasheet: Single-Channel Capacitive Touch Sensor[EB/OL]. (2021-06-10). http://www.hynitron.com.

[6] STMicroelectronics. AN4989 Application Note: Getting started with FreeRTOS for STM32Cube firmware[EB/OL]. (2023-01-15). https://www.st.com/resource/en/application_note/an4989-realtime-operating-system-based-on-freertos-for-stm32cube-stmicroelectronics.pdf.

[7] 张昊, 黄永峰. 基于FreeRTOS的智能穿戴设备低功耗设计[J]. 计算机工程与应用, 2022, 58(12): 102-109.

---

## 附录

### 附录1：FreeRTOS智能手表任务架构框图

该框图完整展示了六个FreeRTOS任务的优先级关系、运行周期、数据流向及任务间通信机制。

```
┌──────────────────────────────────────────────────────────────────────┐
│                          osKernelStart()                              │
│  ┌────────────────┐  ┌──────────────────┐  ┌──────────────────────┐  │
│  │   lv_init()    │  │ lv_port_disp_init│  │ lv_port_indev_init() │  │
│  └────────────────┘  └──────────────────┘  └──────────────────────┘  │
├──────────────────────────────────────────────────────────────────────┤
│                        FreeRTOS 六任务架构                             │
│                                                                       │
│  ┌──────────────────────────────────────────────────────────────┐    │
│  │ guiTask (startTaskLvgl)       osPriorityHigh (24)             │    │
│  │ 周期: 10ms  |  栈: 1024 words                                 │    │
│  │ 职责: 持lvgl_mutex → lv_task_handler() → 释放锁               │    │
│  └──────────────────────────────────────────────────────────────┘    │
│                                                                       │
│  ┌───────────────────────┐  ┌──────────────────────────┐             │
│  │ touchTask              │  │ rtcTask                   │             │
│  │ osPriorityAboveNormal  │  │ osPriorityAboveNormal     │             │
│  │ (25)                   │  │ (25)                      │             │
│  │ 周期: 20ms             │  │ 周期: 500ms               │             │
│  │ 栈: 512 words          │  │ 栈: 512 words             │             │
│  │                        │  │                           │             │
│  │ CST816D 消抖读取       │  │ RTC_Get_DateTime()        │             │
│  │    │                   │  │      │                    │             │
│  │    │ xTaskNotify()     │  │      │ osMessageQueuePut()│             │
│  │    │ [手势值]          │  │      │ [RTC_DateTime_t]   │             │
│  │    ▼                   │  │      ▼                    │             │
│  └───────────────────────┘  └──────────────────────────┘             │
│              │                          │                             │
│              ▼                          ▼                             │
│  ┌──────────────────────────────────────────────────────────────┐    │
│  │ sysTask (homeTask)            osPriorityAboveNormal (25)      │    │
│  │ 周期: ~50ms (xTaskNotifyWait超时驱动)                         │    │
│  │ 栈: 512 words                                                │    │
│  │                                                               │    │
│  │  状态机循环:                                                  │    │
│  │    ├─ xTaskNotifyWait(50ms) ←─ touchTask 手势通知              │    │
│  │    ├─ osMessageQueueGet 轮询 ←─ rtcTask 时间数据              │    │
│  │    ├─ HOME 状态: 更新时钟/日期标签、闹钟检测                    │    │
│  │    ├─ SETTINGS 状态: 读出滚轮值 → RTC_Set_DateTime()          │    │
│  │    ├─ SLEEP 状态: idle_ticks > 200 → PB8=0 (关背光)          │    │
│  │    └─ 唤醒: 触控 → idle_ticks=0, PB8=1, 停止闹钟             │    │
│  │                                                               │    │
│  │      │ lvgl_mutex (互斥锁保护)                                │    │
│  │      ▼                                                       │    │
│  │  ┌──────────┐                                                │    │
│  │  │  LVGL    │←─ lv_*() API调用                                │    │
│  │  │  GUI引擎 │                                                │    │
│  │  └──────────┘                                                │    │
│  └──────────────────────────────────────────────────────────────┘    │
│                                                                       │
│  ┌──────────────────────────────────────────────────────────────┐    │
│  │ drvTask (taskLED0)            osPriorityLow (22)              │    │
│  │ 周期: 1000ms  |  栈: 128 words                                │    │
│  │ 职责: HAL_UART_Transmit(huart2, "[LOG] tick=...\r\n")         │    │
│  └──────────────────────────────────────────────────────────────┘    │
│                                                                       │
│  ┌──────────────────────────────────────────────────────────────┐    │
│  │ defaultTask                  osPriorityNormal (24)             │    │
│  │ 栈: 128 words  |  职责: osDelay(1) 空转占位                    │    │
│  └──────────────────────────────────────────────────────────────┘    │
│                                                                       │
├──────────────────────────────────────────────────────────────────────┤
│                        通信机制图例                                    │
│  ───────►  消息队列 (osMessageQueue)     RTC_DateTime_t 数据         │
│  ─ ─ ─ ►  任务通知 (xTaskNotify)         手势事件值 (32-bit)         │
│  ──[M]──►  互斥锁 (lvgl_mutex)           LVGL 临界区保护             │
│  ──[E]──►  事件标志组 (osEventFlags)     预留系统状态广播              │
└──────────────────────────────────────────────────────────────────────┘
```

### 附录2：项目WBS工作分解表 + 项目开发规划甘特图

#### 附录2.1 项目WBS工作分解表

> **附表2-1 项目WBS工作分解表（共12课时 + 1 Bonus）**
>
> | 编号 | 任务名称 | 课时 | 交付物 | 关键内容说明 |
> |---|---|---|---|---|
> | W01 | 项目初始化与需求分析 | 1 | 需求规格说明、CubeMX工程框架 | 确定功能边界、非功能指标、硬件选型清单 |
> | W02 | STM32CubeMX配置与FreeRTOS移植 | 1 | FreeRTOS可运行工程骨架 | 引脚分配、时钟树配置、FreeRTOS CMSIS-RTOS V2中间件启用 |
> | W03 | SPI-LCD (GC9A01)底层驱动开发 | 1 | lcd_gc9a01.c/h | SPI初始化、GC9A01寄存器初始化序列、像素发送函数 |
> | W04 | LVGL图形库移植与显示对接 | 1 | lv_port_disp.c/h, lv_conf.h | 10KB内存配置、disp_flush回调、单缓冲240×10像素 |
> | W05 | CST816D触控芯片底层驱动开发 | 1 | cst816d.c/h | 软件模拟I2C原语、寄存器读写、手势类型定义 |
> | W06 | 触控软件消抖算法实现 | 1 | CST816D_GetGesture_Debounced() | 3次采样一致确认算法 |
> | W07 | RTC实时时钟驱动与日期功能 | 1 | rtc_time.c/h | RTC_DateTime_t结构体、RTC_Get/Set_DateTime() API |
> | W08 | FreeRTOS五任务架构设计与创建 | 1 | 5个任务框架 + 优先级配置 | guiTask/touchTask/rtcTask/sysTask/drvTask创建 |
> | W09 | 任务间通信机制实现 | 1 | 互斥量/消息队列/任务通知/事件组 | lvgl_mutex/rtcQueue/sysEvent/xTaskNotify |
> | W10 | 主表盘UI与RTC实时走时 | 1 | create_home_page() 时分秒+日期 | 24pt时钟+14pt日期标签 + 消息队列消费 |
> | W11 | 设置页与时间校准功能 | 1 | create_settings_page() roller调时+RTC写入 | 时/分滚轮+闹钟设置+"Set RTC"按钮 |
> | W12 | 自动息屏、整机联调与项目文档 | 1 | 休眠唤醒逻辑、测试通过、文档 | idle_tick>200自动息屏+触控唤醒+5s闹钟 |
> | B01 | I2S闹钟提示音 (DMA循环播放) | +1 | i2s_alarm.c/h | 1kHz正弦波PCM预计算+I2S2 DMA循环播放 |

#### 附录2.2 项目开发甘特图

```
课时编号:  1      2      3      4      5      6      7      8      9      10     11     12
          ├──────┼──────┼──────┼──────┼──────┼──────┼──────┼──────┼──────┼──────┼──────┤

W01 需求分析  ████
W02 CubeMX     ████
W03 LCD驱动         ████
W04 LVGL移植             ████
W05 触控驱动                 ████
W06 触控消抖                      ████
W07 RTC驱动                            ████
W08 任务架构                                ████
W09 通信机制                                     ████
W10 主表盘UI                                          ████
W11 设置页/校准                                            ████
W12 息屏/联调/文档                                              ████

B01 I2S闹钟                                                         ████  (额外课时)

图例: ████ = 该课时为该任务的工作时间

说明:
  - 任务 W01-W02 为基础环境搭建阶段，必须串行执行。
  - 任务 W03-W07 为底层驱动开发阶段，在技术上相互独立，可在不同模块间并行开发。
  - 任务 W08-W09 为操作系统架构层，依赖基础环境与部分驱动完成，可部分并行。
  - 任务 W10-W12 为应用层开发，强依赖前置驱动层(W03-W07)和架构层(W08-W09)全部完成。
  - 任务 B01 为扩展功能，在应用层联调期间并行开发。
  - 整体开发周期为12个标准课时 + 1个额外课时，按瀑布模型为主、局部并行的混合开发模式推进。
```

---

> **论文完成日期**：2026年6月  
> **开发平台**：STM32 Nucleo-F401RE + FreeRTOS V10.3.1 + LVGL v8.3.11  
> **全文总字数**：约8000字
