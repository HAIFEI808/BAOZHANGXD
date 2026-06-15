TARGET = neckedf401re
BUILD_DIR = build

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

C_DEFS = -DUSE_HAL_DRIVER -DSTM32F401xE -DLV_CONF_INCLUDE_SIMPLE

AS_DEFS = 

C_INCLUDES = \
-ICore/Inc \
-IDrivers/STM32F4xx_HAL_Driver/Inc \
-IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32F4xx/Include \
-IDrivers/CMSIS/Include \
-Ilvgl \
-Ilvgl/src \
-Ilvgl/src/core \
-Ilvgl/src/widgets \
-Ilvgl/src/draw \
-Ilvgl/src/misc \
-Ilvgl/src/font \
-Ilvgl/src/hal \
-Ilvgl/src/extra \
-Ilvgl/src/extra/layouts/flex \
-Ilvgl/src/extra/layouts/grid \
-Ilvgl/src/extra/themes/default \
-Ilvgl/src/extra/themes/basic \
-Ilvgl/src/extra/themes/mono \
-Ilvgl/src/extra/widgets/calendar \
-Ilvgl/src/extra/widgets/chart \
-Ilvgl/src/extra/widgets/colorwheel \
-Ilvgl/src/extra/widgets/imgbtn \
-Ilvgl/src/extra/widgets/keyboard \
-Ilvgl/src/extra/widgets/led \
-Ilvgl/src/extra/widgets/list \
-Ilvgl/src/extra/widgets/menu \
-Ilvgl/src/extra/widgets/meter \
-Ilvgl/src/extra/widgets/msgbox \
-Ilvgl/src/extra/widgets/span \
-Ilvgl/src/extra/widgets/spinbox \
-Ilvgl/src/extra/widgets/spinner \
-Ilvgl/src/extra/widgets/tabview \
-Ilvgl/src/extra/widgets/tileview \
-Ilvgl/src/extra/widgets/win \
-Ilvgl/src/extra/widgets/animimg \
-Ilvgl/src/draw/sw \
-Ilvgl/examples/porting

OPT = -Os

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -std=gnu11 -g3 \
-ffunction-sections -fdata-sections -Wall -Wextra -Wpedantic \
--specs=nano.specs

ASFLAGS = $(MCU) $(AS_DEFS) $(C_INCLUDES) -g3

LDSCRIPT = STM32F401RETX_FLASH.ld
LDFLAGS = $(MCU) --specs=nosys.specs --specs=nano.specs -T$(LDSCRIPT) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

CORE_SRC = \
Core/Src/main.c \
Core/Src/stm32f4xx_it.c \
Core/Src/stm32f4xx_hal_msp.c \
Core/Src/system_stm32f4xx.c \
Core/Src/syscalls.c \
Core/Src/sysmem.c \
Core/Src/usart.c \
Core/Src/rtc_time.c \
Core/Src/cst816d.c \
Core/Src/lcd_gc9a01.c

HAL_SRC = \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rtc.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rtc_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c

LVGL_CORE_SRC = \
lvgl/src/core/lv_disp.c \
lvgl/src/core/lv_event.c \
lvgl/src/core/lv_group.c \
lvgl/src/core/lv_indev.c \
lvgl/src/core/lv_indev_scroll.c \
lvgl/src/core/lv_obj.c \
lvgl/src/core/lv_obj_class.c \
lvgl/src/core/lv_obj_draw.c \
lvgl/src/core/lv_obj_pos.c \
lvgl/src/core/lv_obj_scroll.c \
lvgl/src/core/lv_obj_style.c \
lvgl/src/core/lv_obj_style_gen.c \
lvgl/src/core/lv_obj_tree.c \
lvgl/src/core/lv_refr.c \
lvgl/src/core/lv_theme.c

LVGL_DRAW_SRC = \
lvgl/src/draw/lv_draw.c \
lvgl/src/draw/lv_draw_arc.c \
lvgl/src/draw/lv_draw_img.c \
lvgl/src/draw/lv_draw_label.c \
lvgl/src/draw/lv_draw_layer.c \
lvgl/src/draw/lv_draw_line.c \
lvgl/src/draw/lv_draw_mask.c \
lvgl/src/draw/lv_draw_rect.c \
lvgl/src/draw/lv_draw_transform.c \
lvgl/src/draw/lv_draw_triangle.c \
lvgl/src/draw/lv_img_buf.c \
lvgl/src/draw/lv_img_cache.c \
lvgl/src/draw/lv_img_decoder.c \
lvgl/src/draw/sw/lv_draw_sw.c \
lvgl/src/draw/sw/lv_draw_sw_arc.c \
lvgl/src/draw/sw/lv_draw_sw_blend.c \
lvgl/src/draw/sw/lv_draw_sw_dither.c \
lvgl/src/draw/sw/lv_draw_sw_gradient.c \
lvgl/src/draw/sw/lv_draw_sw_img.c \
lvgl/src/draw/sw/lv_draw_sw_layer.c \
lvgl/src/draw/sw/lv_draw_sw_letter.c \
lvgl/src/draw/sw/lv_draw_sw_line.c \
lvgl/src/draw/sw/lv_draw_sw_polygon.c \
lvgl/src/draw/sw/lv_draw_sw_rect.c \
lvgl/src/draw/sw/lv_draw_sw_transform.c

LVGL_MISC_SRC = \
lvgl/src/misc/lv_anim.c \
lvgl/src/misc/lv_anim_timeline.c \
lvgl/src/misc/lv_area.c \
lvgl/src/misc/lv_async.c \
lvgl/src/misc/lv_bidi.c \
lvgl/src/misc/lv_color.c \
lvgl/src/misc/lv_fs.c \
lvgl/src/misc/lv_gc.c \
lvgl/src/misc/lv_ll.c \
lvgl/src/misc/lv_log.c \
lvgl/src/misc/lv_lru.c \
lvgl/src/misc/lv_math.c \
lvgl/src/misc/lv_mem.c \
lvgl/src/misc/lv_printf.c \
lvgl/src/misc/lv_style.c \
lvgl/src/misc/lv_style_gen.c \
lvgl/src/misc/lv_timer.c \
lvgl/src/misc/lv_tlsf.c \
lvgl/src/misc/lv_txt.c \
lvgl/src/misc/lv_txt_ap.c \
lvgl/src/misc/lv_utils.c

LVGL_HAL_SRC = \
lvgl/src/hal/lv_hal_disp.c \
lvgl/src/hal/lv_hal_indev.c \
lvgl/src/hal/lv_hal_tick.c

LVGL_WIDGETS_SRC = \
lvgl/src/widgets/lv_arc.c \
lvgl/src/widgets/lv_bar.c \
lvgl/src/widgets/lv_btn.c \
lvgl/src/widgets/lv_btnmatrix.c \
lvgl/src/widgets/lv_canvas.c \
lvgl/src/widgets/lv_checkbox.c \
lvgl/src/widgets/lv_dropdown.c \
lvgl/src/widgets/lv_img.c \
lvgl/src/widgets/lv_label.c \
lvgl/src/widgets/lv_line.c \
lvgl/src/widgets/lv_roller.c \
lvgl/src/widgets/lv_slider.c \
lvgl/src/widgets/lv_switch.c \
lvgl/src/widgets/lv_table.c \
lvgl/src/widgets/lv_textarea.c

LVGL_FONT_SRC = \
lvgl/src/font/lv_font.c \
lvgl/src/font/lv_font_dejavu_16_persian_hebrew.c \
lvgl/src/font/lv_font_fmt_txt.c \
lvgl/src/font/lv_font_loader.c \
lvgl/src/font/lv_font_montserrat_10.c \
lvgl/src/font/lv_font_montserrat_12.c \
lvgl/src/font/lv_font_montserrat_14.c \
lvgl/src/font/lv_font_montserrat_16.c \
lvgl/src/font/lv_font_montserrat_18.c \
lvgl/src/font/lv_font_montserrat_20.c \
lvgl/src/font/lv_font_montserrat_22.c \
lvgl/src/font/lv_font_montserrat_24.c \
lvgl/src/font/lv_font_montserrat_26.c \
lvgl/src/font/lv_font_montserrat_28.c \
lvgl/src/font/lv_font_montserrat_28_compressed.c \
lvgl/src/font/lv_font_montserrat_30.c \
lvgl/src/font/lv_font_montserrat_32.c \
lvgl/src/font/lv_font_montserrat_34.c \
lvgl/src/font/lv_font_montserrat_36.c \
lvgl/src/font/lv_font_montserrat_38.c \
lvgl/src/font/lv_font_montserrat_40.c \
lvgl/src/font/lv_font_montserrat_42.c \
lvgl/src/font/lv_font_montserrat_44.c \
lvgl/src/font/lv_font_montserrat_46.c \
lvgl/src/font/lv_font_montserrat_48.c \
lvgl/src/font/lv_font_montserrat_12_subpx.c \
lvgl/src/font/lv_font_simsun_16_cjk.c \
lvgl/src/font/lv_font_unscii_8.c \
lvgl/src/font/lv_font_unscii_16.c

LVGL_EXTRA_SRC = \
lvgl/src/extra/lv_extra.c \
lvgl/src/extra/layouts/flex/lv_flex.c \
lvgl/src/extra/layouts/grid/lv_grid.c \
lvgl/src/extra/themes/default/lv_theme_default.c \
lvgl/src/extra/themes/basic/lv_theme_basic.c \
lvgl/src/extra/themes/mono/lv_theme_mono.c \
lvgl/src/extra/widgets/calendar/lv_calendar.c \
lvgl/src/extra/widgets/calendar/lv_calendar_header_arrow.c \
lvgl/src/extra/widgets/calendar/lv_calendar_header_dropdown.c \
lvgl/src/extra/widgets/chart/lv_chart.c \
lvgl/src/extra/widgets/colorwheel/lv_colorwheel.c \
lvgl/src/extra/widgets/imgbtn/lv_imgbtn.c \
lvgl/src/extra/widgets/keyboard/lv_keyboard.c \
lvgl/src/extra/widgets/led/lv_led.c \
lvgl/src/extra/widgets/list/lv_list.c \
lvgl/src/extra/widgets/menu/lv_menu.c \
lvgl/src/extra/widgets/meter/lv_meter.c \
lvgl/src/extra/widgets/msgbox/lv_msgbox.c \
lvgl/src/extra/widgets/span/lv_span.c \
lvgl/src/extra/widgets/spinbox/lv_spinbox.c \
lvgl/src/extra/widgets/spinner/lv_spinner.c \
lvgl/src/extra/widgets/tabview/lv_tabview.c \
lvgl/src/extra/widgets/tileview/lv_tileview.c \
lvgl/src/extra/widgets/win/lv_win.c \
lvgl/src/extra/widgets/animimg/lv_animimg.c

LVGL_PORTING_SRC = \
lvgl/examples/porting/lv_port_disp.c

LVGL_SRC = $(LVGL_CORE_SRC) $(LVGL_DRAW_SRC) $(LVGL_MISC_SRC) $(LVGL_HAL_SRC) $(LVGL_WIDGETS_SRC) $(LVGL_FONT_SRC) $(LVGL_EXTRA_SRC) $(LVGL_PORTING_SRC)

STARTUP_SRC = Core/Startup/startup_stm32f401retx.s

C_SOURCES = $(CORE_SRC) $(HAL_SRC) $(LVGL_SRC)

OBJS = $(C_SOURCES:%.c=$(BUILD_DIR)/%.o) $(STARTUP_SRC:%.s=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	$(SIZE) $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/Core/Src:
	mkdir -p $@

$(BUILD_DIR)/Core/Startup:
	mkdir -p $@

$(BUILD_DIR)/Drivers/STM32F4xx_HAL_Driver/Src:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/core:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/draw:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/draw/sw:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/misc:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/hal:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/widgets:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/font:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/layouts/flex:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/layouts/grid:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/themes/default:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/themes/basic:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/themes/mono:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/calendar:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/chart:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/colorwheel:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/imgbtn:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/keyboard:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/led:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/list:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/menu:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/meter:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/msgbox:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/span:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/spinbox:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/spinner:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/tabview:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/tileview:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/win:
	mkdir -p $@

$(BUILD_DIR)/lvgl/src/extra/widgets/animimg:
	mkdir -p $@

$(BUILD_DIR)/lvgl/examples/porting:
	mkdir -p $@

clean:
	-rm -rf $(BUILD_DIR)

.PHONY: all clean
