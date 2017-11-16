#include "App_Radio.h"
#define App_Uart
#define App_Msg
#define App_Rtc
#define App_Protocol
#include "App_Libraries.h"
#include "nrf_delay.h"
#include "string.h" 
#include "app_reader.h"
/*****************************************************************
		接收器参数定义
*****************************************************************/
#define RADIO_RX_OT 	825
extern uint8_t Work_Mode;//工作模式
//消息
uint8_t MSG_PUSH_State = MSG_IDLE;//消息推送状态
//串口缓冲
extern UART_Typedef US_Master;
//过滤
extern Filter_Typedef Filter_Radio;
//读写器-记录标签
TID_Typedef  TID_RECORD[CAPACITY];
//时间设置
extern uint8_t Need_Time_Set;//1：允许设置时间0：不允许设置时间
extern RSSIFilter_Typedef RSSIFilter;//门限值
extern ReaderPara_Typedef ReaderPara;
/***************************************************************
			公共参数定义
****************************************************************/
extern uint8_t packet[PACKET_PAYLOAD_MAXSIZE];
extern uint8_t DeviceID[RADIO_RID_LENGTH];
extern uint8_t para_record[PARA_RECORD_LEN];
extern uint8_t radio_status;//射频运行状态
extern MSG_Store_Typedef MSG_Store;//消息定义消息序列号
//消息
extern Message_Typedef Msg_Packet;//消息推送与接收
volatile Radio_State_Typedef Radio_State;//射频工作状态
Payload_Typedef cmd_packet;//射频缓冲区 命令射频处理
Serial_Typedef DEV_SER;//设备流水
//文件操作
File_Typedef f_para;//文件操作命令数据缓存
/****************************************************************
			公共函数
****************************************************************/
void radio_pwr(uint8_t txpower);
void Reader_RadioCmdDeal(void);//自身参数设置

/*
@Description:清空缓冲记录
@Input:state :
@Output:无
@Return:无
*/
void TID_RECORD_Clear(void)
{
	uint16_t i;
	for(i=0;i<CAPACITY;i++)
	{
		memset(&TID_RECORD[i],0xff,sizeof(TID_Typedef));
	}
	
}
/*
@Description:公共函数-射频启动
@Input:state :
@Output:无
@Return:无
*/
static void radio_on(void)
{
	xosc_hfclk_start();//外部晶振起振
}
/*
Description:公共函数-射频关闭
Input:state :
Output:无
Return:无
*/
static void radio_off(void)
{
	radio_disable();
	xosc_hfclk_stop();
}

/*
Description:公共函数-射频选择
Input:state :
Output:无
Return:无
*/
void radio_select(uint8_t ch,uint8_t dir)
{
	uint8_t channel;
	radio_on();//开启晶振
	channel = (ch == DATA_CHANNEL)?RADIO_CHANNEL_DATA:RADIO_CHANNEL_CONFIG;
	Radio_State.radio_run_channel = (ch == DATA_CHANNEL)?RADIO_RUN_DATA_CHANNEL:RADIO_RUN_CONFIG_CHANNEL;
	#if WHITEEN
	NRF_RADIO->DATAWHITEIV = channel;//数据白化
	#endif
	if(dir == RADIO_TX)
	{
		radio_tx_carrier(RADIO_MODE_MODE_Nrf_1Mbit,channel);
		
	}
	else if(dir == RADIO_RX)
	{
		radio_rx_carrier(RADIO_MODE_MODE_Nrf_1Mbit,channel);
	}
}

/*
@Description:读写器-射频处理
@Input:state :
@Output:无
@Return:无
*/
void Reader_RadioDeal(void)
{
//	uint32_t ot;
//	//平时处于接收状态，偶尔插入一次发送
	if(ReaderPara.SendFlag)
	{
		ReaderPara.SendFlag = 0;
		Radio_Period_Send(WithoutCmd,DATA_CHANNEL,WithoutWin,SendWait);//等待发送完成
//		//后续考虑是否允许通过配置频道更改射频参数
//		ot = RADIO_RX_OT;//接收窗时间
//		while(--ot)
//		{
//			if(Radio_State.radio_rcvok)
//				break;
//				
//		}	
//		if(Radio_State.radio_rcvok)//接收成功，设置自身参数
//		{
//			Radio_State.radio_rcvok = 0;
//			Reader_RadioCmdDeal();//命令处理
//		}		
		radio_select(DATA_CHANNEL,RADIO_RX);
	}

//		

}

/***********************************************************
@Description:公共函数-周期发送射频信息
@Input：	cmdflag - 命令返回 1：返回命令 0：常规发送
			winflag - 是否携带窗口
			wait_send_finish:1:等待发送完成，0:不等待
@Output：无
@Return:无
************************************************************/
void Radio_Period_Send(uint8_t cmdflag,uint8_t Channel,uint8_t winflag,uint8_t wait_send_finish)
{
	uint32_t ot = 0;
	while(1 == radio_tx_isbusy())
	{
		ot++;
		if(ot > RADIO_OVER_TIME)
			return;
	}
	my_memset(packet,0,PACKET_PAYLOAD_MAXSIZE);
	if(cmdflag)
	{
		memcpy(packet,cmd_packet.packet,cmd_packet.length+RADIO_HEAD_LENGTH);//更新所有包
	}
	else
	{
		//标签身份发送
		packet[RADIO_LENGTH_IDX] = 0; //payload长度，后续更新
		packet[RADIO_S1_IDX] = 0x00;
		packet[TAG_SER_IDX] = (RADIO_DIR_UP << RADIO_DIR_POS);//方向上行
		
		packet[TAG_SER_IDX] |= ((DEV_SER.State_Ser_Num << TAG_STATESER_Pos)&TAG_STATESER_Msk);
		packet[TAG_SER_IDX] |= ((DEV_SER.Send_Ser_Num << TAG_SENDSER_Pos)&TAG_SENDSER_Msk);
		my_memcpy(packet+TAG_ID_IDX,DeviceID,RADIO_TID_LENGTH);//ID
		packet[TAG_STATE_IDX] = 0;
		packet[TAG_VERSION_IDX] |= TAG_SFVER_NUM << TAG_SFVERSION_POS;//软件版本号
		cmd_packet.length = TAG_VERSION_IDX-1;//PAYLOAD长度 
		packet[RADIO_LENGTH_IDX] = cmd_packet.length ;
		packet[PYLOAD_XOR_IDX] = Get_Xor(packet+TAG_SER_IDX,cmd_packet.length-1);//PAYLOAD-XOR	
		DEV_SER.Send_Ser_Num++;//发射流水+1
		if(DEV_SER.Send_Ser_Num > TAG_SENDSER_MAX_VALUE)
		{
			DEV_SER.Send_Ser_Num = 0;
		}
	}
	radio_select(Channel,RADIO_TX);
	//是否等待发送完成
	if(1 == wait_send_finish)
	{
		while(NRF_RADIO->STATE != RADIO_STATE_STATE_TxIdle)
		{
			ot++;
			if(ot > RADIO_OVER_TIME)
				break;			
		}
		if(NRF_RADIO->EVENTS_END)
			NRF_RADIO->EVENTS_END = 0;
	}

}

/************************************************* 
Description:公共函数-射频功率选择
Input:输入发射功率
Output:无
Return:无
*************************************************/  
void radio_pwr(uint8_t txpower)
{
	switch(txpower)
	{
		case P_PWR_N30DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg30dBm << RADIO_TXPOWER_TXPOWER_Pos);
			break;
		case P_PWR_N20DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg20dBm << RADIO_TXPOWER_TXPOWER_Pos);
			break;
		case P_PWR_N16DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg16dBm << RADIO_TXPOWER_TXPOWER_Pos);
			break;
		case P_PWR_N12DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg12dBm << RADIO_TXPOWER_TXPOWER_Pos);
			break;
		case P_PWR_N8DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg8dBm << RADIO_TXPOWER_TXPOWER_Pos);
			break;
		case P_PWR_N4DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg4dBm << RADIO_TXPOWER_TXPOWER_Pos);
			break;
		case P_PWR_P0DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);
			break;
		case P_PWR_P4DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos);
			break;
		default:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos);
			break;
	}
}

/*
@Description:公共函数-异或检查
@Input:src:原数组，长度
@Output:
@Return:无
*/
uint8_t Xor_Check(uint8_t *src,uint8_t size)
{
	uint8_t sum=0;
	uint8_t i;
	
	for(i = 0;i<size;i++)
	{
		sum ^= src[i];
	}
	if(sum == 0)
		return TRUE;
	else
		return FALSE;
}

/*
@Description:读写器-ID对比
@Input:src:
@Output:
@Return:无
*/
uint8_t ID_CMP(const uint8_t *src,const uint8_t *dest)
{
	if(src[0]!=dest[0])
		return 1;
	if(src[1]!=dest[1])
		return 1;
	if(src[2]!=dest[2])
		return 1;
	if(src[3]!=dest[3])
		return 1;
	if(src[4]!=dest[4])
		return 1;
	return 0;
}

/*
@Description:读写器-标签记录
@Input:src:
@Output:
@Return:无
*/
void reader_record(uint8_t *t_packet)
{
	uint16_t i,j;
	for(i=0,j=CAPACITY;i<CAPACITY;i++)
	{
		if(TID_RECORD[i].TID[3]==READER_ID_MBYTE)
		{
			if(j > i) j=i;//记录下最后一个出现0xff的索引号
		}
		else if(0 == ID_CMP(&t_packet[TAG_ID_IDX],&TID_RECORD[i].TID[0]))
		{
			j=i;
			break;
		}
	}
	if(j < CAPACITY)//允许记录
	{
		if(i == CAPACITY)//新纪录
		{
			TID_RECORD[j].TID[0] = t_packet[TAG_ID_IDX];
			TID_RECORD[j].TID[1] = t_packet[TAG_ID_IDX + 1];
			TID_RECORD[j].TID[2] = t_packet[TAG_ID_IDX + 2];
			TID_RECORD[j].TID[3] = t_packet[TAG_ID_IDX + 3];
			TID_RECORD[j].State =  t_packet[TAG_STATE_IDX];
//			TID_RECORD[j].Sensor_Type = t_packet[TAG_TYPE_IDX];
//			memcpy(TID_RECORD[j].Sensor_Data,t_packet[TAG_RSSIBASE_IDX],TAG_TYPE_SchoolWatch_LEN-1);
			TID_RECORD[j].PosVer = t_packet[TAG_VERSION_IDX];
			TID_RECORD[j].RSSI = NRF_RADIO->RSSISAMPLE;
			
		}
		else//更新数据
		{
			TID_RECORD[j].State =  t_packet[TAG_STATE_IDX];
//			TID_RECORD[j].Sensor_Type = t_packet[TAG_TYPE_IDX];
//			memcpy(TID_RECORD[j].Sensor_Data,t_packet[TAG_RSSIBASE_IDX],TAG_TYPE_SchoolWatch_LEN-1);	
			TID_RECORD[j].RSSI = NRF_RADIO->RSSISAMPLE;
			TID_RECORD[j].PosVer = t_packet[TAG_VERSION_IDX];
		}
	}
}

/*
Description:标签记录
Input:src:
Output:
Return:无
*/
uint16_t error;
void tag_record(uint8_t *t_packet)
{
	uint16_t i,j;
	uint8_t data_len;

	for(i=0,j=CAPACITY;i<CAPACITY;i++)
	{
		if(TID_RECORD[i].TID[0]==READER_ID_MBYTE)
		{
			if(j > i) j=i;//记录下第一个出现0xff的索引号
		}
		else if(0 == ID_CMP(&t_packet[TAG_ID_IDX],&TID_RECORD[i].TID[0]))
		{
			j=i;
			break;
		}
	}
	if(j < CAPACITY)//允许记录
	{
		if(i == CAPACITY)//新纪录
		{
			memcpy(TID_RECORD[j].TID,&t_packet[TAG_ID_IDX],RADIO_TID_LENGTH);
			TID_RECORD[j].State =  t_packet[TAG_STATE_IDX];
			TID_RECORD[j].RSSI = NRF_RADIO->RSSISAMPLE;
			TID_RECORD[j].StateSeq = t_packet[TAG_SER_IDX];
			TID_RECORD[j].PosVer = TAG_IN_Msk|t_packet[TAG_VERSION_IDX];
			data_len = t_packet[RADIO_LENGTH_IDX]-TAG_REPORTFIX_LENGTH;//最短9字节
			if(0 == data_len)
			{
				TID_RECORD[j].Sensor_Type = 0x10;
			}
			else if(data_len <= INFO_TYPE_LEN)	
			{
				memcpy(TID_RECORD[j].Sensor_Data,&t_packet[TAG_RSSIBASE_IDX],(t_packet[RADIO_LENGTH_IDX]-TAG_REPORTFIX_LENGTH));
				TID_RECORD[j].Sensor_Type = t_packet[TAG_TYPE_IDX]|0x10;
			}
			TID_RECORD[j].LeaveTime = 0;
		}
		else//更新数据
		{
			TID_RECORD[j].State =  t_packet[TAG_STATE_IDX];
			TID_RECORD[j].RSSI = NRF_RADIO->RSSISAMPLE;
			TID_RECORD[j].StateSeq = t_packet[TAG_SER_IDX];
			TID_RECORD[j].PosVer |= t_packet[TAG_VERSION_IDX];
			data_len = t_packet[RADIO_LENGTH_IDX]-TAG_REPORTFIX_LENGTH;//最短9字节
			if(0 == data_len)
			{
				TID_RECORD[j].Sensor_Type = 0x10;
			}
			else if(data_len <= INFO_TYPE_LEN)	
			{
				error++;
				memcpy(TID_RECORD[j].Sensor_Data,&t_packet[TAG_RSSIBASE_IDX],(t_packet[RADIO_LENGTH_IDX]-TAG_REPORTFIX_LENGTH));
				TID_RECORD[j].Sensor_Type = t_packet[TAG_TYPE_IDX]|0x10;
			}
//			else
//			{
//				error++;
//			}
			TID_RECORD[j].LeaveTime = 0;
		}
	}		

}

/*
@Description:读写器-自身参数设置
@Input:src:
@Output:
@Return:无
*/
void Reader_RadioCmdDeal(void)
{
	uint8_t cmd;
	uint16_t cmd_state;
	cmd_packet.length = 0;
	if(0 == ID_CMP(DeviceID,packet + TAG_ID_IDX))//点对点
	{
		my_memcpy(cmd_packet.packet,packet,packet[RADIO_LENGTH_IDX]+RADIO_HEAD_LENGTH);
		cmd = (cmd_packet.packet[CMD_IDX]&CMD_Msk);
		switch(cmd)
		{
			case FILE_READ_CMD://文件读取-点对点
			{
				f_para.mode = cmd_packet.packet[FILE_MODE_IDX];
				f_para.offset = cmd_packet.packet[FILE_OFFSET_IDX];
				f_para.length = cmd_packet.packet[FILE_LENGTH_IDX];
				cmd_state = Read_Para(f_para,&cmd_packet.packet[CMD_PARA_IDX],&cmd_packet.packet[EXCUTE_STATE_IDX+2]);//读文件
				if(CMD_RUN_SUCCESS == cmd_state)
				{
					cmd_packet.length = CMD_ACK_FIX_LENGTH + f_para.length;
				}
				else//只返回执行状态
				{
					cmd_packet.length = CMD_ACK_FIX_LENGTH;
				}
				cmd_packet.packet[CMD_IDX] = (RADIO_DIR_UP<<RADIO_DIR_POS)&RADIO_DIR_Msk;//上行
				cmd_packet.packet[CMD_IDX] |= FILE_READ_CMD;//命令
				cmd_packet.packet[EXCUTE_STATE_IDX] = cmd_state >>8;
				cmd_packet.packet[EXCUTE_STATE_IDX+1] = cmd_state;						
				cmd_packet.packet[RADIO_LENGTH_IDX] = cmd_packet.length;
				cmd_packet.packet[PYLOAD_XOR_IDX] = Get_Xor(cmd_packet.packet+CMD_IDX,cmd_packet.length-1);
				Radio_Period_Send(WithCmd,CONFIG_CHANNEL,WithoutWin,SendWait);
			}
			break;
			case FILE_WRITE_CMD://文件写入-点对点
			{
				f_para.mode = cmd_packet.packet[FILE_MODE_IDX];
				f_para.offset = cmd_packet.packet[FILE_OFFSET_IDX];
				f_para.length = cmd_packet.packet[FILE_LENGTH_IDX];
				cmd_state = Write_Para(f_para,&cmd_packet.packet[CMD_PARA_IDX]);
				
				cmd_packet.packet[CMD_IDX] = (RADIO_DIR_UP<<RADIO_DIR_POS)&RADIO_DIR_Msk;//上行
				cmd_packet.packet[CMD_IDX] |= FILE_WRITE_CMD;//命令
				cmd_packet.length = CMD_ACK_FIX_LENGTH;
				cmd_packet.packet[EXCUTE_STATE_IDX] = cmd_state >>8;
				cmd_packet.packet[EXCUTE_STATE_IDX+1] = cmd_state;
				cmd_packet.packet[RADIO_LENGTH_IDX] = cmd_packet.length;
				cmd_packet.packet[PYLOAD_XOR_IDX]=Get_Xor(cmd_packet.packet+CMD_IDX,cmd_packet.length-1);
				Radio_Period_Send(WithCmd,CONFIG_CHANNEL,WithoutWin,SendWait);		
			}	
			break;
			case FILE_ERASE_CMD://文件擦除
			{
				f_para.mode = cmd_packet.packet[FILE_MODE_IDX];
				cmd_state = Erase_Para(f_para);//擦除
				
				cmd_packet.packet[CMD_IDX] = (RADIO_DIR_UP<<RADIO_DIR_POS)&RADIO_DIR_Msk;//上行
				cmd_packet.packet[CMD_IDX] |= FILE_ERASE_CMD;//命令
				cmd_packet.packet[EXCUTE_STATE_IDX] = cmd_state >>8;
				cmd_packet.packet[EXCUTE_STATE_IDX+1] = cmd_state;
				cmd_packet.length = CMD_ACK_FIX_LENGTH;
				cmd_packet.packet[RADIO_LENGTH_IDX] = cmd_packet.length;
				cmd_packet.packet[PYLOAD_XOR_IDX]=Get_Xor(cmd_packet.packet+CMD_IDX,cmd_packet.length-1);
				Radio_Period_Send(WithCmd,CONFIG_CHANNEL,WithoutWin,SendWait);							
			}
			break;
			default:break;	
		}							
	}
	else if(READER_ID_MBYTE == packet[TAG_ID_IDX])//广播
	{
		
	}
	radio_select(DATA_CHANNEL,RADIO_RX);
}

/************************************************* 
@Description:读写器-射频时间设置
@Input:无
@Output:无
@Return:无
*************************************************/ 
void Radio_Time_Set(void)
{
	uint8_t len;
	cmd_packet.packet[CMD_IDX] = (RADIO_DIR_DOWN<<RADIO_DIR_POS)&RADIO_DIR_Msk;//下行
	cmd_packet.packet[CMD_IDX] |= MSG_PUSH_CMD;//命令
	my_memcpy(&cmd_packet.packet[TAG_ID_IDX],&packet[TAG_ID_IDX],RADIO_TID_LENGTH);//目标ID
	my_memcpy(&cmd_packet.packet[READER_ID_IDX],&DeviceID,RADIO_RID_LENGTH);//读写器ID
	cmd_packet.packet[CMD_PARA_IDX] = (msg_calendar<<READER_MSG_TYPE_Pos)&READER_MSG_TYPE_Msk;//消息分类
	cmd_packet.packet[CMD_PARA_IDX] |= (MSG_Store.R_MSG2_Seq<<READER_MSG_SEQ_Pos)&READER_MSG_SEQ_Msk;//消息序号
	cmd_packet.packet[CMD_PARA_IDX+1] = 0x00;//最大编号、当前分包编号
	TIME_BCDToDec(&cmd_packet.packet[CMD_PARA_IDX+2]);//命令参数
	len = CMD_ONEFIX_LENGTH + MSG_HEAD_LEN + TIME_PARA_LEN;
	cmd_packet.packet[RADIO_LENGTH_IDX] = len;
	
	cmd_packet.packet[PYLOAD_XOR_IDX]=Get_Xor(cmd_packet.packet+CMD_IDX,len-1);
	cmd_packet.length = cmd_packet.packet[RADIO_LENGTH_IDX];//射频数据长度
}

/************************************************* 
@Description:读写器-报警清除
@Input:无
@Output:无
@Return:无
*************************************************/ 
void Alarm_Clear(void)
{
	if( ((packet[TAG_STATE_IDX]&TAG_KEY_Msk)>>TAG_KEY_Pos) == 1)
	{
		uint8_t len;
		cmd_packet.packet[CMD_IDX] = (RADIO_DIR_DOWN<<RADIO_DIR_POS)&RADIO_DIR_Msk;//下行
		cmd_packet.packet[CMD_IDX] |= ALARM_CLEAR_CMD;
		my_memcpy(&cmd_packet.packet[TAG_ID_IDX],&packet[TAG_ID_IDX],RADIO_TID_LENGTH);//目标ID
		my_memcpy(&cmd_packet.packet[READER_ID_IDX],&DeviceID,RADIO_RID_LENGTH);//读写器ID
		cmd_packet.packet[CMD_PARA_IDX] = 0X00;
		len = CMD_ONEFIX_LENGTH + 1;
		cmd_packet.packet[RADIO_LENGTH_IDX] = len;
		cmd_packet.packet[PYLOAD_XOR_IDX]=Get_Xor(cmd_packet.packet+CMD_IDX,len-1);
		cmd_packet.length = cmd_packet.packet[RADIO_LENGTH_IDX];//射频数据长度
		Radio_Period_Send(WithCmd,WithWin,CONFIG_CHANNEL,SendNoWait);//配置频道下发命令
	}

}


/************************************************* 
@Description:RADIO中断处理程序
@Input:无
@Output:无
@Return:无
*************************************************/
void Radio_RX_Deal(void)
{
//	u16 state;
	uint8_t ValidTag = 0;
	uint8_t m;
	if(TRUE == Xor_Check(&packet[PYLOAD_XOR_IDX],packet[RADIO_LENGTH_IDX]))
	{
		if( RADIO_RUN_DATA_CHANNEL == Radio_State.radio_run_channel )//数据频道采集标签信息
		{
			//射频设置自身参数
			if(((packet[TAG_SER_IDX]&RADIO_DIR_Msk)>>RADIO_DIR_POS) == RADIO_DIR_DOWN) //下行
			{
				Radio_State.radio_rcvok = 1;
				Reader_RadioCmdDeal();
			}
			else
			{
				if(Idle ==  Work_Mode)
				{
					//查找局部过滤设备类型
					for(m=0;m<RSSIFilter.LocalRssiNum;m++)
					{
						if(RSSIFilter.LocalRssi[m][0] == packet[TAG_ID_IDX+4])
						{
							if(NRF_RADIO->RSSISAMPLE <= RSSIFilter.LocalRssi[m][0])
								ValidTag = 1;
							else
								ValidTag = 0;
							break;
						}
					}
					if(m == RSSIFilter.LocalRssiNum)//未找到对应类型的标签，则采用全局过滤
					{
						if(NRF_RADIO->RSSISAMPLE <= RSSIFilter.GobalRssi)
							ValidTag = 1;
					}
					if(ValidTag)//有效的标签
					{
						tag_record(packet);//标签收集
					}
					if ( ((packet[TAG_STATE_IDX]&TAG_WIHTWIN_Msk)>>TAG_WITHWIN_Pos) == WithWin )//携带接收窗口
					{
						//消息处理
						debug_printf("\n\r携带接收窗口");
						if(TRUE == Reader_Msg1_Get(packet[TAG_MSG_IDX],&packet[TAG_ID_IDX]))//并且有新信息发送
						{
							memcpy(Msg_Packet.MSG_PUSH_TID,packet+TAG_ID_IDX,RADIO_RID_LENGTH);//获取目标ID
							Radio_MSG_Push(Msg_Packet.MSG_PUSH_TID);//获取消息
							Radio_Period_Send(WithCmd,CONFIG_CHANNEL,WithWin,SendNoWait);//配置频道下发命令
							Work_Mode = Msg_Deal;//消息处理
//							debug_printf("\n\r下发消息通知命令");
						}
						//时间设置
						else if( TRUE == Reader_Msg2_Get(packet[TAG_MSG_IDX]))
						{
							/********************************************************************
							时间设置机制：平台每天下发一次时间设置，当消息编号不同时，更新时间
							*******************************************************************/
							Radio_Time_Set();
							Radio_Period_Send(WithCmd,CONFIG_CHANNEL,WithWin,SendNoWait);//配置频道下发命令
							Work_Mode = Time_Set;
						}
//						else if( ((packet[TAG_STATE_IDX]&TAG_KEY_Msk)>>TAG_KEY_Pos) == 1)
//						{
//							Alarm_Clear(cmd_packet.packet,packet);
//							cmd_packet.length = cmd_packet.packet[RADIO_LENGTH_IDX];//射频数据长度
//							Radio_Period_Send(WithCmd,WithWin,SendNoWait);//配置频道下发命令
//							Work_Mode = Alarm_Clr;
//						}
					}
				}
				else if( File_Deal == Work_Mode)//标签文件处理
				{
					if(0 == ID_CMP(&packet[TAG_ID_IDX],&cmd_packet.packet[TAG_ID_IDX]))//目标ID
					{
						if( ((packet[TAG_STATE_IDX]&TAG_WIHTWIN_Msk)>>TAG_WITHWIN_Pos) == WithWin )//目标携带接收窗
						{
							Radio_Period_Send(WithCmd,CONFIG_CHANNEL,WithWin,SendNoWait);//配置频道下发命令
						}
					}	
				}
				#if 0
				else if( Device_Test == Work_Mode)//整机测试
				{
					if(0 == ID_CMP(&packet[TAG_ID_IDX],&cmd_packet.packet[TAG_ID_IDX]))//目标ID
					{
						if( ((packet[TAG_STATE_IDX]&TAG_WIHTWIN_Msk)>>TAG_WITHWIN_Pos) == WithWin )//目标携带接收窗
						{
							Radio_Period_Send(WithCmd,CONFIG_CHANNEL,WithWin,SendWait);//配置频道下发命令
							US_Master.tx_en = 1;
						}
					}	
				}
				else if(Search_Ack == Work_Mode)//查询回复
				{
					switch(Filter_Radio.TAG_TYPE)
					{
						case TagReader:
						{
							if(NRF_RADIO->RSSISAMPLE < Filter_Radio.RSSI_Filter_Value )
							{
								if(Filter_Radio.LP_Filter_En)
								{
									if(packet[TAG_STATE_IDX]&TAG_LOWPWR_Msk)//低电过滤
									{
										reader_record(packet);//读写器收集	
									}
								}
								else
								{
									reader_record(packet);//读写器收集
								}
							}						
						}
						break;
						case Tag:
						{
							if(packet[TAG_ID_IDX]!=READER_ID_MBYTE)//标签信息收集
							{
								if(NRF_RADIO->RSSISAMPLE < Filter_Radio.RSSI_Filter_Value )
								{
									if(Filter_Radio.LP_Filter_En)
									{
										if(packet[TAG_STATE_IDX]&TAG_LOWPWR_Msk)//低电过滤
										{
											tag_record(packet);//标签收集
										}
									}
									else
									{
										tag_record(packet);//标签收集
									}
								}
							}								
						}
						break;
						case Reader:
						{
							if(packet[TAG_ID_IDX]==READER_ID_MBYTE)//读写器信息收集
							{
								if(NRF_RADIO->RSSISAMPLE < Filter_Radio.RSSI_Filter_Value )
								{
									if(Filter_Radio.LP_Filter_En)
									{
										if(packet[TAG_STATE_IDX]&TAG_LOWPWR_Msk)//低电过滤
										{
											reader_record(packet);//读写器收集	
										}
									}
									else
									{
										reader_record(packet);//读写器收集
									}
								}
							}							
						}
						break;
					}	
				}
				else if(Auto_Reoprt == Work_Mode)//自动上报
				{
					switch(Filter_Radio.TAG_TYPE)
					{
						case TagReader:
						{
							if(NRF_RADIO->RSSISAMPLE < Filter_Radio.RSSI_Filter_Value )
							{
								if(Filter_Radio.LP_Filter_En)
								{
									if(packet[TAG_STATE_IDX]&TAG_LOWPWR_Msk)//低电过滤
									{
										reader_record(packet);//读写器收集	
									}
								}
								else
								{
									reader_record(packet);//读写器收集
								}
							}						
						}
						break;
						case Tag:
						{
							if(packet[TAG_ID_IDX]!=READER_ID_MBYTE)//标签信息收集
							{
								if(NRF_RADIO->RSSISAMPLE < Filter_Radio.RSSI_Filter_Value )
								{
									if(Filter_Radio.LP_Filter_En)
									{
										if(packet[TAG_STATE_IDX]&TAG_LOWPWR_Msk)//低电过滤
										{
											tag_record(packet);//标签收集
											Alarm_Clear();
										}
									}
									else
									{
										tag_record(packet);//标签收集
										Alarm_Clear();
									}
								}
							}								
						}
						break;
						case Reader:
						{
							if(packet[TAG_ID_IDX]==READER_ID_MBYTE)//读写器信息收集
							{
								if(NRF_RADIO->RSSISAMPLE < Filter_Radio.RSSI_Filter_Value )
								{
									if(Filter_Radio.LP_Filter_En)
									{
										if(packet[TAG_STATE_IDX]&TAG_LOWPWR_Msk)//低电过滤
										{
											reader_record(packet);//读写器收集	
										}
									}
									else
									{
										reader_record(packet);//读写器收集
									}
								}
							}							
						}
						break;
					}	
				}	
				#endif
			}

		}
		else if( RADIO_RUN_CONFIG_CHANNEL == Radio_State.radio_run_channel)
		{
			//射频设置自身参数
			if(((packet[TAG_SER_IDX]&RADIO_DIR_Msk)>>RADIO_DIR_POS) == RADIO_DIR_DOWN) //下行
			{
				Radio_State.radio_rcvok = 1;
			}
			else
			{
				//射频设置其他设备
				if( File_Deal == Work_Mode||Reader_File_Deal == Work_Mode)//文件处理
				{
					if(0 == ID_CMP(&packet[TAG_ID_IDX],&cmd_packet.packet[TAG_ID_IDX]))//目标ID
					{
						//命令接收完成，串口回复
						if(packet[CMD_IDX] == FILE_READ_CMD || packet[CMD_IDX] == FILE_WRITE_CMD || packet[CMD_IDX] == FILE_ERASE_CMD)
						{
							my_memcpy(cmd_packet.packet,packet,packet[RADIO_LENGTH_IDX]+RADIO_HEAD_LENGTH);
							US_Master.tx_en = 1;
						}
					}
				}					
			}
												
		}
	}
	else//中科微
	{
		if( RADIO_RUN_DATA_CHANNEL == Radio_State.radio_run_channel )//数据频道采集标签信息
		{
			if(packet[TAG_ID_IDX+4] > 0x0f && packet[TAG_ID_IDX+4] < 0x20)//中科微类型字
			{
				if(Idle ==  Work_Mode)
				{
					//查找局部过滤设备类型
					for(m=0;m<RSSIFilter.LocalRssiNum;m++)
					{
						if(RSSIFilter.LocalRssi[m][0] == packet[TAG_ID_IDX+4])
						{
							if(NRF_RADIO->RSSISAMPLE <= RSSIFilter.LocalRssi[m][0])
								ValidTag = 1;
							else
								ValidTag = 0;
							break;
						}
					}
					if(m == RSSIFilter.LocalRssiNum)//未找到对应类型的标签，则采用全局过滤
					{
						if(NRF_RADIO->RSSISAMPLE <= RSSIFilter.GobalRssi)
							ValidTag = 1;
					}
					if(ValidTag)//有效的标签
					{					
						tag_record(packet);//标签收集	
					}						
				}
				#if 0
				else if(Search_Ack == Work_Mode || Auto_Reoprt == Work_Mode)
				{
					if(packet[TAG_ID_IDX]!=READER_ID_MBYTE)//标签信息收集
					{
						if(NRF_RADIO->RSSISAMPLE < Filter_Radio.RSSI_Filter_Value )
						{
							if(Filter_Radio.LP_Filter_En)
							{
								if(packet[TAG_STATE_IDX]&TAG_LOWPWR_Msk)//低电过滤
								{
									tag_record(packet);//标签收集
								}
							}
							else
							{
								tag_record(packet);//标签收集
							}
						}
					}					
				}	
				#endif
			}				
		}

	}
}
/************************************************* 
@Description:RADIO中断处理程序-发送
@Input:无
@Output:无
@Return:无
*************************************************/
void Radio_TX_Deal(void)
{
	if(RADIO_RUN_CONFIG_CHANNEL == Radio_State.radio_run_channel)//配置频道
	{
		if(File_Deal == Work_Mode)//文件发送处理
		{
//			Radio_Period_Send(WithCmd,WithWin,SendNoWait);//配置频道下发命令
			radio_select(CONFIG_CHANNEL,RADIO_RX);//切换到配置频道接收,等待回复
		}
		else if(Msg_Deal == Work_Mode)
		{
			//超过重发次数
			if(Msg_Packet.MSG_REPUSH_NUM > MSG1_RPUSH_TIMES)
			{
				Work_Mode = Idle;
				MSG_PUSH_State = MSG_IDLE;
			}
			else
			{
				MSG_NEXT_PKT();
				Radio_MSG_Push(Msg_Packet.MSG_PUSH_TID);
				nrf_delay_us(100);
				Radio_Period_Send(WithCmd,CONFIG_CHANNEL,WithWin,SendNoWait);//配置频道下发命令
				
			}
		}
		else if(Time_Set ==  Work_Mode)
		{
			radio_select(DATA_CHANNEL,RADIO_RX);//切换到数据频道接收
			Work_Mode = Idle;
		}
		#if 0
		else if(Auto_Reoprt == Work_Mode)
		{
			radio_select(DATA_CHANNEL,RADIO_RX);//切换到数据频道接收
		}
		#endif
	}
	else if(RADIO_RUN_DATA_CHANNEL == Radio_State.radio_run_channel)
	{
		if(Work_Mode == Reader_File_Deal)//文件发送处理
		{
			radio_select(CONFIG_CHANNEL,RADIO_RX);//切换到配置频道接收
		}		
	}
}
/************************************************* 
@Description:RADIO中断处理程序
@Input:无
@Output:无
@Return:无
*************************************************/  
uint8_t rx_cnt;
void RADIO_IRQHandler(void)
{
	if(NRF_RADIO->EVENTS_END)//EVENTS_END
	{
		NRF_RADIO->EVENTS_END = 0;
		if(NRF_RADIO->STATE == RADIO_STATE_STATE_RxIdle)//射频接收
		{	
			if(NRF_RADIO->CRCSTATUS == RADIO_CRCSTATUS_CRCSTATUS_CRCOk)//CRC校验
			{
				Radio_RX_Deal();//射频处理
				rx_cnt++;
			}
			
		}
		else if(NRF_RADIO->STATE == RADIO_STATE_STATE_TxIdle)//射频发送
		{
			Radio_TX_Deal();
			Radio_State.radio_sndok = 1;//射频发送成功
		}
		if(RADIO_STATUS_RX == radio_status)
		{
			NRF_RADIO->TASKS_START = 1U;//重新启动射频
		}
	}
	
}
