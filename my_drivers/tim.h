#ifndef TIM_A
#define TIM_A
#include "nrf.h"

#define rtc_cont 20 //1s定时

typedef struct
{
	uint8_t cnt;//RTC计时
	uint8_t sec_cnt;//RTC秒计时
	uint8_t flag;//定时，射频发送
}RadioTimeCnt_Typedef;

#endif

