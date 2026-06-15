/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd_gc9a01.h"
#include "cst816d.h"
#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "rtc_time.h"
#include "i2s_alarm.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for taskLED0 */
osThreadId_t taskLED0Handle;
const osThreadAttr_t taskLED0_attributes = {
  .name = "taskLED0",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for homeTask */
osThreadId_t homeTaskHandle;
const osThreadAttr_t homeTask_attributes = {
  .name = "homeTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for startTaskLvgl */
osThreadId_t startTaskLvglHandle;
const osThreadAttr_t startTaskLvgl_attributes = {
  .name = "startTaskLvgl",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for rtcTimer */
osTimerId_t rtcTimerHandle;
const osTimerAttr_t rtcTimer_attributes = {
  .name = "rtcTimer"
};
/* Definitions for lvgl_mutex */
osMutexId_t lvgl_mutexHandle;
const osMutexAttr_t lvgl_mutex_attributes = {
  .name = "lvgl_mutex"
};
/* USER CODE BEGIN PV */
/* new task handles */
osThreadId_t touchTaskHandle;
osThreadId_t rtcTaskHandle;
/* rtcQueue */
osMessageQueueId_t rtcQueueHandle;
/* sysEvent */
osEventFlagsId_t sysEventHandle;
#define SYS_EVENT_TOUCH  0x01
#define SYS_EVENT_SLEEP  0x02
/* sleep timer handle */
osTimerId_t sleepTimerHandle;
/* helper */
static volatile uint8_t settings_confirm;
/* alarm */
AlarmTime_t alarm_time = {7, 0, 0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartHomeTask(void *argument);
void StartTasklvgl(void *argument);
void RtcTimer(void *argument);

/* USER CODE BEGIN PFP */
/** @brief 触控采集任务 — 消抖读取手势，任务通知 sysTask */
void StartTouchTask(void *argument);
/** @brief RTC 采集任务 — 每 500ms 读取完整日期时间并发送到消息队列 */
void StartRtcTask(void *argument);
/** @brief 休眠定时器回调 — 超时后置位 SYS_EVENT_SLEEP 事件标志 */
void SleepTimerCallback(void *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static lv_obj_t *scr_home;
static lv_obj_t *scr_settings;
static lv_obj_t *clock_label;
static lv_obj_t *date_label;
static lv_obj_t *roller_hour;
static lv_obj_t *roller_min;
static lv_obj_t *roller_alarm_hour;
static lv_obj_t *roller_alarm_min;
static lv_obj_t *btn_alarm_toggle;
static lv_obj_t *alarm_status_label;

static void alarm_toggle_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        alarm_time.enabled = !alarm_time.enabled;
        lv_label_set_text(alarm_status_label, alarm_time.enabled ? "ON" : "OFF");
    }
}

static void settings_confirm_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        settings_confirm = 1;
    }
}

static void sync_settings_rollers(void)
{
    RTC_DateTime_t now;
    RTC_Get_DateTime(&now);
    lv_roller_set_selected(roller_hour, now.hour, LV_ANIM_OFF);
    lv_roller_set_selected(roller_min, now.min, LV_ANIM_OFF);
    lv_roller_set_selected(roller_alarm_hour, alarm_time.hour, LV_ANIM_OFF);
    lv_roller_set_selected(roller_alarm_min, alarm_time.min, LV_ANIM_OFF);
    lv_label_set_text(alarm_status_label, alarm_time.enabled ? "ON" : "OFF");
}

static void page_gesture_handler(lv_event_t *e)
{
    lv_obj_t *screen = lv_event_get_current_target(e);
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir == LV_DIR_LEFT || dir == LV_DIR_RIGHT) {
        if(screen == scr_home) {
            lv_scr_load(scr_settings);
            sync_settings_rollers();
        } else if(screen == scr_settings) {
            lv_scr_load(scr_home);
        }
    }
}

/**
 * @brief 创建设置页面 UI
 * @note  上半部分：时间校准（时/分 roller + 确认按钮）
 *        下半部分：闹钟设置（时/分 roller + ON/OFF 按钮）
 *        确认后通过 settings_confirm 标志通知 sysTask 写入 RTC
 */
static void create_settings_page(void)
{
    scr_settings = lv_obj_create(NULL);

    /* ---- Section: Time Calibration ---- */
    lv_obj_t *title = lv_label_create(scr_settings);
    lv_label_set_text(title, "Time Set");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

    roller_hour = lv_roller_create(scr_settings);
    lv_roller_set_options(roller_hour,
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n"
        "12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23",
        LV_ROLLER_MODE_INFINITE);
    lv_obj_set_style_text_font(roller_hour, &lv_font_montserrat_14, 0);
    lv_obj_align(roller_hour, LV_ALIGN_LEFT_MID, 30, -35);

    lv_obj_t *sep1 = lv_label_create(scr_settings);
    lv_label_set_text(sep1, ":");
    lv_obj_set_style_text_font(sep1, &lv_font_montserrat_20, 0);
    lv_obj_align(sep1, LV_ALIGN_CENTER, 0, -35);

    roller_min = lv_roller_create(scr_settings);
    lv_roller_set_options(roller_min,
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n"
        "10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n"
        "20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n"
        "30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n"
        "40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n"
        "50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
        LV_ROLLER_MODE_INFINITE);
    lv_obj_set_style_text_font(roller_min, &lv_font_montserrat_14, 0);
    lv_obj_align(roller_min, LV_ALIGN_RIGHT_MID, -30, -35);

    lv_obj_t *btn = lv_btn_create(scr_settings);
    lv_obj_set_size(btn, 90, 30);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn, settings_confirm_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Set RTC");
    lv_obj_center(btn_label);

    /* ---- Section: Alarm ---- */
    lv_obj_t *alarm_title = lv_label_create(scr_settings);
    lv_label_set_text(alarm_title, "Alarm");
    lv_obj_set_style_text_font(alarm_title, &lv_font_montserrat_18, 0);
    lv_obj_align(alarm_title, LV_ALIGN_TOP_MID, 0, 50);

    roller_alarm_hour = lv_roller_create(scr_settings);
    lv_roller_set_options(roller_alarm_hour,
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n"
        "12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23",
        LV_ROLLER_MODE_INFINITE);
    lv_obj_set_style_text_font(roller_alarm_hour, &lv_font_montserrat_14, 0);
    lv_obj_align(roller_alarm_hour, LV_ALIGN_LEFT_MID, 30, 15);

    lv_obj_t *sep2 = lv_label_create(scr_settings);
    lv_label_set_text(sep2, ":");
    lv_obj_set_style_text_font(sep2, &lv_font_montserrat_20, 0);
    lv_obj_align(sep2, LV_ALIGN_CENTER, 0, 15);

    roller_alarm_min = lv_roller_create(scr_settings);
    lv_roller_set_options(roller_alarm_min,
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n"
        "10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n"
        "20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n"
        "30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n"
        "40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n"
        "50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
        LV_ROLLER_MODE_INFINITE);
    lv_obj_set_style_text_font(roller_alarm_min, &lv_font_montserrat_14, 0);
    lv_obj_align(roller_alarm_min, LV_ALIGN_RIGHT_MID, -30, 15);

    btn_alarm_toggle = lv_btn_create(scr_settings);
    lv_obj_set_size(btn_alarm_toggle, 90, 30);
    lv_obj_align(btn_alarm_toggle, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_event_cb(btn_alarm_toggle, alarm_toggle_cb, LV_EVENT_CLICKED, NULL);
    alarm_status_label = lv_label_create(btn_alarm_toggle);
    lv_label_set_text(alarm_status_label, "OFF");
    lv_obj_center(alarm_status_label);

    lv_obj_add_event_cb(scr_settings, page_gesture_handler, LV_EVENT_GESTURE, NULL);
}

/**
 * @brief 创建主表盘页面 UI
 * @note  包含时钟标签(24pt)和日期标签(14pt)，注册手势回调
 */
static void create_home_page(void)
{
    scr_home = lv_scr_act();

    clock_label = lv_label_create(scr_home);
    lv_obj_set_style_text_font(clock_label, &lv_font_montserrat_24, 0);
    lv_label_set_text(clock_label, "00:00:00");
    lv_obj_align(clock_label, LV_ALIGN_CENTER, 0, -30);

    date_label = lv_label_create(scr_home);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(date_label, "2026/01/01");
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 20);

    lv_obj_add_event_cb(scr_home, page_gesture_handler, LV_EVENT_GESTURE, NULL);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
  /*LCD_Init();
  LCD_Clear(WHITE);

  LCD_Fill(0, 0, 239, 39, BLUE);
  LCD_Fill(0, 40, 239, 239, WHITE);

  LCD_ShowString(10, 50, "Hello World!", RED, WHITE);
  LCD_ShowString(10, 70, "CST816D Touch", BLUE, WHITE);

  if(CST816D_Init() == 0)
  {
      LCD_Fill(0, 200, 239, 239, GREEN);
  }
  else
  {
      LCD_Fill(0, 200, 239, 239, RED);
  }*/
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of lvgl_mutex */
  lvgl_mutexHandle = osMutexNew(&lvgl_mutex_attributes);

  /* creation of rtcTimerQueue */
  rtcQueueHandle = osMessageQueueNew(4, sizeof(RTC_DateTime_t), NULL);

  /* creation of sysEvent */
  sysEventHandle = osEventFlagsNew(NULL);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of rtcTimer */
  rtcTimerHandle = osTimerNew(RtcTimer, osTimerPeriodic, NULL, &rtcTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* creation of sleepTimer — 10s one-shot for auto screen-off */
  osTimerAttr_t sleepTimer_attr = { .name = "sleepTimer" };
  sleepTimerHandle = osTimerNew(SleepTimerCallback, osTimerOnce, NULL, &sleepTimer_attr);
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of taskLED0 */
  taskLED0Handle = osThreadNew(StartTask02, NULL, &taskLED0_attributes);

  /* creation of homeTask */
  homeTaskHandle = osThreadNew(StartHomeTask, NULL, &homeTask_attributes);

  /* creation of startTaskLvgl */
  startTaskLvglHandle = osThreadNew(StartTasklvgl, NULL, &startTaskLvgl_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* creation of touchTask */
  osThreadAttr_t touchTask_attr = {
    .name = "touchTask",
    .stack_size = 512 * 4,
    .priority = osPriorityAboveNormal,
  };
  touchTaskHandle = osThreadNew(StartTouchTask, NULL, &touchTask_attr);

  /* creation of rtcTask */
  osThreadAttr_t rtcTask_attr = {
    .name = "rtcTask",
    .stack_size = 512 * 4,
    .priority = osPriorityAboveNormal,
  };
  rtcTaskHandle = osThreadNew(StartRtcTask, NULL, &rtcTask_attr);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  I2S_Alarm_Init();
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
  osKernelStart();

  /* We should never get here */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  }
  /* USER CODE END WHILE */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_4
                          |GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : L1_Pin */
  GPIO_InitStruct.Pin = L1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(L1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB2 PB10 PB4
                           PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_4
                          |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief 触控采集任务 (touchTask)
 * @note  中断驱动：EXTI ISR 通过 vTaskNotifyGiveFromISR 唤醒本任务，
 *        消抖读取手势后通过 xTaskNotify + osEventFlagsSet 通知 sysTask
 */
void StartTouchTask(void *argument)
{
  CST816D_EnableTouchIRQ();  /* enable touch INT after scheduler started */
  for(;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  /* wait for INT pin ISR */
    uint8_t gesture = CST816D_GetGesture_Debounced(20);
    if(gesture == CST816D_GESTURE_SWIPE_LEFT || gesture == CST816D_GESTURE_SWIPE_RIGHT) {
      xTaskNotify(homeTaskHandle, gesture, eSetValueWithOverwrite);
      osEventFlagsSet(sysEventHandle, SYS_EVENT_TOUCH);
    } else if(gesture == CST816D_GESTURE_SINGLE_CLICK) {
      xTaskNotify(homeTaskHandle, CST816D_GESTURE_SINGLE_CLICK, eSetValueWithOverwrite);
      osEventFlagsSet(sysEventHandle, SYS_EVENT_TOUCH);
    }
  }
}

/**
 * @brief RTC 采集任务 (rtcTask)
 * @note  每 500ms 调用 RTC_Get_DateTime() 读取完整年月日时分秒，
 *        通过 osMessageQueuePut 发送到 rtcQueueHandle 供 sysTask 消费
 */
void StartRtcTask(void *argument)
{
  for(;;)
  {
    RTC_DateTime_t dt;
    RTC_Get_DateTime(&dt);
    osMessageQueuePut(rtcQueueHandle, &dt, 0, 0);
    osDelay(500);
  }
}

/**
 * @brief 休眠定时器回调
 * @note  10s 无触控超时后置位 SYS_EVENT_SLEEP 事件标志，sysTask 检测后息屏
 */
void SleepTimerCallback(void *argument)
{
  osEventFlagsSet(sysEventHandle, SYS_EVENT_SLEEP);
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the taskLED0 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* drvTask: 硬件驱动/日志任务，每 1s 通过 USART2 输出系统 tick */
  char log[64];
  for(;;)
  {
    int len = sprintf(log, "[LOG] tick=%lu\r\n", osKernelGetTickCount());
    HAL_UART_Transmit(&huart2, (uint8_t*)log, len, 100);
    osDelay(1000);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartHomeTask */
/**
* @brief Function implementing the homeTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartHomeTask */
void StartHomeTask(void *argument)
{
  /* USER CODE BEGIN StartHomeTask */
  /*
   * sysTask: 系统管理任务
   *   - 状态机: HOME(主表盘) / SETTINGS(设置页) / SLEEP(息屏)
   *   - 通过 xTaskNotifyWait 接收 touchTask 的手势通知
   *   - 通过 osMessageQueueGet 接收 rtcTask 的时间数据
   *   - 通过 osTimer + osEventFlags 实现 10s 无触控自动息屏
   *   - touchTask 手势触发时置位 SYS_EVENT_TOUCH 事件标志
   *   - 设置页确认后将 roller 值写入 RTC 并切回主表盘
   */
  osMutexAcquire(lvgl_mutexHandle, osWaitForever);
  create_home_page();
  create_settings_page();
  osMutexRelease(lvgl_mutexHandle);

  RTC_DateTime_t dt;
  uint8_t screen_on = 1;
  osTimerStart(sleepTimerHandle, 10000);  /* start 10s sleep timer */

  for(;;)
  {
    uint32_t gesture;
    BaseType_t notified = xTaskNotifyWait(0, 0xFFFFFFFF, &gesture, pdMS_TO_TICKS(50));

    if(notified == pdTRUE) {
      osTimerStart(sleepTimerHandle, 10000);  /* restart timer on touch */
      osEventFlagsClear(sysEventHandle, SYS_EVENT_TOUCH);  /* consume touch flag */
      if(I2S_Alarm_IsPlaying()) I2S_Alarm_Stop();
      if(!screen_on) {
        screen_on = 1;
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
      }
      osMutexAcquire(lvgl_mutexHandle, osWaitForever);
      lv_obj_t *cur = lv_scr_act();
      if(gesture == CST816D_GESTURE_SWIPE_LEFT || gesture == CST816D_GESTURE_SWIPE_RIGHT) {
        if(cur == scr_home) {
          lv_scr_load(scr_settings);
          sync_settings_rollers();
        } else {
          lv_scr_load(scr_home);
        }
      }
      osMutexRelease(lvgl_mutexHandle);
    }

    /* check sleep event flag from timer callback */
    if(screen_on) {
      uint32_t flags = osEventFlagsGet(sysEventHandle);
      if(flags & SYS_EVENT_SLEEP) {
        screen_on = 0;
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
        osEventFlagsClear(sysEventHandle, SYS_EVENT_SLEEP);
      }
    }

    while(osMessageQueueGet(rtcQueueHandle, &dt, NULL, 0) == osOK) {
      osMutexAcquire(lvgl_mutexHandle, osWaitForever);
      if(lv_scr_act() == scr_home) {
        lv_label_set_text_fmt(clock_label, "%02d:%02d:%02d", dt.hour, dt.min, dt.sec);
        lv_label_set_text_fmt(date_label, "20%02d/%02d/%02d", dt.year, dt.month, dt.date);
        if(alarm_time.enabled && dt.hour == alarm_time.hour &&
           dt.min == alarm_time.min && dt.sec < 5 && !I2S_Alarm_IsPlaying()) {
          I2S_Alarm_Start();
        }
        if(I2S_Alarm_IsPlaying() && dt.sec >= 5) {
          I2S_Alarm_Stop();
        }
      }
      if(lv_scr_act() == scr_settings) {
        if(settings_confirm) {
          RTC_DateTime_t new_dt = dt;
          new_dt.hour = lv_roller_get_selected(roller_hour);
          new_dt.min  = lv_roller_get_selected(roller_min);
          new_dt.sec  = 0;
          RTC_Set_DateTime(&new_dt);
          alarm_time.hour = lv_roller_get_selected(roller_alarm_hour);
          alarm_time.min  = lv_roller_get_selected(roller_alarm_min);
          lv_scr_load(scr_home);
          settings_confirm = 0;
        }
      }
      osMutexRelease(lvgl_mutexHandle);
    }
  }
  /* USER CODE END StartHomeTask */
}

/* USER CODE BEGIN Header_StartTasklvgl */
/**
* @brief Function implementing the startTaskLvgl thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTasklvgl */
void StartTasklvgl(void *argument)
{
  /* USER CODE BEGIN StartTasklvgl */
  /* guiTask: GUI 刷新任务，每 10ms 持 lvgl_mutex 调用 lv_task_handler() */
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for(;;)
  {
    osMutexAcquire(lvgl_mutexHandle, osWaitForever);
    lv_task_handler();
    osMutexRelease(lvgl_mutexHandle);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
  }
  /* USER CODE END StartTasklvgl */
}

/* RtcTimer function */
void RtcTimer(void *argument)
{
  /* USER CODE BEGIN RtcTimer */
  /* deprecated — RTC read handled by rtcTask */
  /* USER CODE END RtcTimer */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
