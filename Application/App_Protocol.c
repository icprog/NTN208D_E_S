#include "App_Protocol.h"
#define App_Sys
#define App_Crc16
#define App_Var
#define App_Common
#define App_NVMC
#include "App_Libraries.h"
#define BackupCodeStartAddr    0x1B000
uint32_t BackupCodeAddr = BackupCodeStartAddr;
UART_Typedef US_Master;//定义串口缓冲
const uint8_t App_Cur_Name[] = "NTN208D-ESCF22V01";//包含结束符，使用Seziof计算包含'/0'的长度
uint8_t UpPktName[32]; //存储升级包名字
Update_Typdef UpdatePkt = {
	.PktNum = 0,//包总数
	.Name= UpPktName,//升级包名称
	.NameLen = 0,
	.RxState = 0,//接收状态
	.TxState = 0,//发送状态
	.Data = 0,//升级包数据
};
/************************************************* 
@Description:校验内容，长度~信息内容
@Input:Src 帧头~CRC值
@Output:无
@Return:1-校验成功
*************************************************/  
u8 CRC16_Check(uint8_t* Src)
{
	uint16_t crc_len = (Src[US_LEN_IDX]<<8)|Src[US_LEN_IDX+1] + US_LENTH_LEN - US_CRC_LEN;//长度字段包含CRC
	uint16_t crc_src = (Src[crc_len+US_HEAD_LEN] << 8) | Src[crc_len+US_HEAD_LEN+1];//CRC值
	uint16_t crc_data = crc16(&Src[US_LEN_IDX],crc_len);
	if(crc_src == crc_data)
		return 1;
	else
		return 0;
}

/************************************************* 
@Description:数据帧校验
@Input:
@Output:无
@Return:1-校验成功
*************************************************/  
u8 DataFrameCheck(uint8_t* data)
{
	uint8_t Channel485;
	Channel485 = data[US_CHANNEL_IDX];
	//485通信需要检查通道号
	if(Channel485 !=0xff && Channel485 != ReaderPara.Channel485_Num)
		return 0;
	if(0 == CRC16_Check(data))//crc校验
		return 0;
	return 1;//校验成功
}
/************************************************* 
@Description:擦除备份扇区
@Input:
@Output:
@Return:
*************************************************/  
void BackupCodeErase(void)
{
	uint8_t i=0;
	BackupCodeAddr = BackupCodeStartAddr;
	for(i=0;i<100;i++)
	{
		nrf_nvmc_page_erase(BackupCodeAddr);
		BackupCodeAddr=BackupCodeAddr+0x400;
	}
	BackupCodeAddr = BackupCodeStartAddr;
}

/************************************************* 
@Description:写入备份区
@Input:
@Output:
@Return:
*************************************************/  
void BackupCodeWrite(uint8_t* Src,uint8_t Num)
{
	uint8_t k=0;
	nrf_nvmc_write_bytes(BackupCodeAddr, Src, Num);
	for(k = 0; k < Num ; k++)
	{
		if((*(uint8_t *)(BackupCodeAddr + k)) != Src[k])
		{
			uint32_t t;
			nrf_nvmc_write_bytes(BackupCodeAddr, Src, Num);
			for(t = 0; t < Num ; t++)
			{
				if((*(uint8_t *)(BackupCodeAddr + k)) != Src[k])
				{
					return; // unfortunately, upgrade is faild and irretrievable
				}
			}
		}
	}
	BackupCodeAddr += Num;
}
/************************************************* 
@Description:命令处理
@Input:
@Output:无
@Return:无
串口通信 上位机->接收器
写命令F0
参数设置信息内容
目标ID：XXXXXXXX 4字节
超时时间:0~9  0：无超时时间  单位s
保留：0000
区选择：01~04
写最新参数:FFFF
写长度：01~10
数据内容：字节数，最大16字节
-----------------------------------------------------
读命令F1
目标ID：XXXXXXXX 4字节
超时时间:0~9  0：无超时时间  单位s
保留：0000
区选择：01~04
读最新参数:FFFF  参数区 保留区 0~15 用户区1、2 0~31
长度 01~10
******************************************************/
void Pro_Communication(void)
{
	uint8_t US_CMD;
	uint8_t US_DATA_LEN;
	uint16_t state;
	uint8_t msg_seq;//消息编号
	if(US_Master.has_finished)
	{
		US_Master.has_finished = 0;
		if(DataFrameCheck(US_Master.rx_buf))//数据帧校验
		{
			US_DATA_LEN = ((US_Master.rx_buf[US_LEN_IDX]<<8)|US_Master.rx_buf[US_LEN_IDX+1]) - US_AfterLEN_FIX_LEN;//信息长度
			US_CMD = US_Master.rx_buf[US_CMD_IDX];//命令处理
			switch(US_CMD)
			{
				case US_CMD_CHANNEL_CONFIG://分配485通道
				{
					if(Reset_Channel485 == US_Master.rx_buf[US_DATA_IDX+4])//重置设备ID
					{
						ResetDeviceID();
						state =US_RUN_SUCCESS;
					}
					else if(Channel485_UnConfig == ReaderPara.Channel485_Num)// 通道未配置
					{
						UpdateDeviceID(&US_Master.rx_buf[US_DATA_IDX]);
						state =US_RUN_SUCCESS;
					}
					else
					{
						state = US_CHANNEL_ERR;//失败
					}
					Com_ChannelConfigAck_To_Master(US_Master.tx_buf,state);
				}	
				break;
				case US_CMD_GET_VER://获取版本
				{
					Com_GetVerAck_To_Master(US_Master.tx_buf,(u8*)App_Cur_Name);
				}
				break;	
				case US_CMD_ENTER_UPDATE://触发升级
				{
					uint8_t CodeNum = US_Master.rx_buf[US_DATA_IDX];
					if(CodeNum == ReaderPara.AppCodeNum )//
					{
						UpdatePkt.CurPktNum = 0;//当前包
						BackupCodeErase();
						Com_Update_Ack_To_Master(US_Master.tx_buf,Update_Rx_Ok);
					}
					else
					{
						Com_Update_Ack_To_Master(US_Master.tx_buf,Update_Rx_CodeNumError);
					}
				}
				break;
				case US_CMD_UPDATE_SDATA://升级数据包
				{
					uint8_t CodeNum = US_Master.rx_buf[US_DATA_IDX];
					uint8_t tPktNum = (US_Master.rx_buf[US_DATA_IDX+1]<<8) |US_Master.rx_buf[US_DATA_IDX+2];//获取下发的包编号
					if(CodeNum == ReaderPara.AppCodeNum )//包序号相同
					{
						if(UpdatePkt.CurPktNum == tPktNum)//包编号是否相同
						{
							UpdatePkt.DataLen = US_DATA_LEN - UpdateDataFixInfoLen;
							if(UpdatePkt.DataLen == UpDataLen)//数据长度固定200		
							{
								UpdatePkt.Data = &US_Master.rx_buf[US_DATA_IDX+3];//升级数据
								BackupCodeWrite(UpdatePkt.Data,UpdatePkt.DataLen);
								Com_Update_Ack_To_Master(US_Master.tx_buf,Update_Rx_Ok);	
								UpdatePkt.CurPktNum++;
							}	
							else
							{
								Com_Update_Ack_To_Master(US_Master.tx_buf,Update_Rx_CodeLenError);
							}
						}
						else
						{
							Com_Update_Ack_To_Master(US_Master.tx_buf,Update_Rx_Same);
						}
					}
					else
					{
						Com_Update_Ack_To_Master(US_Master.tx_buf,Update_Rx_CodeNumError);
					}
				}
				break;
				case US_CMD_UPDATA_END:
				{
					if(US_Master.rx_buf[US_DATA_IDX] == 1)//传输完成
					{
						Com_Update_Ack_To_Master(US_Master.tx_buf,Update_Rx_Ok);
					}
				}
				break;
			}				
		}
	}
}



/************************************************* 
@Description:记录串口数据
@Input:
@Output:无
@Return:无
*************************************************/  
void Uart_ReceiveBuff(uint8_t rx_temp)
{
	switch(US_Master.rx_state)
	{
		case PKT_HEAD1://帧头1
			if(pkt_head1 == rx_temp)	                                
			{                                                   
				US_Master.rx_idx = 0;//索引号为0
				US_Master.rx_buf[US_Master.rx_idx] = rx_temp;//缓存数据
				US_Master.rx_state = PKT_HEAD2;//状态切换
			}
			break;
		case PKT_HEAD2:
			if(pkt_head2 ==  rx_temp)
			{
				US_Master.rx_idx++;		
				US_Master.rx_buf[US_Master.rx_idx] = rx_temp;
				US_Master.rx_state = PKT_PUSH_LEN;
			}
			else
			{
				US_Master.rx_state = PKT_HEAD1;
			}
			break;
		case PKT_PUSH_LEN:
			US_Master.rx_idx++;
			US_Master.rx_buf[US_Master.rx_idx] = rx_temp;
			if(US_Master.rx_idx == len_finish)//长度接收完成
			{
				US_Master.rx_len = (( US_Master.rx_buf[US_Master.rx_idx-1] << 8 ) | US_Master.rx_buf[US_Master.rx_idx]);
				if(US_Master.rx_len>250)
				{
					US_Master.rx_state = PKT_HEAD1;
				}
				else
				{
					US_Master.rx_state = PKT_DATA;
				}
			}
			break;
		case PKT_DATA://缓存数据
			if(US_Master.rx_len > 0)
			{
				US_Master.rx_idx++;
				US_Master.rx_buf[US_Master.rx_idx] = rx_temp;	
				US_Master.rx_len--;
			}
			if(0 == US_Master.rx_len )
			{
//				US_Master.rx_state = PKT_CRC;
				US_Master.has_finished = 1;
				US_Master.rx_state = PKT_HEAD1;
			}
			break;
//		case PKT_CRC:
//			US_Master.rx_idx++;
//			US_Master.rx_buf[US_Master.rx_idx] = rx_temp;	
//			US_Master.rx_len++;
//			if(crc_bytes == US_Master.rx_len)
//			{
//				US_Master.has_finished = 1;
//				US_Master.rx_state = PKT_HEAD1;
//			}
//			break;
		default : US_Master.rx_state = PKT_HEAD1;
	}
}
/************************************************* 
@Description:串口中断
@Input:
@Output:无
@Return:无
*************************************************/  
void UART0_IRQHandler()
{
	uint8_t temp;
	if(NRF_UART0->EVENTS_RXDRDY)
	{
		NRF_UART0->EVENTS_RXDRDY = 0;
		temp = NRF_UART0->RXD;
		Uart_ReceiveBuff(temp);
	}
	else if(NRF_UART0->EVENTS_ERROR)
	{
		NRF_UART0->EVENTS_ERROR = 0;
	}
}

