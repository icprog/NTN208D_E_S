#include "App_Common.h"
#define App_Protocol
#define App_Crc16
#define App_Var
#define App_Uart
#define App_Str
#define App_Init
#include "App_Libraries.h"
#include "string.h"
/************************************************* 
从机->主机
@Description:
@Input:
buff
状态
@Output:无
@Return:返回长度
*************************************************/ 	
void Com_ChannelConfigAck_To_Master(u8 *temp,u16 state)
{	
	uint16_t crc,crc_idx,t_Len;
	
	temp[US_HEADER_IDX] = pkt_head1;
	temp[US_HEADER_IDX+1] = pkt_head2;
	t_Len = US_AfterLEN_FIX_LEN +  ChannelConfigAckInfoLen;
	temp[US_LEN_IDX] =  t_Len>>8;
	temp[US_LEN_IDX+1] =  t_Len;
	temp[US_CHANNEL_IDX] = ReaderPara.Channel485_Num;//通道
	temp[US_CMD_IDX] = US_CMD_CHANNEL_CONFIG_ACK;//命令
	temp[US_DATA_IDX] = state>>8;
	temp[US_DATA_IDX+1] = state;
	
	crc = crc16(&temp[US_LEN_IDX],(t_Len+US_LENTH_LEN-US_CRC_LEN));//长度~信息内容
	crc_idx = US_HEAD_LEN + US_LENTH_LEN + t_Len - US_CRC_LEN; //帧头长度+长度长度+长度后面的长度 - CRC长度	
	temp[crc_idx] = (crc>>8);
	temp[crc_idx+1] = crc;//crc
	t_Len = US_FIX_LEN+t_Len;//固定长度：帧头2字节，长度2字节
	SlaveTxEn();
	UART_Send(temp,t_Len);
	SlaveRxEn();	
}

/************************************************* 
从机->主机
@Description:获取版本
@Input:
buff
状态
@Output:无
@Return:
*************************************************/ 	
void Com_GetVerAck_To_Master(u8 *temp,u8* tVerName)
{	
	uint16_t crc,crc_idx,t_Len;
	uint8_t tVerNameLen;
	temp[US_HEADER_IDX] = pkt_head1;
	temp[US_HEADER_IDX+1] = pkt_head2;
	tVerNameLen = app_strlen(tVerName)+1;
	t_Len = US_AfterLEN_FIX_LEN +  GetVerFixInfoLen + tVerNameLen;
	temp[US_LEN_IDX] =  t_Len>>8;
	temp[US_LEN_IDX+1] =  t_Len;
	temp[US_CHANNEL_IDX] = ReaderPara.Channel485_Num;//通道
	temp[US_CMD_IDX] = US_CMD_GET_VER_ACK;//命令
	memcpy(&temp[US_DATA_IDX],tVerName,tVerNameLen);
	crc = crc16(&temp[US_LEN_IDX],(t_Len+US_LENTH_LEN-US_CRC_LEN));//长度~信息内容
	crc_idx = US_HEAD_LEN + US_LENTH_LEN + t_Len - US_CRC_LEN; //帧头长度+长度长度+长度后面的长度 - CRC长度	
	temp[crc_idx] = (crc>>8);
	temp[crc_idx+1] = crc;//crc
	t_Len = US_FIX_LEN+t_Len;//固定长度：帧头2字节，长度2字节
	SlaveTxEn();
	UART_Send(temp,t_Len);	
	SlaveRxEn();
}


/************************************************* 
从机->主机
@Description:升级确认包
@Input:
buff
代码序号
状态
@Output:无
@Return:
*************************************************/ 	
void Com_Update_Ack_To_Master(u8 *temp,u8 state)
{	
	uint16_t crc,crc_idx,t_Len;
	temp[US_HEADER_IDX] = pkt_head1;
	temp[US_HEADER_IDX+1] = pkt_head2;
	t_Len = US_AfterLEN_FIX_LEN +  UpdateAckFixInfoLen;
	temp[US_LEN_IDX] =  t_Len>>8;
	temp[US_LEN_IDX+1] =  t_Len;
	temp[US_CHANNEL_IDX] = ReaderPara.Channel485_Num;//通道
	temp[US_CMD_IDX] = US_CMD_UPDATE_ACK;//命令
	temp[US_DATA_IDX] = ReaderPara.AppCodeNum;//命令
	temp[US_DATA_IDX+1] = state;//状态
	crc = crc16(&temp[US_LEN_IDX],(t_Len+US_LENTH_LEN-US_CRC_LEN));//长度~信息内容
	crc_idx = US_HEAD_LEN + US_LENTH_LEN + t_Len - US_CRC_LEN; //帧头长度+长度长度+长度后面的长度 - CRC长度	
	temp[crc_idx] = (crc>>8);
	temp[crc_idx+1] = crc;//crc
	t_Len = US_FIX_LEN+t_Len;//固定长度：帧头2字节，长度2字节
	SlaveTxEn();
	UART_Send(temp,t_Len);
	SlaveRxEn();	
}





