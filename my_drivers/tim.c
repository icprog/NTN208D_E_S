#include "tim.h"
#include "app_var.h"
#include "rtc.h"
#include "slave_uart.h"
extern rtc_typedef Global_Time;//全局时间 
extern ReaderPara_Typedef ReaderPara;
uint8_t rtc_cnt =rtc_cont;//rtc秒计数器
extern Time_Typedef Time_type;//超时处理
uint8_t sec_flag;//秒计数
RadioTimeCnt_Typedef RadioTime;
extern ReaderPara_Typedef ReaderPara;//读写器模式
void TIMER0_IRQHandler()
{
	if(NRF_TIMER0->EVENTS_COMPARE[0])
	{
		NRF_TIMER0->EVENTS_COMPARE[0] = 0;
		//周期发送
		if(ReaderPara.SendEn)
		{
			ReaderPara.SendBaseTime++;
			if(ReaderPara.SendBaseTime > 4)
			{
				ReaderPara.SendBaseTime = 0;
				ReaderPara.SendCnt++;//基准250ms
				if(ReaderPara.SendCnt > ReaderPara.Send_Period)
				{
					ReaderPara.SendCnt = 0;
					ReaderPara.SendFlag = 1;
				}				
			}

		}
		//时钟
		rtc_cnt++;
		if(rtc_cont == rtc_cnt)
		{
			Calendar21Century(&Global_Time);//更新时间
			rtc_cnt = 0;
			ReaderPara.LeaveCheckFlag = 1;//1s检测标签离开情况
		}
		//超时
		if(Time_type.Radio_Time_En)
		{
			Time_type.Radio_Time_Cnt++;//
		}
			
	}
}

