#ifndef __APP_PROTOCOL
#define __APP_PROTOCOL
#include "App_Sys.h"

/*----------------串口定义------------------*/
#define head_bytes 2
#define len_bytes  2
#define crc_bytes 2
#define len_finish 3
#define pkt_head1 0xAA
#define pkt_head2 0xAA
/****************************************************
串口通信 主控->从机
描述	帧头	长度	通道号		
IDX		0~1		2~3	 	4		
描述	命令字	信息内容 CRC
IDX		5		6			

******************************************************/
#define US_HEAD_LEN 2  //帧头的长度
#define US_LENTH_LEN 2 //长度的长度
#define US_CRC_LEN  2  //校验和的长度
#define US_CHANNEL_LEN 1//通道号的长度
#define U_ConfigOT_LEN 1   //下发配置指令，超时字节

#define US_FIX_LEN 4 //帧长2+长度2
#define US_AfterLEN_FIX_LEN 4 //长度字段除信息内容外的固定长度:通道号+命令字+CRC校验
//信息内容长度计算buf[2]<<8|buf[3] - US_AfterLEN_FIX_LEN = 信息内容长度

/****************************************************
串口通信 从机->主控
描述	帧头	长度	通道号		
IDX		0~1		2~3	 	4		
描述	命令字	信息内容 CRC校验
IDX		5		6			
******************************************************/
#define US_HEADER_IDX 0
#define US_LEN_IDX 2
#define US_CHANNEL_IDX 4
#define US_STATE_LEN 2
#define US_CMD_IDX 5
#define US_DATA_IDX 6
#define US_FileData_IDX (US_DATA_IDX+6)
#define US_FileTimeOut_IDX (US_DATA_IDX+5)
/****************************************************
				命令 
******************************************************/
//通道配置
#define Reset_Channel485 0xFE //重置ID(通道)
#define Channel485_UnConfig  0xFF //通道未配置
typedef enum
{
	Channel485_0=0,
	Channel485_1=1,
	Channel485_2=2,
	Channel485_3=3,
}Channel_Typedef;

typedef enum
{
	US_CMD_CHANNEL_CONFIG = 0X01,//485通道分配
	US_CMD_WRITE_FILE = 0X02,//写命令
	US_CMD_READ_FILE = 0X03,//读命令
	US_CMD_GET_VER = 0X05,//获取版本
	US_CMD_MSG_PUSH = 0X09,//消息命令
	US_CMD_READ_TAG_RECORD = 0x04,//读取标签记录
	US_CMD_ERASE_FILE = 0X71,//擦除命令
	US_CMD_READER_ID = 0XF2,  //读写器ID
	US_CMD_DEVICE_TEST = 0XF3,    //整机测试
	US_CMD_ENTER_UPDATE = 0X60,	//触发升级
	US_CMD_UPDATE_SDATA = 0X61,  //升级数据包
	US_CMD_UPDATA_END = 0X62,//传输完成
}U_CMD_Typdef;

typedef enum
{
	US_CMD_CHANNEL_CONFIG_ACK = 0X81,//485通道分配
	US_CMD_WRITE_FILE_ACK = 0X82,//参数设置回复
	US_CMD_READ_FILE_ACK =0X83,//参数读取回复包
	US_CMD_ERASE_FILE_ACK = 0XF1,//参数擦除回复包
	US_CMD_GET_VER_ACK = 0X85,//获取版本回复
	US_CMD_MSG_PUSH_ACK = 0X89,//消息命令回复
	US_CMD_READ_TAG_RECORD_ACK = 0X84,//查询回复
	US_CMD_UPDATE_ACK = 0xE1,//升级确认包
}U_CMD_ACK_Typedef;

enum 
{
	ChannelConfigAckInfoLen = 2,//配置通道回复固定长度
	GetVerFixInfoLen  = 0,//获取版本固定长度
	UpdateAckFixInfoLen = 2,//升级确认固定长度
	UpdateDataFixInfoLen = 3,//升级数据包固定长度
};
//返回长度
typedef enum
{
	US_MSG_ACK_INFO_LEN = 0X03,//消息命令数据内容返回长度
	US_TIME_ACK_LEN = 0X0A,//时间命令返回长度
	US_READER_ACK_INFO_LEN = 0X05,//读写器ID命令数据内容返回长度
	US_CMD_DEVICE_TEST_LEN = 0X0A//整机测试命令返回长度
}U_ACK_LEN_Typedef;
typedef enum
{
	US_RUN_SUCCESS=0X0000,
	US_CHANNEL_ERR=0X0100,//通道配置失败
}US_State_Typedef;
typedef enum
{
	US_FILETIME_ERR = 0X0605
}UFILE_State_Typedef;
//消息
typedef enum
{
	US_MSG_SUCCESS=0X0000,
	US_MSG_LEN_ERR = 0X0700,//超出最大长度
	US_MSG_DATA_ERR = 0X0701,//时间内容错误
	US_MSG_SEQ_ERR = 0X0702//编号错误
}UMSG_State_Typedef;

typedef enum
{
	US_DEVICE_TEST_SUCCESS = 0X0000,
	US_DEVICE_TEST_ERR = 0X0901
}U_DEVICE_TEST_State_Typedef;

//列出标签或者读写器

typedef enum
{
    mode_idle = 0,//空闲模式
    mode_change = 1,//变动模式
    mode_report = 2,//自动上报
    mode_search = 3//查询
}modeset_typedef;

typedef struct
{
	uint8_t LP_Filter_En;//低电过滤使能 1：使能，0：不使能
//	uint8_t RSSI_Filter_En;//RSSI过滤使能 1：使能 0：不使能
	uint8_t RSSI_Filter_Value;//RSSI过滤值
	uint8_t TAG_TYPE;
}Filter_Typedef;
#define RADIO_RSSI_NO_Filter 127

#define US_LP_FILTEREN_Pos   0
#define US_LP_FILTEREN_Msk   0x01  //使能低电过滤
#define US_RSSI_FILTEREN_Pos 7
#define US_RSSI_FILTEREN_Msk 0x80  //使能RSSI过滤
#define US_RSSI_FILTERVALUE_Pos 0
#define US_RSSI_FILTERVALUE_Msk 0x7F  //RSSI过滤值
#define US_SEARCH_TIME_Pos     0
#define US_SEARCH_TIME_Msk     0xFF   //
#define US_LEAVE_TIME_Msk 0xFF //离开时间
#define US_LEAVE_TIME_Pos 0
#define US_TAG_TYPE_Msk 0x06
#define US_TAG_TYPE_Pos 1

typedef enum
{
    TagReader = 0,
    Tag = 1,
    Reader = 2
}type_typedef;

//超时
typedef struct
{
	uint8_t Radio_Time_En;//计数使能
	uint16_t Radio_Time_Cnt;//计数值
	uint16_t TimeOut_Cycle;//超时时间
	uint16_t Radio_SearchT_Cnt;
	uint8_t LeaveTime;//离开时间
}Time_Typedef;

typedef enum 
{
	PKT_HEAD1=0,//帧头1
	PKT_HEAD2,//帧头2
	PKT_PUSH_LEN,//帧长度
	PKT_DATA,//数据
	PKT_CRC
}state_typdef;

typedef struct
{
	uint8_t rx_state;//接收状态
	uint8_t has_finished;//接收完成
	uint8_t rx_idx;//接收索引号
	uint16_t rx_len;//数据长度
	uint8_t rx_buf[250];//接收缓冲区
	uint8_t tx_buf[1024];//发送缓冲区
	uint16_t len;//长度
	uint8_t tx_en;//1：允许发送
}UART_Typedef;
		
//升级包
#define UpStateSendCnt  3
#define UpDataLen 200
typedef struct
{
	uint16_t PktNum;//包总数
	uint16_t CurPktNum;//当前包编号
	uint16_t CurRxPktNum;//临时缓存
	uint8_t *Name;//升级包名称
	uint8_t NameLen;
	uint8_t RxState;//接收状态
	uint8_t TxState;//发送状态
	uint8_t *Data;//升级包数据
	uint8_t DataLen;//数据长度
	uint8_t TxStateCnt;
	uint8_t HexAnyState;
}Update_Typdef;
enum
{
	Update_Rx_Ok = 0,//接收成功
	Update_Rx_Error = 1,//接收失败
	Update_Rx_Same =2,//包相同
	Update_Rx_CodeNumError =3,//代码序号错误
	Update_Rx_CodeLenError =4,//长度错误
};
void Pro_Communication(void);
void Stop_Update_Time(void);
#endif

