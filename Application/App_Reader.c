/*******************************************************************************
** 版权:		
** 文件名: 		app_reader.c
** 版本：  		1.0
** 工作环境: 	MDK-ARM 5.23
** 作者: 		cc
** 生成日期: 	2017-07-26
** 功能:		  
** 相关文件:	app_key.h
** 修改日志：	
** 版权所有   
*******************************************************************************/
#include "App_Reader.h"
#define App_Sys
#define App_Var
#define App_Uart
#define App_Crc16
#define App_Radio
#define App_Rtc
#define App_Protocol
#include "App_Libraries.h"
extern Payload_Typedef cmd_packet;//命令射频处理
extern volatile Radio_State_Typedef Radio_State;//射频工作状态
extern TID_Typedef  TID_RECORD[CAPACITY];
uint8_t Work_Mode = Idle;//工作模式
uint8_t tx_buf[254];//串口发送
extern UART_Typedef US_Master;//定义串口缓冲
extern uint8_t Hour;//一个小时过后，停止时间更新功能
extern rtc_typedef Global_Time;
//设备ID
extern uint8_t DeviceID[4];
extern Time_Typedef Time_type;//超时处理
extern uint8_t sec_flag;//秒计数
//过滤
extern Filter_Typedef Filter_Radio;
extern ReaderPara_Typedef ReaderPara;//读写器模式
extern RSSIFilter_Typedef RSSIFilter;//门限值
typedef struct
{
	uint8_t max_pkt;//最大分包编号
	uint8_t crt_pkt;//当前分包编号
}U_PKT_Typdef;
U_PKT_Typdef U_TAG_PKT;
#define MAX_TAG_NUM  56  //每包最大标签个数 (1024-10)/18=56
#define TAG_INFO_LEN 18  //最大标签内容长度 
//#define MAX_PKT_Pos    4
//#define CRT_PKT_Msk    0x0f


void Read_Tag_Record(void)
{

}
/************************************************* 
@Description:根据串口命令，工作在不同的模式下
@Input:无
@Output:无
@Return:无
*************************************************/ 
void app_process(void)
{
}
