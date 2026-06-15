cmake_minimum_required(VERSION 3.16)
project(neckedf401re VERSION 1.0.0 LANGUAGES C ASM)

# ============================================================
# Toolchain
# ============================================================
set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP      arm-none-eabi-objdump)
set(CMAKE_SIZE         arm-none-eabi-size)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ============================================================
# MCU / Arch flags
# ============================================================
set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

# ============================================================
# C definitions
# ============================================================
add_compile_definitions(
    USE_HAL_DRIVER
    STM32F401xE
    LV_CONF_INCLUDE_SIMPLE
)

# ============================================================
# Include paths
# ============================================================
set(INC_DIRS
    Core/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include

    # FreeRTOS
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F

    # LVGL
    lvgl
    lvgl/src
    lvgl/src/core
    lvgl/src/widgets
    lvgl/src/draw
    lvgl/src/misc
    lvgl/src/font
    lvgl/src/hal
    lvgl/src/extra
    lvgl/src/extra/layouts/flex
    lvgl/src/extra/layouts/grid
    lvgl/src/extra/themes/default
    lvgl/src/extra/themes/basic
    lvgl/src/extra/themes/mono
    lvgl/src/extra/widgets/calendar
    lvgl/src/extra/widgets/chart
    lvgl/src/extra/widgets/colorwheel
    lvgl/src/extra/widgets/imgbtn
    lvgl/src/extra/widgets/keyboard
    lvgl/src/extra/widgets/led
    lvgl/src/extra/widgets/list
    lvgl/src/extra/widgets/menu
    lvgl/src/extra/widgets/meter
    lvgl/src/extra/widgets/msgbox
    lvgl/src/extra/widgets/span
    lvgl/src/extra/widgets/spinbox
    lvgl/src/extra/widgets/spinner
    lvgl/src/extra/widgets/tabview
    lvgl/src/extra/widgets/tileview
    lvgl/src/extra/widgets/win
    lvgl/src/extra/widgets/animimg
    lvgl/src/draw/sw
    lvgl/examples/porting
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE ${INC_DIRS})

# ============================================================
# Compiler / Linker flags
# ============================================================
set(CMAKE_C_FLAGS_INIT
    "${MCU_FLAGS} -Os -std=gnu11 -g3 -ffunction-sections -fdata-sections -Wall -Wextra -Wpedantic --specs=nano.specs"
)
set(CMAKE_ASM_FLAGS_INIT "${MCU_FLAGS} -g3")

set(CMAKE_EXE_LINKER_FLAGS_INIT
    "${MCU_FLAGS} --specs=nosys.specs --specs=nano.specs -T${CMAKE_SOURCE_DIR}/STM32F401RETX_FLASH.ld -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map,--cref -Wl,--gc-sections"
)

# ============================================================
# Source files
# ============================================================
# Core
file(GLOB_RECURSE CORE_SRC
    Core/Src/*.c
    Core/Src/*.s
)

# HAL
file(GLOB_RECURSE HAL_SRC
    Drivers/STM32F4xx_HAL_Driver/Src/*.c
)

# FreeRTOS kernel
set(FREERTOS_SRC
    Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    Middlewares/Third_Party/FreeRTOS/Source/queue.c
    Middlewares/Third_Party/FreeRTOS/Source/list.c
    Middlewares/Third_Party/FreeRTOS/Source/timers.c
    Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
    Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
)

# LVGL
file(GLOB_RECURSE LVGL_SRC
    lvgl/src/*.c
    lvgl/examples/porting/*.c
)

# Startup
set(STARTUP_SRC Core/Startup/startup_stm32f401retx.s)

# ============================================================
# Executable
# ============================================================
add_executable(${PROJECT_NAME}.elf
    ${CORE_SRC}
    ${HAL_SRC}
    ${FREERTOS_SRC}
    ${LVGL_SRC}
    ${STARTUP_SRC}
)

# ============================================================
# Post-build: hex + bin
# ============================================================
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
        $<TARGET_FILE:${PROJECT_NAME}.elf>
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S
        $<TARGET_FILE:${PROJECT_NAME}.elf>
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Generating hex, bin, and size report"
)
