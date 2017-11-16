#ifndef _APP_INIT_H
#define _APP_INIT_H
#include "App_Sys.h"

//io定义
#define IO_LP_State 0x00000002	
#define IO_OUTPUT  	0X00000003
#define IO_INPUT  	0X00000000
#define IO_INPUT_Pulldown  	0X00000004
#define IO_INPUT_Pullup  	0X0000000C

//串口定义
//#define RX_PIN_NUMBER  11    // UART RX pin number.
//#define TX_PIN_NUMBER 9   // UART TX pin number.
#define RX_PIN_NUMBER  13    // UART RX pin number.
#define TX_PIN_NUMBER 15   // UART TX pin number.
#define CTS_PIN_NUMBER 6   // UART Clear To Send pin number. Not used if HWFC is set to false
#define RTS_PIN_NUMBER 7    // Not used if HWFC is set to false 
#define HWFC           false // UART hardware flow control
#define TX_EN_NUMBER  14    // 使能引脚
#define SlaveTxEn() do{NRF_GPIO->OUTSET = (1 << TX_EN_NUMBER);}while(0);
#define SlaveRxEn() do{NRF_GPIO->OUTCLR = (1 << TX_EN_NUMBER);}while(0);

#define TIM0_TIME 50  //50MS计数一次

//中断优先级定义
typedef enum
{
    APP_IRQ_PRIORITY_HIGHEST = 0,
    APP_IRQ_PRIORITY_HIGH    = 1,
    APP_IRQ_PRIORITY_MID     = 2,
    APP_IRQ_PRIORITY_LOW     = 3
} app_irq_priority_t;


#define RADIO_PRIORITY		APP_IRQ_PRIORITY_HIGH
#define RTC_PRIORITY		APP_IRQ_PRIORITY_HIGHEST
#define PORT_PRIORITY    	APP_IRQ_PRIORITY_HIGH
#define UART0_PRIORITY 		APP_IRQ_PRIORITY_HIGHEST
#define TIM0_PRIORITY      	APP_IRQ_PRIORITY_LOW

//ext function
extern void rtc_init(void);//1s定时，用来时间定时和射频周期发送
extern void rtc_update_interval(void);//增加随机时间
extern void xosc_hfclk_start(void);//射频发送，需要启动外部16M晶振
extern void xosc_hfclk_stop(void);//停止射频发送时，关闭外部晶振
extern void app_init(void);
void UART_Init(void);


#endif

