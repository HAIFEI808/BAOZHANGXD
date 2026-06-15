# 基于 FreeRTOS 的智能手表嵌入式系统

基于 STM32F401RE + FreeRTOS + LVGL 的智能手表原型系统，具备时间显示、闹钟提醒、触控交互与自动息屏低功耗管理功能。

## 硬件平台

| 模块 | 型号 | 接口 |
|------|------|------|
| 主控 | STM32F401RET6 (Cortex-M4F, 84MHz, 512KB Flash, 96KB SRAM) | — |
| 开发板 | STM32 Nucleo-F401RE | — |
| 显示屏 | GC9A01 1.28" 圆形 TFT-LCD (240×240) | SPI1 |
| 触控 | CST816D 电容式触摸 | 软件 I2C |
| 音频 | I2S 外接 DAC / 功放 | I2S2 |
| 时钟 | 内部 RTC (LSI 32kHz) | — |

## 功能特性

- **时间显示** — 24 小时制时:分:秒 + 日期 (YYYY/MM/DD)，由内部 RTC 提供精确走时
- **闹钟提醒** — 用户可设置闹钟时间与开关，触发时通过 I2S DMA 播放 1kHz 正弦波提示音
- **触控交互** — 左滑/右滑切换主表盘与设置页，单击唤醒屏幕
- **时间校准** — 设置页提供滚轮选择器调整时间，一键写入 RTC 硬件
- **自动息屏** — 10 秒无操作自动关闭背光，触控即唤醒
- **调试日志** — USART2 周期输出系统 tick 日志

## 软件架构

```
┌─────────────────────────────────────┐
│              应用层                  │
│  guiTask  touchTask  rtcTask        │
│  sysTask  drvTask   defaultTask     │
│  ┌──────────────────────────────┐   │
│  │     LVGL v8.3.11 图形引擎    │   │
│  └──────────────────────────────┘   │
├─────────────────────────────────────┤
│         FreeRTOS V10.3.1            │
│  互斥锁 | 消息队列 | 任务通知 | 事件组 │
├─────────────────────────────────────┤
│      STM32Cube HAL (F4 V1.27)       │
├─────────────────────────────────────┤
│   STM32F401RET6 / Nucleo-F401RE     │
└─────────────────────────────────────┘
```

### FreeRTOS 任务列表

| 任务 | 优先级 | 周期 | 栈 | 职责 |
|------|--------|------|-----|------|
| guiTask | High (24) | 10ms | 1024w | LVGL 图形刷新 |
| touchTask | AboveNormal (25) | 20ms | 512w | 触控消抖采集 |
| rtcTask | AboveNormal (25) | 500ms | 512w | RTC 时间读取 |
| sysTask | AboveNormal (25) | ~50ms | 512w | 状态机/UI 更新/闹钟/息屏 |
| drvTask | Low (22) | 1000ms | 128w | 调试日志输出 |
| defaultTask | Normal (24) | — | 128w | 空闲占位 |

### 任务间通信

- **互斥锁** `lvgl_mutex` — 保护 LVGL 临界区，guiTask/sysTask 双向持锁
- **消息队列** `rtcQueue` — rtcTask → sysTask，传输 RTC 时间数据，容量 4
- **任务通知** `xTaskNotify` — touchTask → sysTask，传递手势事件（低延迟）
- **事件标志组** `sysEvent` — 触控唤醒 + 休眠超时协同

## 引脚分配

| 外设 | 引脚 | 功能 |
|------|------|------|
| SPI1_SCK | PA5 | LCD 时钟 |
| SPI1_MOSI | PA7 | LCD 数据 |
| LCD_CS | PB4 | LCD 片选 |
| LCD_DC | PB2 | LCD 数据/命令 |
| LCD_RST | PB1 | LCD 复位 |
| LCD_BL | PB8 | LCD 背光控制 |
| CST816D_SCL | PB6 | 触控 I2C 时钟 (软件模拟) |
| CST816D_SDA | PB7 | 触控 I2C 数据 (软件模拟) |
| CST816D_RST | PB9 | 触控复位 |
| CST816D_INT | PB10 | 触控中断 |
| I2S2_CK | PB12 | I2S 位时钟 |
| I2S2_WS | PB13 | I2S 字选 |
| I2S2_SD | PB15 | I2S 数据 |
| USART2_TX | PA2 | 调试日志输出 |
| USART2_RX | PA3 | 调试输入 |
| I2C1_SCL | PB6 | 传感器 I2C (预留) |
| I2C1_SDA | PB7 | 传感器 I2C (预留) |

## 构建说明

### 前置依赖

- GNU Arm Embedded Toolchain (`arm-none-eabi-gcc` 10.3+)
- GNU Make 或 CMake 3.22+
- STM32CubeMX (可选，用于修改引脚/时钟配置)

### 编译

```bash
# 使用 Make
make -j$(nproc)

# 或使用 CMake
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../CMakeLists.txt
make -j$(nproc)
```

### 烧录

```bash
# 使用 ST-Link (板载 ST-Link/V2-1)
st-flash write build/neckedf401re.bin 0x08000000
```

## 项目结构

```
.
├── Core/
│   ├── Inc/           # 头文件 (cst816d, lcd_gc9a01, rtc_time, i2s_alarm 等)
│   ├── Src/           # 源文件
│   └── Startup/       # 启动文件
├── Drivers/           # STM32 HAL 库
├── Middlewares/       # FreeRTOS 内核
├── lvgl/              # LVGL v8.3.11 图形库
├── MDK-ARM/           # Keil MDK 工程 (可选)
├── neckedf401re.ioc   # STM32CubeMX 项目文件
├── Makefile           # GNU Make 构建
├── CMakeLists.txt     # CMake 构建
└── README.md
```

## 许可

本项目为学术论文配套工程，源代码及相关文档仅作学习与交流用途。

---

**论文完成日期**：2026 年 6 月
