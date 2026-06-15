总成线分组（触摸屏 + ESP8266 完整线序）  

组 1：电源总线（CN6 → 所有模块）  
红：CN6-4 (+3V3) → 屏幕 VCC、ESP8266 VCC/CH_PD  
黑：CN6-6 (GND) → 屏幕 GND、ESP8266 GND、触摸 GND  

组 2：ESP8266（UART6 → CN8）  
绿：PC6 (RX) → ESP TX  
橙：PC7 (TX) → ESP RX  
黑：共 GND  
常用指令  
AT+CWMODE=1  
AT+CWJAP="HUAWEI-E105YI","32701"  
#扫描当前可用的AP  
AT+CWLAP  
#退出wifi网络  
AT+CWQAP  
AT+CWJAP?  
AT+HTTPCLIENT=2,0,"http://www.baidu.com","baidu.com","/get",1  
ERROR  
AT+CIFSR  
AT+CIPDOMAIN="iot.espressif.cn"  
AT+CIPSTART="TCP","baidu.com",80,0  
AT+GMR  
AT version:1.2.0.0(Jul  1 2016 20:04:45)  
SDK version:1.5.4.1(39cb9a32)  
Ai-Thinker Technology Co. Ltd.  
Dec  2 2016 14:21:16  
OK  
固件太老,需要升级固件程序  

esp8266不执行输入回调  
cubeMX里面设置如下：  
USART6 选择 Asynchronous  
波特率：115200  
打开 NVIC  使能 USART6 全局中断  
如果用 FreeRTOS：  
SYS  Timebase Source = TIM1  

整合图形库  
lvgl-8.3.11  
组 3：SPI 显示（ILI9341 → CN7）  
--        GND     电源地  
          VCC     3.3v电源  
          SCK     PB3  SPI时钟信号SPI1-SCK  
          SDA     PA7  SPI数据信号SPI1-MOSI  
          RES     PB1  复位脚  
          DC      PB2  数据指令选择脚  
          CS      PB4  片选  
--		  BLC     PB8  控制背光  

组 4：IIC 触摸（XPT2046 → CN7）  
触        SDA     PB7  触摸IIC 数据线  
摸        SCL     PB6  触摸IIC 时钟线  
接        RST     PB9  触摸复位脚  
口        INT     PB10  触摸中断  
nucleo-f401re接入触摸板芯片CST816D使用是I2C，SDA接PB7，SCL接PB6，RST接PB9，INT接PB10，编写使用例子  
The STM32F401xD/xE incorporate high-speed embedded memories (512 Kbytes of flash memory, 96 Kbytes of SRAM)  
加入显示模块lk.h与lk.c  
加入显示厂家模块  
lcd.h,lcd.c,lcdfont.h,lcdfont.c模块  