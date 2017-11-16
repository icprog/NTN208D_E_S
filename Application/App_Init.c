#include "App_Init.h"
#define App_Uart
#define App_Var
#include "App_Libraries.h"

#define rtc_interval 1  //单位s
#define rtc_base ((32768*rtc_interval) - 1)
/************************************************* 
@Description:配置低频时钟时钟源  
@Input:
（1）、source : 1:选择外部晶振 XOSC 0：内部rc ROSC
@Output:无
@Return:无
*************************************************/  
static void lfclk_init(uint8_t source)
{
	uint8_t lfclksrc;
	NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
	//选择时钟源
	lfclksrc = source ? CLOCK_LFCLKSRC_SRC_Xtal : CLOCK_LFCLKSRC_SRC_RC;
	NRF_CLOCK->LFCLKSRC = lfclksrc << CLOCK_LFCLKSRC_SRC_Pos;
	NRF_CLOCK->TASKS_LFCLKSTART = 1;
	while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) 
	{
	}	
}

/************************************************* 
@Description:产生0~255随机数 
@Input:无
@Output:输出随机值
@Return:无
*************************************************/ 
//uint8_t rng_value = 0;
static uint8_t random_vector_generate()
{
	static uint8_t rng_value = 0;
	rng_value = NRF_RNG->VALUE;
	NRF_RNG->EVENTS_VALRDY = 0;
	NRF_RNG->SHORTS = RNG_SHORTS_VALRDY_STOP_Enabled << RNG_SHORTS_VALRDY_STOP_Pos;
	NRF_RNG->TASKS_START = 1;
	return rng_value;
}
/************************************************* 
@Description:rtc初始化  
@Input:
@Output:无
@Return:无
*************************************************/  
void rtc0_init(void)
{
	lfclk_init(1);//1：XOSC 0：ROSC
	NRF_RTC0->PRESCALER = 0;//32.768khz 约等于0.03ms
	NRF_RTC0->CC[0] = rtc_base;//
	NRF_RTC0->EVENTS_COMPARE[0] = 0;//EVENTS_TICK
	NRF_RTC0->INTENSET =  RTC_INTENCLR_COMPARE0_Msk;//
//	NRF_RTC0->TASKS_START = 1;
	
	NVIC_SetPriority(RTC0_IRQn,RTC_PRIORITY);
	NVIC_ClearPendingIRQ(RTC0_IRQn);
	NVIC_EnableIRQ( RTC0_IRQn );
}
/************************************************* 
@Description:rtc启动计数
@Input:
@Output:无
@Return:无
*************************************************/  
void rtc0_start(void)
{
	NRF_RTC0->TASKS_START = 1;
}

/************************************************* 
@Description:rtc停止计数
@Input:
@Output:无
@Return:无
*************************************************/  
void rtc0_stop(void)
{
	NRF_RTC0->TASKS_STOP = 1;
}
/************************************************* 
@Description:广播间隔增加0~7.65ms的随机延时
@Input:无
@Output:无
@Return:无
*************************************************/ 
void rtc_update_interval(void)
{
	uint8_t advDelay = random_vector_generate();//0~255 0~7.65ms
	NRF_RTC0->CC[0] = rtc_base + advDelay;
}
/************************************************* 
@Description:启动外部晶振
@Input:无
@Output:无
@Return:无
*************************************************/ 
void xosc_hfclk_start(void)
{
	/*当外部晶振未启动时，才启动外部晶振*/
	if((NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKSTAT_SRC_Xtal)!=1)
	{
		/*clear event*/
		NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
		/* Start 16 MHz crystal oscillator */		
		NRF_CLOCK->TASKS_HFCLKSTART = 1;

		/* Wait for the external oscillator to start up */
		while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) 
		{
			
		}
		//等待HFCLK running
		while((NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKSTAT_STATE_Msk) == 0)
		{
		}
	}		
}

/************************************************* 
@Description:关闭外部晶振
@Input:无
@Output:无
@Return:无
*************************************************/ 
void xosc_hfclk_stop(void)
{
	/**/
	if((NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKSTAT_SRC_Xtal) == 1)
	{
		NRF_CLOCK->TASKS_HFCLKSTOP = 1;
	}		
}
/************************************************* 
@Description:定时器初始化
@Input:毫秒精确定时
@Output:无
@Return:无
*************************************************/ 
void timer0_init(uint8_t delayms)
{
	NRF_TIMER0->SHORTS     = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);
	NRF_TIMER0->MODE           = TIMER_MODE_MODE_Timer;        // Set the timer in Timer Mode.
	NRF_TIMER0->TASKS_CLEAR    = 1;                            // clear the task first to be usable for later.
	NRF_TIMER0->PRESCALER      = 4;                            // 1us
	NRF_TIMER0->BITMODE        = (TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos); // 16 bit mode.
	NRF_TIMER0->INTENSET      = (TIMER_INTENSET_COMPARE0_Enabled<<TIMER_INTENSET_COMPARE0_Pos);
	NRF_TIMER0->CC[0]       = (uint32_t)delayms * 1000;
//	NRF_TIMER0->TASKS_START    = 1; // Start timer. 
	NVIC_SetPriority(TIMER0_IRQn, 2);
	NVIC_EnableIRQ(TIMER0_IRQn);
}
/************************************************* 
@Description:定时器启动计数
@Input:
@Output:无
@Return:无
*************************************************/ 
void timer0_start(void)
{
	NRF_TIMER0->TASKS_START = 1;
}


/************************************************* 
@Description:定时器停止计数
@Input:
@Output:无
@Return:无
*************************************************/ 
void timer0_stop(void)
{
	NRF_TIMER0->TASKS_START = 1;
}
	
/************************************************* 
@Description:串口初始化
@Input:无
@Output:无
@Return:无
*************************************************/ 
void UART_Init(void)
{
	
	simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, HWFC);  
	NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos)
						|(UART_INTENSET_ERROR_Enabled << UART_INTENSET_ERROR_Pos);
	NRF_GPIO->PIN_CNF[TX_EN_NUMBER]=IO_OUTPUT;	//485发送使能引脚
	NVIC_SetPriority(UART0_IRQn, UART0_PRIORITY);
	NVIC_EnableIRQ(UART0_IRQn);
}

/************************************************* 
Description:app初始化
Input:无
Output:
Return:无
*************************************************/ 
void app_init(void)
{
	SystemParaInit();
	UART_Init();
//	#ifdef LOG_ON
//	debug_log_init();
//	#endif
////	debug_printf("app_reader start");
//	Radio_Init();//射频初始化
//	TID_RECORD_Clear();//清空缓存
//	rtc0_init();
//	timer0_init(TIM0_TIME);//50ms定时
//	radio_select(DATA_CHANNEL,RADIO_RX);//进入接收状态
//	timer0_start();//开始计数
}




