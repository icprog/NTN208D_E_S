/*******************************************************************************
** 版权:		
** 文件名: 		app_msg.c
** 版本：  		1.0
** 工作环境: 	MDK-ARM 5.23
** 作者: 		cc
** 生成日期: 	2017-07-22
** 功能:		  
** 相关文件:	app_msg.h
** 修改日志：	
** 版权所有   
*******************************************************************************/

#include "App_Msg.h"
#define App_Sys
#define App_Init
#define App_NVMC
#define App_Var
#define App_Uart
#define App_Radio
#include "App_Libraries.h"
#include "string.h"

extern uint8_t DeviceID[4];
MSG_Store_Typedef MSG_Store;
extern ROM_BaseAddr_Typedef   ROM_BaseAddr;//ROM基地址定义
//消息开始命令调用
extern void Radio_Period_Send(uint8_t cmdflag,uint8_t Channel,uint8_t winflag,uint8_t wait_send_finish);
//消息数量
//消息处理
Message_Typedef Msg_Packet;
typedef enum
{
	pkt_seq0=0,
	pkt_seq1=1,
	pkt_seq2=2,
	pkt_seq3=3
}pkt_seq;

/************************************************* 
@Description:公共函数-消息初始化
@Input:无
@Output:无
@Return:无
*************************************************/ 
void MSG_Addr_Init(void)
{
	MSG_Store.MSG1_ROM = ROM_BaseAddr.page_size * (ROM_BaseAddr.page_num-5);
	MSG_Store.MSG2_ROM = ROM_BaseAddr.page_size * (ROM_BaseAddr.page_num-6);
	MSG_Store.MSG3_ROM = ROM_BaseAddr.page_size * (ROM_BaseAddr.page_num-7);
	MSG_Store.NEW_MSG_ROM = ROM_BaseAddr.page_size * (ROM_BaseAddr.page_num-8);
	MSG_Store.MSG_ADDR[0] = &MSG_Store.MSG1_ROM;
	MSG_Store.MSG_ADDR[1] = &MSG_Store.MSG2_ROM;
	MSG_Store.MSG_ADDR[2] = &MSG_Store.MSG3_ROM;
}


/************************************************* 
@Description:公共函数-擦除所有消息
@Input:无
@Output:无
@Return:无
*************************************************/ 
void MSG_Erase_ALL(void)
{
	uint32_t msg_addr;
	//消息1
	msg_addr = MSG_Store.MSG1_ROM;
	nrf_nvmc_page_erase(msg_addr);
	//消息2
	msg_addr = MSG_Store.MSG2_ROM;
	nrf_nvmc_page_erase(msg_addr);
	//消息3
	msg_addr = MSG_Store.MSG3_ROM;
	nrf_nvmc_page_erase(msg_addr);
	//擦除最新消息记录
	msg_addr  = MSG_Store.NEW_MSG_ROM;
	nrf_nvmc_page_erase(msg_addr);
	
	memset(&MSG_Store,0,sizeof(MSG_Store_Typedef));
	MSG_Addr_Init();
	
}				
/************************************************* 
@Description:公共函数-检查消息序列号
@Input:无
@Output:无
@Return:无
*************************************************/ 
uint8_t MSG_ROM_Check(uint32_t temp_addr)
{
	if(*(uint8_t*)temp_addr <= MSG1_SEQ_Max_Value)  //检查消息序号
		return TRUE;
	return FALSE;
}
/************************************************* 
@Description:公共函数-查找最新消息,消息数量、最新消息存储区索引号、获取消息序列号
如果flash中消息数量和消息索引号不符合要求，全部擦除。
@Input:无
@Output:无
@Return:无
*************************************************/ 
void MSG_Find_New(void)
{
	uint8_t msg_idx;
	//获取存储区消息数量
	MSG_Store.MSG_Num += MSG_ROM_Check(MSG_Store.MSG1_ROM);
	MSG_Store.MSG_Num += MSG_ROM_Check(MSG_Store.MSG2_ROM);
	MSG_Store.MSG_Num += MSG_ROM_Check(MSG_Store.MSG3_ROM);
	if(0 == MSG_Store.MSG_Num)
	{
		MSG_Erase_ALL();
		return;
	}
	//获取最新消息索引号0~2,对应FLASH区
	msg_idx = *(uint8_t*)MSG_Store.NEW_MSG_ROM;
	if(msg_idx <= MSG1_IDX_MAX_Value)
	{
		MSG_Store.MSG1_IDX = (MSG_IDX_Typedef)msg_idx;
	}
	else
	{
		MSG_Erase_ALL();
		return;
	}
	//获取序列号
	MSG_Store.T_MSG1_Seq = *(uint8_t*)(*MSG_Store.MSG_ADDR[MSG_Store.MSG1_IDX]);
	MSG_Store.R_MSG1_Seq = *(uint8_t*)(*MSG_Store.MSG_ADDR[MSG_Store.MSG1_IDX]);

	if(MSG_Store.T_MSG1_Seq > MSG1_SEQ_Max_Value)
	{
		MSG_Erase_ALL();
		return;
	}
}


/************************************************* 
@Description:公共函数-消息包写入FLASH,替换最旧的消息
@Input:idx 最新消息索引号，一包消息
@Output:无
@Return:无
*************************************************/ 
void MSG_Write(uint8_t idx,u8* buff)
{
	uint32_t msg_addr;
	uint8_t m_idx;
	m_idx = (idx + MSG1_FLASH_NUM - MSG1_IDX_MAX_Value)%MSG1_FLASH_NUM;//索引号减去2
	msg_addr = *MSG_Store.MSG_ADDR[m_idx];
	nrf_nvmc_page_erase(msg_addr);
	nrf_nvmc_write_bytes(msg_addr,buff,(buff[MSG1_LEN_IDX]+MSG1_FLASH_HEAD_LEN));
	MSG_Store.MSG1_IDX = (MSG_Store.MSG1_IDX + 1)%MSG1_FLASH_NUM;//索引号+1
	if(MSG_Store.MSG_Num < MSG1_FLASH_NUM)
		MSG_Store.MSG_Num++;
	msg_addr = MSG_Store.NEW_MSG_ROM;
	nrf_nvmc_page_erase(msg_addr);
	nrf_nvmc_write_byte(msg_addr,MSG_Store.MSG1_IDX);//写入最新索引号
}

/************************************************* 
@Description:标签-消息包置位
@Input:无
@Output:无
@Return:无
*************************************************/ 
void MSG_Packet_ReSet(void)
{
//	MSG_Store.T_MSG1_BUFF_IDX = MSG1_STORE_IDX;//一条消息存储索引号置位
//	Msg_Packet.msg_pkt_seq = 0;//包序号置位
	Msg_Packet.T_MSG1_ONE_LEN = 0;//清空消息内容长度
	memset(Msg_Packet.PKT_CHK,pkt_unwrite,Msg_Packet.PKT_MAX_NUM+1);//清空分包指示
}



/************************************************* 
@Description:标签-消息接收完成检查
@Input:无
@Output:无
@Return:无
*************************************************/ 
u8 Msg_RFinish_Check(uint8_t *pCheck)
{
	uint8_t i;
	for(i=0;i<=Msg_Packet.PKT_MAX_NUM;)
	{
		if(Msg_Packet.PKT_CHK[i] != pkt_write)
			break;
		i++;
	}
	if(i > Msg_Packet.PKT_MAX_NUM)
		return TRUE;
	else
		return FALSE;
}


/************************************************* 
@Description:标签-消息处理
@Input:p_mpacket射频数据
@Output:无
@Return:无
*************************************************/ 
u16 Message_Deal(uint8_t *p_mpacket)
{
//	u16 cmd_state;
	uint8_t msg1_pkt_len;	//下发的一个包中的信息长度
	MSG_Store.R_MSG1_Seq = ((p_mpacket[CMD_PARA_IDX] & READER_MSG_SEQ_Msk)>>READER_MSG_SEQ_Pos);//获取下发的消息序号
	Msg_Packet.PKT_CUR_NUM = ((p_mpacket[CMD_PARA_IDX+1]&PKT_CUR_NUM_Msk)>>PKT_CUR_NUM_Pos);
	Msg_Packet.PKT_MAX_NUM = ((p_mpacket[CMD_PARA_IDX+1]&PKT_MAX_NUM_Msk)>>PKT_MAX_NUM_Pos);
	if(MSG_Store.R_MSG1_Seq_Pre!=MSG_Store.R_MSG1_Seq)//当消息序号不同时，清空消息内容长度
	{
		MSG_Packet_ReSet();
		memcpy(Msg_Packet.MSG_PUSH_RID,p_mpacket+READER_ID_IDX,RADIO_RID_LENGTH);//复制接收器ID
		MSG_Store.R_MSG1_Seq_Pre = MSG_Store.R_MSG1_Seq;
	}
	#if 0
	//必须是同一个接收器
	if(memcmp(Msg_Packet.MSG_PUSH_RID,p_mpacket + READER_ID_IDX,RADIO_RID_LENGTH)== 0)//点对点接收器
	{
	#endif
		if(Msg_Packet.PKT_CUR_NUM <= Msg_Packet.PKT_MAX_NUM)
		{
			//消息缓存
			if(Msg_Packet.PKT_CHK[Msg_Packet.PKT_CUR_NUM] == pkt_unwrite)
			{
				msg1_pkt_len = p_mpacket[RADIO_LENGTH_IDX] - CMD_ONEFIX_LENGTH -MSG_HEAD_LEN;//消息内容长度
				MSG_Store.T_MSG1_BUFF_IDX = MSG1_STORE_IDX + (Msg_Packet.PKT_CUR_NUM << MSG1_PACKET_OFFSET);//消息存储起始索引号
				my_memcpy(&MSG_Store.T_MSG1_BUFF[MSG_Store.T_MSG1_BUFF_IDX],&p_mpacket[MSG_DATA_IDX],msg1_pkt_len);//存储消息
				Msg_Packet.PKT_CHK[Msg_Packet.PKT_CUR_NUM] = pkt_write;//该分包写过
				Msg_Packet.T_MSG1_ONE_LEN+=msg1_pkt_len;//1类消息长度
			}
		}
		if(TRUE == Msg_RFinish_Check(Msg_Packet.PKT_CHK))
		{
			//包结束，存入FLASH中
			MSG_Store.R_MSG1_Seq = ((p_mpacket[CMD_PARA_IDX]&READER_MSG_SEQ_Msk) >> READER_MSG_SEQ_Pos);
			MSG_Store.T_MSG1_Seq = MSG_Store.R_MSG1_Seq;//更新标签包序号
			MSG_Store.T_MSG1_BUFF[MSG1_SEQ_IDX] = MSG_Store.T_MSG1_Seq;//存储包序号
			MSG_Store.T_MSG1_BUFF[MSG1_LEN_IDX] = Msg_Packet.T_MSG1_ONE_LEN;//消息长度
			MSG_Write(MSG_Store.MSG1_IDX,MSG_Store.T_MSG1_BUFF);
			MSG_Packet_ReSet();//清空消息内容长度
		}	
	#if 0
	}
	#endif
	return 1;
}

/************************************************* 
@Description:标签-获取消息,用于OLED显示
@Input:
@Output:
@Return:
*************************************************/ 
void Tag_Message_Get(void)
{
	uint32_t addr;//地址
	uint8_t msg_len;//消息长度
	uint8_t i;
	uint8_t msg_idx;//消息索引号
	//获取最新消息索引号
	msg_idx = MSG_Store.MSG1_IDX;
	MSG_Store.Tag_Msg_Num = 0;
	for(i=0;i<MSG_Store.MSG_Num;i++)
	{
		addr = *MSG_Store.MSG_ADDR[msg_idx];//获取地址
		msg_len = *(uint8_t *)(addr+MSG1_LEN_IDX);	
		if(msg_len <= MSG1_MAX_LEN && msg_len > 0)
		{
			nrf_nvmc_read_bytes(addr,&MSG_Store.Tag_Msg_Buf[i][0],(msg_len+MSG1_FLASH_HEAD_LEN));//读取数据
			MSG_Store.Tag_Msg_Num++;
		}
		msg_idx = (msg_idx + MSG1_FLASH_NUM - 1)%MSG1_FLASH_NUM;//索引号减去1
	}
}


/************************************************* 
@Description:读写器-为了减少标签开启接收窗口时间，增加消息开始
@Input:
@Output:
@Return:1:获取成功，0：获取失败
命令参数 指示消息，其它为00
*************************************************/ 
void Reader_Msg1_Start(uint8_t* tid_src)
{
	cmd_packet.packet[CMD_IDX] = (RADIO_DIR_DOWN<<RADIO_DIR_POS)&RADIO_DIR_Msk;//下行
	cmd_packet.packet[CMD_IDX] |= MSG_PUSH_CMD;//消息命令
	my_memcpy(&cmd_packet.packet[TAG_ID_IDX],tid_src,RADIO_TID_LENGTH);//目标ID
	my_memcpy(&cmd_packet.packet[READER_ID_IDX],&DeviceID,RADIO_RID_LENGTH);//读写器ID
	cmd_packet.packet[CMD_PARA_IDX] = (msg_news<<READER_MSG_TYPE_Pos)&READER_MSG_TYPE_Msk;//消息分类
	cmd_packet.packet[CMD_PARA_IDX] |= (0<<READER_MSG_SEQ_Pos)&READER_MSG_SEQ_Msk;//消息序号
	cmd_packet.packet[CMD_PARA_IDX+1] = (0<<PKT_MAX_NUM_Pos)&PKT_MAX_NUM_Msk;//最大编号
	cmd_packet.packet[CMD_PARA_IDX+1] |= (0<<PKT_CUR_NUM_Pos)&PKT_CUR_NUM_Msk;//当前分包编号	
	//点对点固定长度+消息头+消息数据长度
	cmd_packet.length = CMD_ONEFIX_LENGTH + MSG_HEAD_LEN;
	//射频长度
	cmd_packet.packet[RADIO_LENGTH_IDX] = cmd_packet.length ;
	//异或
	cmd_packet.packet[PYLOAD_XOR_IDX] = Get_Xor(cmd_packet.packet+CMD_IDX,cmd_packet.length-1);//PAYLOAD-XOR
	Radio_Period_Send(WithCmd,CONFIG_CHANNEL,WithWin,SendNoWait);//配置频道下发命令
}


/************************************************* 
@Description:读写器-获取要下发的消息1
@Input:msg1_seq消息1编号
@Output:
@Return:1:获取成功，0：获取失败
测试最大包处理时间需要480US = RTC计数值16  16*0.03ms=480us
*************************************************/ 
#define tim_cal 1
uint8_t Reader_Msg1_Get(uint8_t msg1_seq,uint8_t* tid_src)
{
	uint32_t ot = 0;//超时时间
	uint8_t i,j;
	uint8_t msg_len;
	uint8_t m_idx;//消息索引号
	uint32_t t_msg_addr;//消息地址
	uint32_t msg_addr;//消息地址
	uint8_t msg_seq;//存储的消息序号
	uint8_t has_pmsg_flag;//有消息下发
	uint8_t reminder;//余数
	uint8_t t_msg1_seq;//实际要下发的消息序号
	uint8_t tag_msg1_seq;//标签消息序号
	tag_msg1_seq = (msg1_seq&TAG_MSG1_Msk)>>TAG_MSG1_Pos;
	if(tag_msg1_seq!=MSG_Store.R_MSG1_Seq)
	{
		#if tim_cal
		NRF_RTC0->TASKS_START = 1;
		uint32_t a = NRF_RTC0->COUNTER;
		#endif
		t_msg1_seq = tag_msg1_seq + 1;//读写器理论上要下发的消息序号
		if(t_msg1_seq == MSG1_SEQ_MAX_NUM)
			t_msg1_seq = 1;
		//往回查找序号，实际中，消息序号可能跳变
		for(i=0;i<MSG1_FLASH_NUM;i++)//FLASH中查找对应的消息序号
		{
			m_idx = (MSG_Store.MSG1_IDX + MSG1_FLASH_NUM - i)%MSG1_FLASH_NUM;//索引号减去，往回查找有效消息
			t_msg_addr = *MSG_Store.MSG_ADDR[m_idx];//获取消息地址
			msg_seq = *(uint8_t*)t_msg_addr;//获取消息序号
			if(msg_seq==t_msg1_seq)//找到理论应下发的消息
			{
				has_pmsg_flag = 1;
				msg_addr = t_msg_addr;
				break;
			}
			else if(msg_seq < MSG1_SEQ_MAX_NUM)//否则往回找有效的最旧消息
			{
				if(msg_seq == tag_msg1_seq)
					break;
				has_pmsg_flag = 1;
				msg_addr = t_msg_addr;
			}
		}	
		if(has_pmsg_flag)
		{
			
			#if tim_cal
			uint32_t b = NRF_RTC0->COUNTER;
			#endif 
			Reader_Msg1_Start(tid_src);//我这种下发方法数据处理需要时间，所以隐含增加消息开始指令，增加消息接收窗口
			#if tim_cal
			uint32_t c = NRF_RTC0->COUNTER;
			#endif
			Msg_Packet.MSG_PUSH_SEQ = *(uint8_t*)msg_addr;
			msg_len = *(uint8_t *)(msg_addr+MSG1_LEN_IDX);//消息长度
			nrf_nvmc_read_bytes(msg_addr,MSG_Store.R_FLASH_MSG_BUFF,(msg_len+MSG1_FLASH_HEAD_LEN));//读取数据
			//获取分包个数，及每个分包的长度
			Msg_Packet.R_PKT_PUSH_NUM = msg_len>>MSG1_PACKET_OFFSET;

			for(j=0;j<Msg_Packet.R_PKT_PUSH_NUM;)//
			{
				Msg_Packet.PKT_PUSH_LEN[j] = MSG1_PACKET_MAX_VALUE;
				//获取每包数据
				my_memcpy(&Msg_Packet.PKT_PUSH_BUF[j][0],
				&MSG_Store.R_FLASH_MSG_BUFF[MSG1_STORE_IDX+(j<<MSG1_PACKET_OFFSET)],MSG1_PACKET_MAX_VALUE);
				j++;
			}
			reminder = msg_len%MSG1_PACKET_MAX_VALUE;
			#if tim_cal
			uint32_t d = NRF_RTC0->COUNTER;
			#endif
			if(reminder)
			{
				//获取每包数据
				my_memcpy(&Msg_Packet.PKT_PUSH_BUF[j][0],
				&MSG_Store.R_FLASH_MSG_BUFF[MSG1_STORE_IDX+(j<<MSG1_PACKET_OFFSET)],reminder);
				Msg_Packet.PKT_PUSH_LEN[j] = reminder;
				Msg_Packet.R_PKT_PUSH_NUM++;//下发的分包个数
			}
			else
			{
//				Msg_Packet.PKT_PUSH_LEN[j] = 0;
			}
			Msg_Packet.PKT_CUR_NUM = 0;//分包编号清0
			Msg_Packet.PKT_MAX_NUM = Msg_Packet.R_PKT_PUSH_NUM-1;//获取最大分包编号
			Msg_Packet.R_PKT_PUSH_NUM_TEMP = Msg_Packet.R_PKT_PUSH_NUM;//缓存分包个数
			Msg_Packet.MSG_REPUSH_NUM = 0;//重发次数
			#if tim_cal
			uint32_t e = NRF_RTC0->COUNTER;
			#endif
			while(NRF_RADIO->STATE != RADIO_STATE_STATE_TxIdle)
			{
				ot++;
				if(ot > RADIO_OVER_TIME)
					return FALSE;			
			}
			if(NRF_RADIO->EVENTS_END)
				NRF_RADIO->EVENTS_END = 0;
			#if tim_cal
			uint32_t f = NRF_RTC0->COUNTER;
			#endif
			return TRUE;			
		}
		return FALSE;
	}
	return FALSE;
}


/************************************************* 
@Description:读写器-获取要下发的消息1
@Input:msg1_seq消息1编号
@Output:
@Return:1:获取成功，0：获取失败
*************************************************/ 
uint8_t Reader_Msg2_Get(uint8_t msg2_seq)
{
	uint8_t tag_msg2_seq;//标签消息序号
	tag_msg2_seq = (msg2_seq&TAG_MSG2_Msk)>>TAG_MSG2_Pos;
	if(tag_msg2_seq!=MSG_Store.R_MSG2_Seq)
	{
		return TRUE;
	}	
	return FALSE;	
}
///************************************************* 
//@Description:读写器-启动消息命令
//@Input:
//@Output:
//@Return:
//*************************************************/ 
//void Radio_MSG_Start(uint8_t *msg_buf,uint8_t* src)
//{
//	uint8_t len;
//	msg_buf[RADIO_S0_IDX] = RADIO_S0_DIR_DOWN;//S0下行
//	my_memcpy(&msg_buf[TAG_ID_IDX],&src[TAG_ID_IDX],RADIO_ID_LENGTH);//2~5目标ID
//	my_memcpy(&msg_buf[READER_ID_IDX],&DeviceID,RADIO_RID_LENGTH);//6~9读写器ID
//	msg_buf[CMD_IDX] = MSG_PUSH_CMD;
//	msg_buf[CMD_IDX+1] = MSG_START_END_VALUE>>8;
//	msg_buf[CMD_IDX+2] = MSG_START_END_VALUE;
//	len = CMD_ONEFIX_LENGTH + MSG_START_END_LEN;
//	msg_buf[RADIO_LENGTH_IDX] = len;
//	msg_buf[len+RADIO_HEAD_LENGTH-1]=Get_Xor(msg_buf,(len+1));
//}
/************************************************* 
@Description:读写器-下一包序号
@Input:
@Output:
@Return:
*************************************************/
void MSG_NEXT_PKT(void)
{
	Msg_Packet.R_PKT_PUSH_NUM_TEMP--;//发送成功再减一
	Msg_Packet.PKT_CUR_NUM++;//发送成功再加1
}
/************************************************* 
@Description:读写器-消息发送
@Input:
@Output:
@Return:
*************************************************/ 
void Radio_MSG_Push(uint8_t* tid_src)
{
	if(Msg_Packet.R_PKT_PUSH_NUM_TEMP > 0)//还有消息需要发送
	{
		cmd_packet.packet[CMD_IDX] = (RADIO_DIR_DOWN<<RADIO_DIR_POS)&RADIO_DIR_Msk;//下行
		cmd_packet.packet[CMD_IDX] |= MSG_PUSH_CMD;//消息命令
		my_memcpy(&cmd_packet.packet[TAG_ID_IDX],tid_src,RADIO_TID_LENGTH);//4~8目标ID
		my_memcpy(&cmd_packet.packet[READER_ID_IDX],&DeviceID,RADIO_RID_LENGTH);//8~12读写器ID
		cmd_packet.packet[CMD_PARA_IDX] = (msg_news<<READER_MSG_TYPE_Pos)&READER_MSG_TYPE_Msk;//消息分类
		cmd_packet.packet[CMD_PARA_IDX] |= (Msg_Packet.MSG_PUSH_SEQ<<READER_MSG_SEQ_Pos)&READER_MSG_SEQ_Msk;//消息序号
		cmd_packet.packet[CMD_PARA_IDX+1] = (Msg_Packet.PKT_MAX_NUM<<PKT_MAX_NUM_Pos)&PKT_MAX_NUM_Msk;//最大编号
		cmd_packet.packet[CMD_PARA_IDX+1] |= (Msg_Packet.PKT_CUR_NUM<<PKT_CUR_NUM_Pos)&PKT_CUR_NUM_Msk;//当前分包编号

		//拷贝消息数据
		my_memcpy(&cmd_packet.packet[MSG_DATA_IDX],&Msg_Packet.PKT_PUSH_BUF[Msg_Packet.PKT_CUR_NUM][0],
					Msg_Packet.PKT_PUSH_LEN[Msg_Packet.PKT_CUR_NUM]);
		//点对点固定长度+消息头+消息数据长度
		cmd_packet.length = CMD_ONEFIX_LENGTH + MSG_HEAD_LEN + Msg_Packet.PKT_PUSH_LEN[Msg_Packet.PKT_CUR_NUM];
		//射频长度
		cmd_packet.packet[RADIO_LENGTH_IDX] = cmd_packet.length ;
		//异或
		cmd_packet.packet[PYLOAD_XOR_IDX] = Get_Xor(cmd_packet.packet+CMD_IDX,cmd_packet.length-1);//PAYLOAD-XOR
	}
	if(Msg_Packet.PKT_CUR_NUM == Msg_Packet.PKT_MAX_NUM)//一包发送完成
	{
		Msg_Packet.MSG_REPUSH_NUM++;//重发次数+1
		if(Msg_Packet.MSG_REPUSH_NUM <= MSG1_RPUSH_TIMES)
		{	
			Msg_Packet.R_PKT_PUSH_NUM_TEMP = Msg_Packet.R_PKT_PUSH_NUM;//包个数
			Msg_Packet.PKT_CUR_NUM = 0;//当前包清空
		}
	}
	
	//包++射频中处理
//	Msg_Packet.R_PKT_PUSH_NUM--;//发送成功再减一	
//	Msg_Packet.PKT_CUR_NUM++;//发送成功再加1
}


