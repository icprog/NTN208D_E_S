#include "App_Var.h"
#define App_Radio
#define App_Msg
#define App_NVMC
#include "App_Libraries.h"
//设备ID
uint8_t DeviceID[RADIO_TID_LENGTH] = {0xFF,0xFF,0xFF,0xF1,0xFF};




//按键定时器40ms
#define delay_interval   40  //40ms

//设备运行默认参数
const uint8_t  para_default[PARA_RECORD_LEN] = 
{
0xFF,//定位器
0xFF,//定位机
0x00,//是否使能发送
0x60,//全局门限
0x06,//发射功率
0x14,//门限偏移值
0x20//标签离开时间
};
//存储内部参数 
uint8_t para_record[PARA_RECORD_LEN];
uint8_t ReserveRecord[RESERVER_LEN];
ReaderPara_Typedef ReaderPara;//读写器模式
RSSIFilter_Typedef RSSIFilter;//门限值
/********************************************
					自定义FLASH存储区
********************************************/
//MARK
const uint8_t nvmc_flash_mark[11]={0x54,0x46,0x4E,0x31,0x31,0x38,0x41,0x00,0x00,0x00,0x00};//TFN118A
const uint8_t Rom_Record_Offset[4] = {16,16,32,32};//4个扇区对应的最大偏移量,分别对应参数区（0~15）、保留区、用户区1、用户区2
const uint8_t Rom_Record_Length[4] = {16,16,16,16};//每条记录对应的字节数
ROM_BaseAddr_Typedef   ROM_BaseAddr;//ROM基地址定义
uint32_t nrf_addr;//临时缓存flash地址

//命令
Radio_Work_Mode_Typedef Radio_Work_Mode = Stand_Send;


/*
@Description:返回最新记录ROM位置
@Input:state : temp_addr存储区地址，temp_size存储区总记录个数，temp_byte每条记录长度
@Output:返回最新记录位置 0：表示无记录，空 ；>1 对应的最新记录
@Return:无
*/
uint8_t Rom_Pos(uint32_t temp_addr,uint8_t temp_size,uint8_t temp_byte)
{
	uint8_t i,j;
	uint32_t base_addr = temp_addr;
//	temp_addr = (uint8_t*)temp_addr;
	//验证buff是否为空，返回最新记录的位置
	//不相等，继续查找,找到空，则返回rom位置，i =  1~~Rom_record_size。返回0表示配置区全空	
	for(i=0;i<temp_size;)
	{
		for(j=0;j<temp_byte;j++)
		{
			if(*(uint8_t*)temp_addr++ != 0xff)
				break;
		}
		if(j>=temp_byte) break;
		i++;
		temp_addr= base_addr + temp_byte*i;
	}
	return i;
}


/****************************************
@Description:获取设备ID
@Input:
@Output:
@Return:无
****************************************/
void GetDeviceID(void)
{
	nrf_nvmc_read_bytes(ID_BEGIN,DeviceID,4);
	ReaderPara.Channel485_Num = DeviceID[4];
	ReaderPara.AppCodeNum = ReaderPara.Channel485_Num + 2;
}
/************************************************* 
@Description:从机-更新设备ID
@Input:mode 0:单激活
@Output:无
@Return:无
*************************************************/ 
void UpdateDeviceID(uint8_t* SrcID)
{
	nrf_nvmc_page_erase(ID_BEGIN);
	nrf_nvmc_write_bytes(ID_BEGIN,SrcID,4);
	DeviceID[4] = SrcID[4];
	GetDeviceID();
}
/************************************************* 
@Description:从机-重置ID
@Input:mode 0:单激活
@Output:无
@Return:无
*************************************************/ 
void ResetDeviceID(void)
{
	nrf_nvmc_page_erase(ID_BEGIN);
}

/************************************************* 
@Description:更新发射模式
@Input:mode 0:单激活
@Output:无
@Return:无
*************************************************/ 
#if 0
void UpdateSendMode(uint8_t mode)
{
	//发射模式：单激活模式、主动模式
	Tag_Mode.SendMode = mode;
}
#endif


/*
@Description:读写器-运行内部参数
@Input:
@Output:
@Return:无
*/
void UpdateRunPara(void)
{
	//绑定的定位器ID
	ReaderPara.BaseID[0] = para_record[READERP_BASEID_IDX];
	ReaderPara.BaseID[1] = para_record[READERP_BASEID_IDX+1];
	//使能周期发射:1-使能
	ReaderPara.SendEn = (para_record[READERP_SENDPARA_IDX]&READERP_SENDEN_Msk) >> READERP_SENDEN_Pos;
	if(1 == ReaderPara.SendEn)
	{
		//自动发射周期，RTC基准时间250MS
		ReaderPara.Send_Period = (para_record[READERP_SENDPARA_IDX] & READERP_SENDPERIOD_Msk) >> READERP_SENDPERIOD_Pos;
		ReaderPara.Send_Period = 1<<ReaderPara.Send_Period;//1 2 4 8 16 32 64 128
		//发射功率
		radio_pwr((para_record[READERP_SENDPARA_IDX] & READERP_SENDPWR_Msk) >> READERP_SENDPWR_Pos);		
	}
	//全局门限
	RSSIFilter.GobalRssi = (para_record[READERP_GOBALRSSI_IDX]&READERP_GOBALRSSI_Msk) >> READERP_GOBALRSSI_Pos;
	//门限偏移值
	RSSIFilter.RssiOffset = (para_record[READERP_RSSIOFFSET_IDX] &READERP_RSSIOFFSET_Msk) >> READERP_RSSIOFFSET_Pos;
	//标签离开时间
	ReaderPara.LeaveTime = (para_record[READERP_LEAVETIME_IDX] & READERP_LEAVETIME_Msk) >> READERP_LEAVETIME_Pos;
}

/*
@Description:读写器-更新Rssi
@Input:
@Output:
@Return:无
*/
void UpdateLocalRssi(void)
{
	//局部门限
	uint8_t i;
	RSSIFilter.LocalRssiNum = 0;//清空设备过滤类型个数
	for(i=0;i<8;i++)
	{
		//0xff-无指定设备 门限取值范围0~0x7f
		if(ReserveRecord[i]!=0xff&&ReserveRecord[i+1]<0x80)
		{
			RSSIFilter.LocalRssi[i][0] = ReserveRecord[i];
			RSSIFilter.LocalRssi[i][1] = ReserveRecord[i+1];
			RSSIFilter.LocalRssiNum++;
		}
	}
}



/*********************************************************
@Description:文件读取
@Input: f1_para：指示模式、记录偏移、字节数
p_packet：射频命令指令
@Output:
@Return:无
内部文件（flash）操作-读写器下发读文件命令
定义		命令		保留 			模式		偏移  	长度	
数组位置	CMD_IDX		CMD_PARA_IDX	+1			+2		+3	
模式：00：内部参数区 01：保留区 02用户区 03用户区
偏移量 0xff读取最新参数
标签应答
定义		命令代码1字节 	执行状态2字节 	数据
数组位置	CMD_IDX			CMD_PARA_IDX	+2
					
*********************************************************/
uint16_t Read_Para(File_Typedef f1_para,uint8_t *para_src,uint8_t *para_dest)
{
	uint8_t max_length;
	uint16_t max_offset;//最大长度、最大偏移
	uint16_t cmd_state = CMD_RUN_SUCCESS;//命令执行状态
	switch(f1_para.mode)
	{
		case FILE_MODE_PARA://内部参数区
		{
			max_offset = Rom_Record_Offset[0];
			max_length = Rom_Record_Length[0];	
			nrf_addr = ROM_BaseAddr.PARA_BASE;//基地址
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.PARA_Pos;//最新记录
		}
		break;
		case FILE_MODE_RESERVER://保留区
		{
			max_offset = Rom_Record_Offset[1];
			max_length = Rom_Record_Length[1];
			nrf_addr = ROM_BaseAddr.RESERVER_BASE;
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.RESERVER_Pos;
		}
		break;
		case FILE_MODE_USER1://用户区1
		{
			max_offset = Rom_Record_Offset[2];
			max_length = Rom_Record_Length[2];			
			nrf_addr = ROM_BaseAddr.USER1_BASE;
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.USER1_Pos;
		}
		break;
		case FILE_MODE_USER2://用户区2
		{
			max_offset = Rom_Record_Offset[3];
			max_length = Rom_Record_Length[3];		
			nrf_addr = ROM_BaseAddr.USER2_BASE;
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.USER2_Pos;
		}
		break; 
		case FILE_MODE_RUNPARA://运行参数区
		{
			max_offset = 1;
			max_length = PARA_RECORD_LEN;
		}
		break;
		default:
		{
			cmd_state = (FILE_ERR <<8) | FILE_MODE_ERR;
		}
		break;
	}
	//长度和偏移量边界检查
	if(f1_para.length>max_length || (f1_para.offset>=max_offset && f1_para.offset<FILE_OFFSET_RNEW))
	{
		cmd_state = FILE_ERR << 8 | FILE_BODER_ERR;
	}
	//命令错误
	if(cmd_state!=CMD_RUN_SUCCESS)
	{
		return cmd_state;
	}
	else
	{
		if(f1_para.mode == FILE_MODE_RUNPARA)//读运行参数
		{
			my_memcpy(para_dest,para_record,PARA_RECORD_LEN);
		}
		else
		{
			if(FILE_OFFSET_RNEW == f1_para.offset)//读取最新记录
			{
				#if 0
				if(0 == *ROM_BaseAddr.pROM_Pos)//记录为空
				{
					if(FILE_MODE_PARA == f1_para.mode)//参数区为空，返回运行参数
						my_memcpy(&p_packet[FILE_RDATA_IDX],para_record,f1_para.length);
				}
				else
				{
				#endif
					nrf_addr += (*ROM_BaseAddr.pROM_Pos-1)*max_length;//偏移量*长度
				#if 0
				}
				#endif
			}
			else
			{
				nrf_addr += f1_para.offset*max_length;			
			}
			nrf_nvmc_read_bytes(nrf_addr, para_dest,f1_para.length);			
		}

	}
	return cmd_state;
}
/*********************************************************
@Description:参数检查-私有，不同设备，需要修改该函数
@Input:state : 
@Output:
@Return:无
**********************************************************/
uint8_t para_check(uint8_t mode,uint8_t *pdata)
{
	switch(mode)
	{
		case FILE_MODE_PARA://内部参数区
		case FILE_MODE_RUNPARA://运行参数
		{
			if(((pdata[2]&READERP_SENDPWR_Msk) >> READERP_SENDPWR_Pos) > PARA_BYTE2_BIT30_MAX)
			{
				return FALSE;
			}
			if(pdata[3] > PARA_BYTE3_MAX)
			{
				return FALSE;
			}
			if(pdata[4] > PARA_BYTE4_MAX)
			{
				return FALSE;
			}
			if(pdata[5] > PARA_BYTE5_MAX||pdata[5] < PARA_BYTE5_MIN)
			{
				return FALSE;
			}		
		}			
		break;
		case FILE_MODE_RESERVER:
		case FILE_MODE_USER1:
		case FILE_MODE_USER2:
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

/*********************************************************
@Description:文件写入
@Input:state : 
@Output:
@Return:无
内部文件（flash）操作-读写器下发读文件命令
定义		命令		保留 			模式	偏移	长度  数据		  		
数组位置	CMD_IDX		CMD_PARA_IDX	+1		+2		+3	  +4	
模式：00：内部参数区 01：保留区 02用户区 03用户区
偏移：0xff写最新记录，当记录满时，擦除所有记录
	  0xfe写最新记录，当记录满时，不擦除所有记录
标签应答
定义		命令代码1字节 	执行状态2字节 	
数组位置	CMD_IDX			EXCUTE_STATE_IDX								
*********************************************************/
uint16_t Write_Para(File_Typedef f1_para,uint8_t *para_src)
{
	uint8_t max_length;
	uint16_t max_offset;//最大长度、最大偏移
	uint16_t cmd_state = CMD_RUN_SUCCESS;//命令状态
	switch(f1_para.mode)
	{
		case FILE_MODE_PARA:
		{
			max_offset = Rom_Record_Offset[0];		
			max_length = Rom_Record_Length[0];	
			nrf_addr = ROM_BaseAddr.PARA_BASE;//基地址
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.PARA_Pos;
		}
		break;
		case FILE_MODE_RESERVER:
		{
			max_offset = Rom_Record_Offset[1];		
			max_length = Rom_Record_Length[1];
			nrf_addr = ROM_BaseAddr.RESERVER_BASE;//基地址
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.RESERVER_Pos;
		}
		break;
		case FILE_MODE_USER1:
		{
			max_offset = Rom_Record_Offset[2];		
			max_length = Rom_Record_Length[2];			
			nrf_addr = ROM_BaseAddr.USER1_BASE;//基地址
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.USER1_Pos;
		}
		break;
		case FILE_MODE_USER2:
		{
			max_offset = Rom_Record_Offset[3];		
			max_length = Rom_Record_Length[3];		
			nrf_addr = ROM_BaseAddr.USER2_BASE;//基地址
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.USER2_Pos;
		}
		break;
		case FILE_MODE_RUNPARA:
		{
			max_offset = 1;
			max_length = PARA_RECORD_LEN;
		}
		break;
		default:
		{
			cmd_state = FILE_ERR <<8 | FILE_MODE_ERR;
		}
		break;
	}
	//长度边界检查
	if(f1_para.length>max_length )
	{
		cmd_state = FILE_ERR << 8 | FILE_BODER_ERR;
	}
	//偏移检查
	switch(f1_para.offset)
	{
		case FILE_OFFSET_WNEW:break;//写最新记录
		case FILE_OFFSET_WNEW_NERASE:
		{
			if(*ROM_BaseAddr.pROM_Pos >= max_offset)//超出最大偏移量，不擦除
			{
				cmd_state = FILE_ERR << 8|FILE_FULL_ERR;//记录满
			}
		}
		break;
		default://错误
		{
			cmd_state = FILE_ERR << 8 | FILE_WOFFSET_ERR;
		}
		break;
	}
	//参数检查
	if(TRUE != para_check(f1_para.mode,&para_src[FILE_WDATA_OFFSET]))
	{
		cmd_state = FILE_ERR << 8|FILE_WDATA_ERR;
	}
	//错误，返回
	if(cmd_state!=CMD_RUN_SUCCESS)
	{
		return cmd_state;
	}
	else
	{
		if(f1_para.mode == FILE_MODE_RUNPARA)//运行参数
		{
			my_memcpy(para_record,&para_src[FILE_WDATA_OFFSET],PARA_RECORD_LEN);
		}
		else
		{
			if(*ROM_BaseAddr.pROM_Pos >= max_offset)//超出最大偏移量，擦除
			{
				nrf_nvmc_page_erase(nrf_addr);
				*ROM_BaseAddr.pROM_Pos = 0;//更新最新偏移量				
			}
			nrf_addr += *ROM_BaseAddr.pROM_Pos*max_length;
			nrf_nvmc_write_bytes(nrf_addr,&para_src[FILE_WDATA_OFFSET],f1_para.length);
			(*ROM_BaseAddr.pROM_Pos)++;
			if(f1_para.mode == FILE_MODE_PARA)//更新参数
			{
				my_memcpy(para_record,&para_src[FILE_WDATA_OFFSET],PARA_RECORD_LEN);
				UpdateRunPara();
			}			
		}

	}
	return cmd_state;
}


/*********************************************************
@Description:擦除
@Input:state : 
@Output:
@Return:无
内部文件（flash）操作-读写器下发读文件命令
定义		命令		保留 			模式		保留	保留
数组位置	CMD_IDX		CMD_PARA_IDX	+1			+2		+3		
模式：00：内部参数区 01：保留区 02用户区 03用户区
偏移：0xff写最新记录，当记录满时，擦除所有记录
	  0xfe写最新记录，当记录满时，不擦除所有记录
	  
标签应答
定义		命令代码1字节 	执行状态2字节 	
数组位置	CMD_IDX			EXCUTE_STATE_IDX							
*********************************************************/
uint16_t Erase_Para(File_Typedef f1_para)
{
	uint16_t cmd_state = CMD_RUN_SUCCESS;//命令状态
	switch(f1_para.mode)
	{
		case FILE_MODE_PARA:
		{
			nrf_addr = ROM_BaseAddr.PARA_BASE;//基地址
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.PARA_Pos;
		}
		break;
		case FILE_MODE_RESERVER:
		{
			nrf_addr = ROM_BaseAddr.RESERVER_BASE;//基地址
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.RESERVER_Pos;
		}
		break;
		case FILE_MODE_USER1:	
		{
			nrf_addr = ROM_BaseAddr.USER1_BASE;//基地址
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.USER1_Pos;
		}
		break;
		case FILE_MODE_USER2:	
		{
			nrf_addr = ROM_BaseAddr.USER2_BASE;//基地址
			ROM_BaseAddr.pROM_Pos = &ROM_BaseAddr.USER2_Pos;
		}
		break;
		default:
		{
			cmd_state = FILE_ERR <<8 | FILE_MODE_ERR;
		}
		break;
	}

	//错误，返回
	if(cmd_state!=CMD_RUN_SUCCESS)
	{
		return cmd_state;
	}
	else
	{
		nrf_nvmc_page_erase(nrf_addr);
	}
	return cmd_state;
}

/*********************************************************
@Description:读运行参数
@Input:
@Output:
@Return:无
内部文件（flash）操作-读写器下发读文件命令
定义		命令		  		
数组位置	CMD_IDX			
标签应答
定义		命令代码1字节 	执行状态2字节 	  数据
数组位置	CMD_IDX			EXCUTE_STATE_IDX  +2			
*********************************************************/
#if 0
uint8_t Read_Record(uint8_t *p_packet)
{
	uint16_t cmd_state = CMD_RUN_SUCCESS;//命令状态
	p_packet[EXCUTE_STATE_IDX] = cmd_state>>8;
	p_packet[EXCUTE_STATE_IDX+1] = cmd_state;
	my_memcpy(&p_packet[EXCUTE_STATE_IDX+2],para_record,PARA_RECORD_LEN);			
	return TRUE;
}

/*********************************************************
@Description:写运行参数
@Input:
@Output:
@Return:无
内部文件（flash）操作-读写器下发读文件命令
定义		命令		  		
数组位置	CMD_IDX			
标签应答
定义		命令代码1字节 	执行状态2字节 	  数据
数组位置	CMD_IDX			EXCUTE_STATE_IDX  +2			
*********************************************************/

uint8_t Write_Record(uint8_t *p_packet)
{
	uint16_t cmd_state = CMD_RUN_SUCCESS;//命令状态
	//参数检查
	if(TRUE != para_check(RUN_RECORD_PARA,&p_packet[FILE_WDATA_IDX]))
	{
		cmd_state = FILE_ERR << 8|FILE_WDATA_ERR;
	}
	//错误，返回
	if(cmd_state!=CMD_RUN_SUCCESS)
	{
		p_packet[EXCUTE_STATE_IDX] = cmd_state>>8;
		p_packet[EXCUTE_STATE_IDX+1] = cmd_state;
		return FALSE;
	}
	p_packet[EXCUTE_STATE_IDX] = cmd_state>>8;
	p_packet[EXCUTE_STATE_IDX+1] = cmd_state;
	my_memcpy(para_record,&p_packet[EXCUTE_STATE_IDX+2],PARA_RECORD_LEN);		
	return TRUE;
}
#endif
/************************************************* 
@Description:获取文件操作地址
@Input:无
@Output:无
@Return:无
*************************************************/ 
void System_Addr_Init(void)
{
	ROM_BaseAddr.page_size = NRF_FICR->CODEPAGESIZE;
	ROM_BaseAddr.page_num  = NRF_FICR->CODESIZE - 1;
	
	ROM_BaseAddr.MARK_BASE = ROM_BaseAddr.page_size * ROM_BaseAddr.page_num;
	
	//para area
	ROM_BaseAddr.PARA_BASE = ROM_BaseAddr.page_size * (ROM_BaseAddr.page_num-1);
	//reserved area
	ROM_BaseAddr.RESERVER_BASE = ROM_BaseAddr.page_size * (ROM_BaseAddr.page_num-2);
	//user area1
	ROM_BaseAddr.USER1_BASE = ROM_BaseAddr.page_size * (ROM_BaseAddr.page_num-3);
	//user area2
	ROM_BaseAddr.USER2_BASE = ROM_BaseAddr.page_size * (ROM_BaseAddr.page_num-4);
}
/*
@Description:获取系统运行参数，及获取参数区、保留区、用户区最新参数偏移量
@Input:state : 
@Output:
@Return:无
*/
void SystemParaInit(void)
{
	uint32_t nrf_addr;
	uint8_t base_offset;
	uint8_t flash_temp[11];//temp memory
	GetDeviceID();//获取设备ID
	System_Addr_Init();//获取基地址
	MSG_Addr_Init();//获取消息存储地址
	nrf_addr = ROM_BaseAddr.MARK_BASE;
	nrf_nvmc_read_bytes(nrf_addr,flash_temp,11);
	//最后一个扇区用来打标记，如果空，则清空ROM_BaseAddr.PARA_Pos-ROM5存储区,判断是否是新下载的程序
	if((flash_temp[0]!=nvmc_flash_mark[0])||(flash_temp[1]!=nvmc_flash_mark[1])||(flash_temp[2]!=nvmc_flash_mark[2])
		||(flash_temp[3]!=nvmc_flash_mark[3])||(flash_temp[4]!=nvmc_flash_mark[4])||(flash_temp[5]!=nvmc_flash_mark[5])
		||(flash_temp[6]!=nvmc_flash_mark[6])||(flash_temp[7]!=nvmc_flash_mark[7])||(flash_temp[8]!=nvmc_flash_mark[8])
		||(flash_temp[9]!=nvmc_flash_mark[9])||(flash_temp[10]!=nvmc_flash_mark[10]))
	{			 
		nrf_addr = ROM_BaseAddr.PARA_BASE;		
		nrf_nvmc_page_erase(nrf_addr);

		nrf_addr = ROM_BaseAddr.RESERVER_BASE;
		nrf_nvmc_page_erase(nrf_addr);

		nrf_addr = ROM_BaseAddr.USER1_BASE;
		nrf_nvmc_page_erase(nrf_addr);

		nrf_addr = ROM_BaseAddr.USER2_BASE;
		nrf_nvmc_page_erase(nrf_addr);

		nrf_addr = ROM_BaseAddr.MARK_BASE;
		nrf_nvmc_page_erase(nrf_addr);
		
		MSG_Erase_ALL();//擦除消息存储区	
		nrf_nvmc_write_bytes(nrf_addr,nvmc_flash_mark,11);
	}
	else
	{
		//打过标记读取上次存储的信息
		//内部参数区
		nrf_addr = ROM_BaseAddr.PARA_BASE;
		ROM_BaseAddr.PARA_Pos = Rom_Pos(nrf_addr,Rom_Record_Offset[0],Rom_Record_Length[0]);
		//reserver area
		nrf_addr = ROM_BaseAddr.RESERVER_BASE;
		ROM_BaseAddr.RESERVER_Pos = Rom_Pos(nrf_addr,Rom_Record_Offset[1],Rom_Record_Length[1]);
		//user area1
		nrf_addr = ROM_BaseAddr.USER1_BASE;
		ROM_BaseAddr.USER1_Pos = Rom_Pos(nrf_addr,Rom_Record_Offset[2],Rom_Record_Length[2]);
		//user area2
		nrf_addr = ROM_BaseAddr.USER2_BASE;
		ROM_BaseAddr.USER2_Pos = Rom_Pos(nrf_addr,Rom_Record_Offset[3],Rom_Record_Length[3]);	
		//更新消息参数
		MSG_Find_New();
	}
	if(ROM_BaseAddr.PARA_Pos)//更新过参数
	{
		nrf_addr = ROM_BaseAddr.PARA_BASE;
		base_offset = (ROM_BaseAddr.PARA_Pos - 1)*Rom_Record_Length[0];
		nrf_nvmc_read_bytes(nrf_addr+base_offset,para_record,Rom_Record_Length[0]);
	}		
	else//否则默认参数
	{
		my_memcpy(para_record,para_default,16);
	}
	if(ROM_BaseAddr.RESERVER_Pos)//更新过rssi
	{
		nrf_addr = ROM_BaseAddr.RESERVER_BASE;
		base_offset = (ROM_BaseAddr.RESERVER_Pos - 1)*Rom_Record_Length[1];
		nrf_nvmc_read_bytes(nrf_addr+base_offset,ReserveRecord,Rom_Record_Length[1]);		
	}
	else
	{
		my_memset(ReserveRecord,0xff,RESERVER_LEN);
	}
	UpdateRunPara();
	UpdateLocalRssi();
}
