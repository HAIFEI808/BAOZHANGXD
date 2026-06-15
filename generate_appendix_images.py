#!/usr/bin/env python3
"""生成论文图片：系统架构图 + 任务架构框图 + 甘特图"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch
import numpy as np
import os

OUT_DIR = r'D:\cxdownload\neckedf401re-main\neckedf401re-main'

# 支持中文
plt.rcParams['font.sans-serif'] = ['Microsoft YaHei', 'SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

# ─────────────────────────────────────────────────────────
# 附录1：FreeRTOS 智能手表任务架构框图
# ─────────────────────────────────────────────────────────

def generate_task_architecture():
    fig, ax = plt.subplots(1, 1, figsize=(16, 12))
    ax.set_xlim(0, 16)
    ax.set_ylim(0, 12)
    ax.axis('off')
    ax.set_facecolor('#FAFBFC')

    # 颜色定义
    C_TASK_BG = '#E8F0FE'
    C_TASK_EDGE = '#2F5496'
    C_ARROW = '#555555'
    C_MUTEX = '#E74C3C'
    C_QUEUE = '#27AE60'
    C_NOTIFY = '#8E44AD'
    C_LABEL = '#333333'

    def draw_task_box(x, y, name, priority, period, stack, duty, color_bg=C_TASK_BG, w=4.8, h=1.8):
        """绘制任务方框"""
        rect = FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.15",
                              facecolor=color_bg, edgecolor=C_TASK_EDGE, linewidth=2)
        ax.add_patch(rect)
        # 任务名称
        ax.text(x + w/2, y + h - 0.25, name, ha='center', va='top', fontsize=11,
                fontweight='bold', color=C_TASK_EDGE, fontfamily='monospace')
        # 属性行
        attrs = [
            f"Priority: {priority}",
            f"Period: {period}",
            f"Stack: {stack}",
        ]
        for j, attr in enumerate(attrs):
            ax.text(x + w/2, y + h - 0.75 - j*0.4, attr, ha='center', va='top',
                    fontsize=7.5, color='#444444')
        # 职责（不含 monospace，支持中文）
        ax.text(x + w/2, y + 0.15, duty, ha='center', va='bottom', fontsize=7,
                color='#666666', style='italic')

    def draw_arrow(x1, y1, x2, y2, color=C_ARROW, style='->', lw=1.5, ls='-'):
        """绘制箭头"""
        ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                    arrowprops=dict(arrowstyle=style, color=color,
                                    lw=lw, ls=ls, connectionstyle='arc3,rad=0'))

    def draw_label(x, y, text, color=C_LABEL, fontsize=8):
        ax.text(x, y, text, ha='center', va='center', fontsize=fontsize,
                color=color, fontweight='bold',
                bbox=dict(boxstyle='round,pad=0.2', facecolor='white',
                          edgecolor=color, linewidth=1, alpha=0.9))

    # ── 标题 ──
    ax.text(8, 11.6, 'FreeRTOS 智能手表任务架构与通信框图',
            ha='center', va='center', fontsize=16, fontweight='bold', color='#1a1a2e')
    ax.text(8, 11.1, 'STM32F401RET6 + FreeRTOS V10.3.1 (CMSIS-RTOS V2) + LVGL v8.3.11',
            ha='center', va='center', fontsize=9, color='#888888')

    # ── LVGL 引擎框 ──
    rect = FancyBboxPatch((6.2, 9.8), 3.6, 0.9, boxstyle="round,pad=0.1",
                          facecolor='#FFF3E0', edgecolor='#E67E22', linewidth=1.5, linestyle='--')
    ax.add_patch(rect)
    ax.text(8, 10.25, 'LVGL GUI Engine', ha='center', va='center', fontsize=10,
            fontweight='bold', color='#E67E22')
    ax.text(8, 9.95, 'lv_task_handler() | lv_scr_load() | lv_label_set_text()',
            ha='center', va='center', fontsize=7, color='#E67E22')

    # ── 六个任务 ──
    tasks = [
        # (x, y, name, priority, period, stack, duty, color)
        (0.3, 7.2, 'guiTask', 'High (24)', '10ms', '4096B', 'lv_task_handler() + mutex',
         '#D5F5E3'),
        (0.3, 4.6, 'touchTask', 'AboveNormal (25)', '20ms', '2048B', 'CST816D 消抖 + xTaskNotify',
         '#D6EAF8'),
        (5.6, 4.6, 'rtcTask', 'AboveNormal (25)', '500ms', '2048B', 'RTC_Get → osMessageQueuePut',
         '#D6EAF8'),
        (3.0, 1.5, 'sysTask', 'AboveNormal (25)', '~50ms (notify驱动)', '2048B',
         '状态机: HOME/SETTINGS/SLEEP + 闹钟 + 息屏',
         '#FDEBD0'),
        (10.5, 4.6, 'drvTask', 'Low (22)', '1000ms', '512B', 'UART日志 "[LOG] tick=..."',
         '#FADBD8'),
        (10.5, 7.2, 'defaultTask', 'Normal (24)', '-', '512B', 'osDelay(1) idle',
         '#E5E7E9'),
    ]

    for t in tasks:
        draw_task_box(*t)

    # ── 通信连线 ──

    # rtcTask → sysTask (消息队列)
    draw_arrow(5.6, 5.5, 5.2, 3.3, color=C_QUEUE, lw=2.5)
    draw_label(5.0, 4.5, '消息队列\nosMessageQueue\n(RTC_DateTime_t)', C_QUEUE, 7)

    # touchTask → sysTask (任务通知)
    draw_arrow(3.0, 4.6, 5.0, 3.3, color=C_NOTIFY, lw=2.5, ls='--')
    draw_label(3.0, 3.8, '任务通知\nxTaskNotify\n(gesture)', C_NOTIFY, 7)

    # guiTask ← LVGL (互斥锁保护)
    draw_arrow(3.0, 8.0, 6.5, 9.8, color=C_MUTEX, lw=2.5, ls='-.')
    draw_label(4.5, 9.2, '互斥锁\nlvgl_mutex', C_MUTEX, 7)

    # sysTask ← LVGL (互斥锁保护)
    draw_arrow(5.0, 3.3, 6.5, 9.8, color=C_MUTEX, lw=2.5, ls='-.')
    draw_label(6.8, 6.5, '互斥锁\nlvgl_mutex', C_MUTEX, 7)

    # sysTask → drvTask (UART日志，低优先级)
    draw_arrow(7.0, 3.3, 10.5, 5.5, color=C_ARROW, lw=1.2)
    draw_label(9.0, 4.2, 'USART2\nHAL_UART_Transmit', C_ARROW, 6.5)

    # touchTask → 硬件层
    ax.annotate('', xy=(2, 6.6), xytext=(2, 5.6),
                arrowprops=dict(arrowstyle='->', color='#3498DB', lw=1.8))
    draw_label(2.7, 6.1, '软件 I2C\n(bit-bang)', '#3498DB', 7)

    # rtcTask → 硬件层
    ax.annotate('', xy=(7, 6.6), xytext=(7, 5.6),
                arrowprops=dict(arrowstyle='->', color='#3498DB', lw=1.8))
    draw_label(7.7, 6.1, 'HAL RTC\n(LSI 32kHz)', '#3498DB', 7)

    # ── 硬件层 ──
    rect = FancyBboxPatch((0.3, 0.2), 15.4, 0.7, boxstyle="round,pad=0.1",
                          facecolor='#2C3E50', edgecolor='#1a252f', linewidth=2)
    ax.add_patch(rect)
    ax.text(8, 0.55, 'SPI1 (GC9A01 LCD)  |  I2C (CST816D Touch)  |  I2S2 (Alarm Audio)  |  RTC (LSI)  |  USART2 (Debug)  |  USART6 (ESP8266 WiFi)  |  GPIO (Backlight PB8)',
            ha='center', va='center', fontsize=7.5, color='white', fontfamily='monospace')

    # ── 图例 ──
    legend_y = 11.6
    legends = [
        (C_QUEUE, '── 消息队列'),
        (C_NOTIFY, '- - 任务通知'),
        (C_MUTEX, '-·- 互斥锁'),
        ('#3498DB', '── 硬件接口'),
    ]
    for k, (color, label) in enumerate(legends):
        ax.text(1.5 + k*2.5, 0.9, label, fontsize=7.5, color=color, fontweight='bold')

    plt.tight_layout()
    path = os.path.join(OUT_DIR, 'appendix1_task_architecture.png')
    fig.savefig(path, dpi=200, bbox_inches='tight', facecolor='white', edgecolor='none')
    plt.close(fig)
    print(f'附录1框图已生成: {path}')
    return path


# ─────────────────────────────────────────────────────────
# 附录2：甘特图
# ─────────────────────────────────────────────────────────

def generate_gantt_chart():
    fig, ax = plt.subplots(1, 1, figsize=(14, 7))
    ax.set_facecolor('#FAFBFC')

    # 任务数据: (名称, 开始课时, 持续课时, 颜色, 阶段)
    tasks = [
        # 基础环境
        ('W01 需求分析',        1, 1, '#2ECC71', '基础环境'),
        ('W02 CubeMX+FreeRTOS', 2, 1, '#2ECC71', '基础环境'),
        # 底层驱动
        ('W03 LCD驱动',         3, 1, '#3498DB', '底层驱动'),
        ('W04 LVGL移植',        4, 1, '#3498DB', '底层驱动'),
        ('W05 触控驱动',        5, 1, '#3498DB', '底层驱动'),
        ('W06 触控消抖',        6, 1, '#3498DB', '底层驱动'),
        ('W07 RTC驱动',         7, 1, '#3498DB', '底层驱动'),
        # 架构层
        ('W08 任务架构',        8, 1, '#9B59B6', '系统架构'),
        ('W09 通信机制',        9, 1, '#9B59B6', '系统架构'),
        # 应用层
        ('W10 主表盘UI',        10, 1, '#E67E22', '应用层'),
        ('W11 设置页/校准',     11, 1, '#E67E22', '应用层'),
        ('W12 息屏/联调/文档',  12, 1, '#E67E22', '应用层'),
        # 扩展
        ('B01 I2S闹钟',         12.5, 0.5, '#E74C3C', '扩展'),
    ]

    y_positions = list(range(len(tasks)))
    colors = [t[3] for t in tasks]
    phase_colors = {
        '基础环境': '#2ECC71',
        '底层驱动': '#3498DB',
        '系统架构': '#9B59B6',
        '应用层': '#E67E22',
        '扩展': '#E74C3C',
    }

    for i, (name, start, duration, color, phase) in enumerate(tasks):
        ax.barh(i, duration, left=start - 0.5, height=0.65, color=color,
                edgecolor='white', linewidth=0.8, alpha=0.9)
        # 标签
        ax.text(start + duration/2 - 0.5, i, name, ha='center', va='center',
                fontsize=8, fontweight='bold', color='white')

    # 阶段分隔线
    ax.axhline(y=1.5, color='#CCCCCC', linewidth=1, linestyle='--', alpha=0.7)
    ax.axhline(y=6.5, color='#CCCCCC', linewidth=1, linestyle='--', alpha=0.7)
    ax.axhline(y=8.5, color='#CCCCCC', linewidth=1, linestyle='--', alpha=0.7)
    ax.axhline(y=11.5, color='#CCCCCC', linewidth=1, linestyle='--', alpha=0.7)

    # 阶段标签
    phase_ranges = [
        (1, '基础环境搭建', 0, 1),
        (2, '底层驱动开发', 2, 6),
        (3, '系统架构设计', 7, 8),
        (4, '应用层开发', 9, 11),
        (5, '扩展功能', 12, 12),
    ]
    for _, label, y_start, y_end in phase_ranges:
        ax.text(-0.6, (y_start + y_end) / 2, label, ha='right', va='center',
                fontsize=8.5, fontweight='bold', color='#555555', rotation=0)

    # 坐标轴
    ax.set_yticks([])
    ax.set_xticks(range(1, 14))
    ax.set_xticklabels([str(i) for i in range(1, 14)], fontsize=9)
    ax.set_xlabel('课时编号', fontsize=11, fontweight='bold')
    ax.set_xlim(0, 14)
    ax.invert_yaxis()

    # 标题
    ax.set_title('项目开发规划甘特图（12课时 + 1 Bonus）', fontsize=14,
                 fontweight='bold', color='#1a1a2e', pad=15)

    # 图例
    legend_patches = [mpatches.Patch(color=c, label=l) for l, c in phase_colors.items()]
    ax.legend(handles=legend_patches, loc='lower right', fontsize=8.5,
              ncol=5, framealpha=0.9, edgecolor='#CCCCCC')

    # 备注
    ax.text(7, len(tasks) + 0.3,
            '注：W03-W07（驱动层）可部分并行开发；W08-W09（架构层）依赖驱动；W10-W12（应用层）依赖驱动+架构完成',
            ha='center', va='top', fontsize=8, color='#999999', style='italic')

    plt.tight_layout()
    path = os.path.join(OUT_DIR, 'appendix2_gantt_chart.png')
    fig.savefig(path, dpi=200, bbox_inches='tight', facecolor='white', edgecolor='none')
    plt.close(fig)
    print(f'附录2甘特图已生成: {path}')
    return path


# ─────────────────────────────────────────────────────────
# 图2-1：系统总体架构图
# ─────────────────────────────────────────────────────────

def generate_system_architecture():
    fig, ax = plt.subplots(1, 1, figsize=(12, 9))
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 9)
    ax.axis('off')
    ax.set_facecolor('#FAFBFC')

    # 颜色
    C_BOX = '#2F5496'
    C_SUB = '#5B9BD5'
    C_BG1 = '#D6E4F0'
    C_BG2 = '#E2EFDA'
    C_BG3 = '#FCE4D6'
    C_BG4 = '#E4DFEC'

    # ── 标题 ──
    ax.text(6, 8.7, '智能手表嵌入式系统总体架构',
            ha='center', va='center', fontsize=16, fontweight='bold', color='#1a1a2e')

    def draw_layer(y, h, label, color_bg, color_text, subtitle=''):
        """绘制一个层次"""
        rect = FancyBboxPatch((0.5, y), 11, h, boxstyle="round,pad=0.15",
                              facecolor=color_bg, edgecolor=color_text, linewidth=2)
        ax.add_patch(rect)
        ax.text(6, y + h/2 + 0.2, label, ha='center', va='center',
                fontsize=13, fontweight='bold', color=color_text)
        if subtitle:
            ax.text(6, y + h/2 - 0.4, subtitle, ha='center', va='center',
                    fontsize=8, color='#666666', style='italic')

    def draw_component(x, y, w, h, text, color_bg, color_text, fontsize=8):
        """绘制组件框"""
        rect = FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.08",
                              facecolor=color_bg, edgecolor=color_text, linewidth=1.5)
        ax.add_patch(rect)
        ax.text(x + w/2, y + h/2, text, ha='center', va='center',
                fontsize=fontsize, fontweight='bold', color=color_text)

    # ── 应用层 ──
    draw_layer(6.0, 2.3, '应用层 (Application Layer)', C_BG1, '#1a3a5c')

    # 五个任务
    tasks_2_1 = [
        (0.8, 6.2, 1.8, 0.8, 'guiTask\nGUI刷新', '#D5F5E3', '#1e8449'),
        (2.8, 6.2, 1.8, 0.8, 'touchTask\n触控采集', '#D6EAF8', '#2471a3'),
        (4.8, 6.2, 1.8, 0.8, 'rtcTask\nRTC读取', '#D6EAF8', '#2471a3'),
        (7.0, 6.2, 2.0, 0.8, 'sysTask\n系统状态管理', '#FDEBD0', '#b9770e'),
        (9.2, 6.2, 1.8, 0.8, 'drvTask\n调试日志', '#FADBD8', '#922b21'),
    ]
    for t in tasks_2_1:
        draw_component(*t)

    # LVGL 引擎
    draw_component(2.5, 7.2, 7.0, 0.7, 'LVGL v8.3.11 图形引擎 (lv_task_handler + lv_* API)',
                   '#FFF3E0', '#E67E22', 9)

    # ── 操作系统层 ──
    draw_layer(3.7, 2.0, 'FreeRTOS 操作系统层 (OS Layer)', C_BG2, '#2d6a2d',
              subtitle='Kernel V10.3.1 + CMSIS-RTOS V2 API')

    # OS 组件
    os_comps = [
        (0.8, 3.9, 2.2, 0.7, '抢占式任务调度\n(Preemptive)', '#D5F5E3', '#1e8449', 8),
        (3.2, 3.9, 2.2, 0.7, '任务间通信\nmutex/queue/notify', '#D6EAF8', '#2471a3', 8),
        (5.6, 3.9, 2.2, 0.7, 'CMSIS-RTOS V2\nAPI 封装层', '#FDEBD0', '#b9770e', 8),
        (8.0, 3.9, 2.2, 0.7, '内存管理\nheap_4.c (15KB)', '#FADBD8', '#922b21', 8),
    ]
    for oc in os_comps:
        draw_component(*oc)

    # ── 硬件抽象层 ──
    draw_layer(1.4, 2.0, '硬件抽象层 (HAL Layer)', C_BG3, '#b85c0e',
              subtitle='STM32Cube HAL + 底层驱动')

    hal_comps = [
        (0.8, 1.6, 1.4, 0.9, 'SPI1\n(GC9A01 LCD)', '#FFF3E0', '#E67E22', 6.5),
        (2.4, 1.6, 1.4, 0.9, 'I2C\n(CST816D)', '#FFF3E0', '#E67E22', 6.5),
        (4.0, 1.6, 1.4, 0.9, 'I2S2\n(Alarm)', '#FFF3E0', '#E67E22', 6.5),
        (5.6, 1.6, 1.4, 0.9, 'RTC\n(LSI 32kHz)', '#FFF3E0', '#E67E22', 6.5),
        (7.2, 1.6, 1.4, 0.9, 'USART2/6\n(Debug/WiFi)', '#FFF3E0', '#E67E22', 6.5),
        (8.8, 1.6, 1.4, 0.9, 'GPIO\n(Backlight)', '#FFF3E0', '#E67E22', 6.5),
    ]
    for hc in hal_comps:
        draw_component(*hc)

    # ── 硬件层 ──
    rect = FancyBboxPatch((0.5, 0.15), 11, 0.95, boxstyle="round,pad=0.1",
                          facecolor='#2C3E50', edgecolor='#1a252f', linewidth=2.5)
    ax.add_patch(rect)
    ax.text(6, 0.62, 'STM32F401RET6 — ARM Cortex-M4F (84MHz) | 512KB Flash | 96KB SRAM',
            ha='center', va='center', fontsize=11, fontweight='bold', color='white')
    ax.text(6, 0.32, 'Nucleo-F401RE Development Board',
            ha='center', va='center', fontsize=8, color='#AAAAAA')

    # ── 层间箭头 ──
    arrow_style = dict(arrowstyle='->', color='#888888', lw=2, connectionstyle='arc3,rad=0')
    ax.annotate('', xy=(0.25, 3.7), xytext=(0.25, 6.0), arrowprops=arrow_style)
    ax.annotate('', xy=(0.25, 1.4), xytext=(0.25, 3.7), arrowprops=arrow_style)

    plt.tight_layout()
    path = os.path.join(OUT_DIR, 'fig2_1_system_architecture.png')
    fig.savefig(path, dpi=200, bbox_inches='tight', facecolor='white', edgecolor='none')
    plt.close(fig)
    print(f'图2-1已生成: {path}')
    return path


# ─────────────────────────────────────────────────────────
# 主入口
# ─────────────────────────────────────────────────────────

if __name__ == '__main__':
    generate_system_architecture()
    generate_task_architecture()
    generate_gantt_chart()
    print('所有论文图片生成完成。')
