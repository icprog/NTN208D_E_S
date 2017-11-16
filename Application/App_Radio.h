#ifndef _APP_RADIO_H
#define _APP_RADIO_H
#include "radio_config.h"
#include "nrf.h"
#include "app_init.h"
#include "app_var.h"


typedef enum
{
	RADIO_RUN_CONFIG_CHANNEL ,
	RADIO_RUN_DATA_CHANNEL
}RADIO_CHANNEL;

typedef enum
{
	Time_NoUpdate = 0,
	Time_Update=1
}Time_Update_Typedef;
/***************************************
				公共定义
****************************************/
#define win_interval 1//标签开接收窗口间隔
//携带命令
typedef enum
{
	WithoutCmd = 0,
	WithCmd
}CMD_Typedef;
//携带接收窗
typedef enum
{
	WithoutWin = 0,
	WithWin
}WIN_Typedef;

//是否等待发送完成
typedef enum
{
	SendNoWait=0,
	SendWait=1
}Send_Wait_Typedef;
typedef struct
{
	uint8_t radio_rcvok;//射频接收完成
	uint8_t radio_sndok;//射频发送完成
	uint8_t radio_run_channel;//射频运行通道 
}Radio_State_Typedef;


extern Payload_Typedef cmd_packet;//射频缓冲区 命令射频处理
/*------------------------------------------------------------------------*/
extern void TID_RECORD_Clear(void);
extern void radio_pwr(uint8_t txpower);
extern void Reader_RadioDeal(void);//射频周期发送
extern void radio_select(uint8_t ch,uint8_t dir);
void Radio_Period_Send(uint8_t cmdflag,uint8_t Channel,uint8_t winflag,uint8_t wait_send_finish);
uint8_t ID_CMP(const uint8_t *src,const uint8_t *dest);
#endif


