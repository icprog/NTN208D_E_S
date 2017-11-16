#ifndef __App_Var_H
#define __App_Var_H
#include "App_Sys.h"
//标签ID存储地址
//CODEPAGESIZE*CODESIZE即为ROM的大小,pg_size=1024,pg_num = 256,256KB的代码存储区
#define nrf_page_size NRF_FICR->CODEPAGESIZE
#define nrf_page_num NRF_FICR->CODESIZE
#define nrf_size (nrf_page_size*nrf_page_num)  //总大小
#define ID_OFFSET 	0X3000
#define ID_BEGIN	(nrf_size-ID_OFFSET)//存放ID号，不同容量的芯片可以采用同一套代码

#define SCF_VER 0//固件版本  
typedef enum 
{
	Stand_Send,//正常发送
	Message_Rx//消息接收
}Radio_Work_Mode_Typedef;

//flash部分,代码存储区，大小根据mcu型号决定.
/*FICR寄存器中的CODEPAGESIZE对应着页个数，CODESIZE对应页包含的memory大小
CODEPAGESIZE*CODESIZE即为ROM的大小,pg_size=1024,pg_num = 256,256KB的代码存储区，FLASH。
255扇区打标记0x3fc00
254扇区存储频道参数  0x3f800
253扇区保留区  0x3f400
252扇区用户区1 0x3f000
251扇区用户区2 0x3ec00
*/

typedef struct
{
	uint32_t MARK_BASE;//打标机
	uint32_t PARA_BASE;//参数区
	uint32_t RESERVER_BASE;//保留区
	uint32_t USER1_BASE;//用户区1
	uint32_t USER2_BASE;//用户区2
	uint16_t page_size;//页个数
	uint16_t page_num;//一页的大小
	//以下几个参数用来记录ROM中的16/32块（偏移量）的计数值 值16/32，表示记录个数，0表示未记录

	uint8_t RomMark;//倒数第一个区打标机
	uint8_t	PARA_Pos;		//倒数第二个区，内部参数区
	uint8_t	RESERVER_Pos;	//倒数第三个扇区，保留区
	uint8_t	USER1_Pos;	  	//倒数第四个扇区，用户区1
	uint8_t	USER2_Pos;   	//倒数第五个扇区,用户区2
	uint8_t *pROM_Pos;		//记录指针
}ROM_BaseAddr_Typedef;   //ROM基地址定义

//内部参数区检查最大值
typedef enum
{
	PARA_BYTE2_BIT30_MAX = 0X07,//发射功率
	PARA_BYTE3_MAX = 0X7F,//全局门限
	PARA_BYTE4_MAX = 0X3F,
	PARA_BYTE5_MIN = 0X02,
	PARA_BYTE5_MAX = 0XFF,
}PARA_MAX_VALUE;

//内部参数区长度
#define PARA_RECORD_LEN									16
#define RESERVER_LEN									16
//内部FLASH区类型
#define RAM_TYPE_PARA									0
#define RAM_TYPE_RESEVER								1
#define RAM_TYPE_USER1									2
#define RAM_TYPE_USER2									3

//射频频道和方向选择
#define DATA_CHANNEL									0
#define CONFIG_CHANNEL									1
#define RADIO_RX										0
#define RADIO_TX										1
//默认射频频道
#define RADIO_ADDRESS_H 0XE1
#define RADIO_ADDRESS_L 0XE3E70000
#define RADIO_CHANNEL_DATA						20
#define RADIO_CHANNEL_CONFIG					50

//PACKET-LENGTH
#define RADIO_LENGTH_IDX 0
//PACKET-S1
#define RADIO_S1_IDX 1


/**************************************************************************
							标签上报参数定义
******************************************************************************/
//PACKET-PAYLOAD-XOR
#define PYLOAD_XOR_LENGTH 1
#define PYLOAD_XOR_IDX    2

#define RADIO_HEAD_LENGTH 2//固定头PACKET-LENGTH+PACKET-S1
//PACKET-PAYLOAD
#define PAYLOAD_BASE_IDX (RADIO_HEAD_LENGTH+PYLOAD_XOR_LENGTH)

//长度
#define RADIO_TID_LENGTH 5
#define RADIO_RID_LENGTH 5
#define RADIO_CMD_LENGTH 1
#define RADIO_SEQ_LENGTH 1

//字节索引号
#define TAG_SER_IDX										(PAYLOAD_BASE_IDX)  //流水号
#define TAG_ID_IDX										(PAYLOAD_BASE_IDX+1)//标签ID 5字节
#define TAG_STATE_IDX 									(TAG_ID_IDX+RADIO_TID_LENGTH)//状态字
#define TAG_VERSION_IDX									(TAG_STATE_IDX+1)//版本索引号
#define TAG_TYPE_IDX									(TAG_VERSION_IDX+1)//类型字
#define TAG_RSSIBASE_IDX								(TAG_VERSION_IDX+2)//125k信号强度
#define TAG_BASEDOOR_IDX								(TAG_VERSION_IDX+3)//门口定位器
#define TAG_BASENORMAL_IDX								(TAG_VERSION_IDX+5)//常规定位器
#define TAG_MSG_IDX										(TAG_VERSION_IDX+7)//消息	


//上报位索引及掩码
//SER
typedef struct
{
	uint8_t State_Ser_Num;//状态流水 0~7，设备状态每变化1次，流水加1，饱和卷绕
	uint8_t Send_Ser_Num;//发射流水 0~15，设备每发射1次，流水加1
	uint8_t State_Pre;//上一次上报的状态
//	uint8_t State_Cur;//当前状态
}Serial_Typedef;
#define TAG_STATESER_Pos 4 //状态流水
#define TAG_STATESER_Msk 0x70 
#define TAG_STATESER_MAX_VALUE 7
#define TAG_SENDSER_Pos 0  //发射流水
#define TAG_SENDSER_Msk 0x0f
#define TAG_SENDSER_MAX_VALUE 15

typedef enum
{
	RADIO_DIR_UP = 0,//上行
	RADIO_DIR_DOWN = 1//下行
}RADIO_DIR_Typedef;
#define RADIO_DIR_POS 7
#define RADIO_DIR_Msk 0X80

//标签状态
#define TAG_LOWPWR_Pos									0//低电 1：低电
#define TAG_LOWPWR_Msk									0x01
#define TAG_SHOCK_IDX									1//振动指示1：振动
#define TAG_SHOCK_Msk									0x02
#define TAG_KEY_Pos										2//按键指示 1
#define TAG_KEY_Msk										0x04
#define TAG_USSVALUE_Pos								3//传感值更新时置1
#define TAG_USSVALUE_Msk								0x08
#define TAG_BASEACT_Pos									4//被边界管理器激活时置1
#define TAG_BASEACT_Msk									0x10
#define TAG_WORKMODE_Pos								6//工作模式指示 1：保存模式 0：活动模式
#define TAG_WORKMODE_Msk								0x40
#define TAG_WITHWIN_Pos									7//接收窗口指示 1：携带接收窗口
#define TAG_WIHTWIN_Msk									0x80

//#define TAG_TIMEUPDATE_Pos							5
//#define TAG_TIMEUPDATE_Msk 							0x20
//版本信息 
#define TAG_SFVERSION_POS								0
#define TAG_SFVER_NUM									1

//校园腕带分类信息
#define TAG_TYPE_SchoolWatch 							0x03//类型字
#define TAG_RSSIBASE_Msk								0x1f//边界管理器信号强度
#define TAG_BASEID_Msk									0XFFFF//边界管理器ID
#define TAG_MSG1_Pos                                    4//一类消息编号
#define TAG_MSG1_Msk									0xf0
#define TAG_MSG2_Pos									1//2类消息编号
#define TAG_MSG2_Msk									0x0f
#define TAG_TYPE_SchoolWatch_LEN						7
//自动上报固定长度(PYLOAD_XOR_LENGTH + RADIO_SEQ_LENGTH + RADIO_TID_LENGTH +状态+版本信息)
//1+1+5+1+1
#define TAG_REPORTFIX_LENGTH 	9	
/********************************************
					标签内部参数
********************************************/
//参数字节索引
typedef enum
{
	//0
	TAGP_ACTIVESEND_IDX= 0,//自主发射使能 1：有效
	TAGP_ADAPTIVE_IDX = 0,//自适应使能 1：有效
	TAGP_SENDMODE_IDX = 0,//发射模式区分
	TAGP_WORKMODE_IDX = 0,//工作模式，0-保存模式，1-活动模式
	//1
	TAGP_AUTOREPORT_IDX = 1,//自动上报发射周期（0~7）
	//2
	TAGP_PWR_IDX = 2,//发射功率(0~7)
	//3
	TAGP_EVENTCLEAR_IDX=3,//事件清除方式
	//4
	TAGP_THRBASE_IDX = 4,//125K信号强度门限 0~31
	//5
	TAGP_SSRUNIT_IDX = 5,// 传感器采样周期 0-秒 1-分 2-时
	TAGP_SSRVALUE_IDX = 5,//传感机采样值
	TAGP_KEYALARM_IDX = 5//按键报警 0~6
}TAGP_Typedef;

//参数位索引、掩码
//自主发射、自适应发射
#define TAGP_ACTIVESEND_Pos  2//自主发射
#define TAGP_ACTIVESEND_Msk  0x04
#define TAGP_ADAPTIVE_Pos    1//自适应发射
#define TAGP_ADAPTIVE_Msk    0x02
//标签发射模式
typedef enum
{
	Passive_Send=0,//单激活发射
	Active_Send=1,//自主发射
//	Base_Passive_Send=2,//自适应模式下，初始模式为单激活发射，允许边界管理器修改初始模式
//	Base_Active_Send=3 //自适应模式下，初始模式为单激活发射，允许边界管理器修改初始模式
}Tag_SendMode_Typedef;

//工作模式,1-保存模式，0-活动模式
#define TAGP_WORKMODE_Pos								0
#define TAGP_WORKMODE_Msk								0x01
#define TAGP_WORKMODE_MAX_VALUE							0x01
//标签工作模式
typedef enum
{
	Active_Mode = 0,
	Save_Mode =1
}Tag_WorkMode_Typedef;
//标签模式
typedef struct
{
	uint8_t WorkMode;//0:活动模式 1：仓储模式
	uint8_t SendMode;//0:被动发送 1：主动发送
	uint8_t AdaptiveMode;//1:自适应模式
	uint8_t ActivatedByBase;//1：被边界管理器激活
	uint8_t Send_Period;// 发射周期
	uint8_t AlarmClr_Mode;//0:上报10次清除 1：读写器清除
	uint8_t THR_BASE;//低频激活信号门限
	uint16_t Key_Alarm_Delay;//按键报警延时
}Tag_Mode_Typedef;
//自动上报周期
#define TAGP_AUTOREPORT_Pos 0
#define TAGP_AUTOREPORT_Msk 0x07

//发射功率，0～7；0--30dbm，1--20dbm，2--16dbm,3--12dbm,4--8dbm,5--4dbm,6--0dbm,7-+4dbm
#define TAGP_PWR_Pos									0
#define TAGP_PWR_Msk									0x0F
#define TAGP_PWR_MAX_VALUE								7  //参数最大值
typedef enum 
{
	P_PWR_N30DBM	= 0,								
	P_PWR_N20DBM	= 1,
	P_PWR_N16DBM	= 2,
	P_PWR_N12DBM	= 3,
	P_PWR_N8DBM	= 4,
	P_PWR_N4DBM	= 5,
	P_PWR_P0DBM	= 6,
	P_PWR_P4DBM	= 7
}TAGP_PWR_Typedef;
//事件清除方式
#define TAGP_EVENTCLEAR_Pos								0
#define TAGP_EVENTCLEAR_Msk								0x01
typedef enum
{
	Tag_Clear = 0,
	Reader_Clear = 1
}EVENTCLEAR_Typedef;
//低频激活信号门限
#define TAGP_THRRSSI_Pos								0
#define TAGP_THRRSSI_Msk								0xFF
//报警延时 
#define TAGP_KEYALARM_Pos								0
#define TAGP_KEYALARM_Msk								0x07



//#define TAG_LOWVOLTAGE_Pos						0
//#define TAG_LOWVOLTAGE_Msk						0x01
//#define TAG_LOWVOLTAGE_WARN						0x01
//#define TAG_LOWVOLTAGE_NORMAL						0x00
/********************************************
					读写器
********************************************/
//标签类型
//typedef enum
//{
//	WCardTag=0,//白卡
//	PoliceTlinkTag = 3,//禁用标签泰凌微
//	AssetTag = 4,//资产标签
//	JudicialWchTag = 5,//司法腕带
//	SocialWchTag = 7,//社会腕带
//	SchoolWchTag = 8,//校园腕带
//	
//}TagType_Typedef;
//读写器模式
typedef struct
{
	uint8_t BaseID[2];//定位器ID
	uint8_t SendEn;//1:发送使能
	uint8_t Send_Period;// 发射周期
	uint8_t LeaveTime;//标签离开时间
	uint8_t SendBaseTime;//基准时间
	uint8_t SendCnt;//发送计数器
	uint8_t SendFlag;//允许发送
	uint8_t Channel485_Num;//485通道号
	uint8_t AppCodeNum;//代码序号
	uint8_t LeaveCheckFlag;//离开检查标志位
}ReaderPara_Typedef;
//门限
typedef struct
{
	uint8_t GobalRssi;//全局门限
	uint8_t RssiOffset;//门限偏离值，用于离开判断
	uint8_t LocalRssi[8][2];//局部门限
	uint8_t LocalRssiNum;//局部过滤个数
//	uint8_t ValidTag;//有效的标签
}RSSIFilter_Typedef;

//参数字节索引
typedef enum
{
	//0
	READERP_BASEID_IDX= 0,// 绑定的定位器ID
	//2
	READERP_SENDPARA_IDX = 2,//发送参数
	//3
	READERP_GOBALRSSI_IDX = 3,//全局门限
	//4
	READERP_RSSIOFFSET_IDX = 4,//离开门限偏移值
	//5
	READERP_LEAVETIME_IDX = 5//标签离开时间
	
}READERP_Typedef;
#define READERP_SENDEN_Pos    7//使能
#define READERP_SENDEN_Msk    0x80//使能
#define READERP_SENDPERIOD_Pos    4//发射周期
#define READERP_SENDPERIOD_Msk    0x70//发射周期
#define READERP_SENDPWR_Pos    4//发射功率
#define READERP_SENDPWR_Msk    0x70//发射功率
#define READERP_GOBALRSSI_Pos    0//全局RSSI
#define READERP_GOBALRSSI_Msk    0x7F
#define READERP_RSSIOFFSET_Pos    0//门限偏移值
#define READERP_RSSIOFFSET_Msk    0x3F
#define READERP_LEAVETIME_Pos    0//离开时间
#define READERP_LEAVETIME_Msk    0xFF
#define READERP_LEAVETIME_Min	 0x02



//标签记录
#define Sensor_Data_Length 8
#define INFO_TYPE_LEN 9
#define CAPACITY 500	//标签容量 
typedef struct
{
	uint8_t TID[RADIO_TID_LENGTH];//标签ID
	uint8_t StateSeq;//状态流水
	uint8_t State;//标签状态
	uint8_t RSSI;//信号强度
	uint8_t LeaveTime;//离开时间
	uint8_t PosVer;//定位和版本信息	
	uint8_t Sensor_Type;//传感类型
	uint8_t Sensor_Data[Sensor_Data_Length];//传感数据，指示消息类型
}TID_Typedef;
//不同类型标签长度
typedef enum
{
	tag_same_len = 11,
	tag_len1 = 11,
	tag_len2 = 16,
	tag_len3 = 18,
	tag_school_len = 17,
}tag_len_typedef;
//不同类型，标签类型字
typedef enum
{
	tag_type1 = 16,
	tag_type2 = 17,
	tag_type3 = 18,
	tag_school = 19
}tag_type_typedef;


//定位信息
#define TAG_IN_Msk 0x40  //标签进入
#define TAG_OUT_Msk 0x80  //标签离开
//读写器
#define READER_ID_IDX									(PAYLOAD_BASE_IDX + RADIO_TID_LENGTH) 

#define ID_TAP_REGION_First							0x00000001//标签ID
#define ID_TAP_REGION_Last							0xFEFFFFFF
#define ID_BROADCAST_TAG             				0xFFFFFFFE//广播id  MSB低地址 实际ID 0xFEFFFFFF
#define ID_RECEIVER_REGION_First					0xFF000000//接收器/读写器ID
#define ID_RECEIVER_REGION_Last						0xFFFDFFFF
#define ID_TRANSCEIVER_REGION_First					0xFFFE0000//做发卡器用(桌面)
#define ID_TRANSCEIVER_REGION_Last					0xFFFEFFFF
#define ID_RESERVER_REGION_First					0xFFFF0000//保留
#define ID_RESERVER_REGION_Last						0xFFFFFFFA
#define ID_SELF_RP									0xFFFFFFFB//本机ID，用于访问自身参数，使能回复(只在UART端出现)
#define ID_BC_TAP_RP								0xFFFFFFFC//广播ID，目标所有标签，使能回复
#define ID_BC_TAP_NRP								0xFFFFFFFD//广播ID，目标所有标签，禁止回复
#define ID_BROADCAST_READER_RP						0xFFFFFFFE//广播ID，目标所有接收器，使能回复
#define ID_RESERVER1								0xFFFFFFFF//保留
#define ID_RESERVER0								0x00000000//保留

#define READER_ID_MBYTE							0XFF


/***************************************************************************
						命令
定义	S0 		max_length		TID  	RID		CMD
数组位置0		1				2		6		10
**************************************************************************/
/********************************************************************
						内部文件（flash）操作
读写器下发文件命令
定义		保留 	模式		偏移  	长度	
数组位置	11		13			14		16				

标签命令读回复
定义		执行状态2字节	数据
数组位置	11				13

标签命令写回复
定义		执行状态2字节
数组位置	11
写文件
PAYLOAD长度 TID+RID+CMD+DATA+XOR = 4+4+1+6+16+1 = 32 
******************************************************************/
//执行状态
#define EXCUTE_STATE_LENGTH	2
//点对点命令固定长度
#define CMD_ONEFIX_LENGTH 		(PYLOAD_XOR_LENGTH + RADIO_CMD_LENGTH + RADIO_TID_LENGTH + RADIO_RID_LENGTH )//12
//命令回复固定长度
#define CMD_ACK_FIX_LENGTH 		(PYLOAD_XOR_LENGTH + RADIO_CMD_LENGTH + RADIO_TID_LENGTH + RADIO_RID_LENGTH + EXCUTE_STATE_LENGTH)//14
//命令索引号
#define CMD_IDX				TAG_SER_IDX//3
#define CMD_PARA_IDX        (RADIO_HEAD_LENGTH + PYLOAD_XOR_LENGTH + RADIO_CMD_LENGTH + RADIO_TID_LENGTH + RADIO_RID_LENGTH )//14
//文件操作索引号
#define FILE_MODE_OFFSET 1
#define FILE_OSET_OFFSET 2
#define FILE_LEN_OFFSET 3
#define FILE_WDATA_OFFSET 4
#define FILE_MODE_IDX (CMD_PARA_IDX+FILE_MODE_OFFSET)
#define FILE_OFFSET_IDX (CMD_PARA_IDX+FILE_OSET_OFFSET)
#define FILE_LENGTH_IDX (CMD_PARA_IDX+FILE_LEN_OFFSET)
#define FILE_WDATA_IDX (CMD_PARA_IDX+FILE_WDATA_OFFSET)			

//命令
typedef enum
{
	FILE_READ_CMD = 0x00,
	FILE_WRITE_CMD = 0X01,
	FILE_ERASE_CMD = 0X02,
//	RECORD_READ_CMD = 0X04,
//	RECORD_WRITE_CMD = 0X05,
	MSG_PUSH_CMD = 0x21,
	DEVICE_TEST_CMD = 0X22,  //整机测试命令
	ALARM_CLEAR_CMD	= 0X23  //命令清除	
}RADIO_CMD_Typedef;
#define CMD_Msk 0x7f
/**************************文件操作***************************************/
//模式
typedef enum
{
	FILE_MODE_PARA = 0x00,//内部参数区
	FILE_MODE_RESERVER = 0x01,//保留区
	FILE_MODE_USER1 = 0x02,//用户区1
	FILE_MODE_USER2 = 0x03,//用户区2
	FILE_MODE_RUNPARA = 0X04//运行参数
}MODE_Typedef;

//读取最新记录
#define FILE_OFFSET_RNEW	0XFF 	//读取最新记录
//数据
#define FILE_RDATA_IDX		(RADIO_HEAD_LENGTH + CMD_ACK_FIX_LENGTH)//16
//写偏移
// 0xff写最新记录，当记录满时，擦除所有记录
// 0xfe写最新记录，当记录满时，不擦除记录
typedef enum
{
	FILE_OFFSET_WNEW = 0xff,
	FILE_OFFSET_WNEW_NERASE = 0xfe
}FILE_OFFSET;

/******************************************************************
						执行状态
					出错代码格式2字节
				出错类别代码 出错子代码
******************************************************************/
#define EXCUTE_STATE_IDX 	CMD_PARA_IDX

typedef enum
{
	FILE_ERR = 0X06
}ERROR_H_Typedef;

typedef enum 
{
	FILE_MODE_ERR=0X00,//操作区不存在
	FILE_BODER_ERR=0X01,//超出边界，长度/读偏移量超出
	FILE_WOFFSET_ERR=0X02,//写偏移错误
	FILE_WDATA_ERR=0X03,    //数据错误
	FILE_FULL_ERR=0X04 //记录满
}ERROR_L_Typedef;
#define CMD_RUN_SUCCESS 0X0000//命令执行成功

typedef struct
{
	uint8_t mode;//文件操作模式
	uint8_t length;//读取长度
	uint16_t offset;//偏移位置
}File_Typedef;

/****************************消息操作*****************************/
//命令参数:消息（消息分类消息编号）+ 分包（最大分包编号当前分包编号）+消息内容
#define MSG_DATA_IDX  (CMD_PARA_IDX+2)
#define MSG_HEAD_LEN   2		
#define MSG_CMD_LEN   (PYLOAD_XOR_LENGTH + RADIO_CMD_LENGTH + RADIO_TID_LENGTH + RADIO_RID_LENGTH + MSG_HEAD_LEN)
typedef enum
{
	msg_calendar = 0,//日历
	msg_news=1
}MsgType_Typedef;

#define READER_MSG_TYPE_Pos  	4//消息类别
#define READER_MSG_TYPE_Msk  	0xf0
#define READER_MSG_SEQ_Pos		0//RF下发消息序号
#define READER_MSG_SEQ_Msk		0x0f

#define PKT_MAX_NUM_Pos			4//最大分包编号
#define PKT_MAX_NUM_Msk			0xf0//最大分包编号
#define PKT_CUR_NUM_Pos			0//当前分包编号
#define PKT_CUR_NUM_Msk			0x0f
#define PKT_MAX_NUM_VALUE		15//最大分包编号
#define PKT_ONE_MAX_VALUE  		16 //一包数据最大字节数

#define MSG1_SEQ_Max_Value		15//1类消息最大消息序号
#define MSG2_SEQ_Max_Value		15//2类消息最大消息序号
//#define PKT_ONE_OFFSET			4//*16 消息存储时，索引号
#define MSG1_RPUSH_TIMES		1//重发次数为1次
//分包是否接收完成
typedef enum
{
	pkt_unwrite= 0,
	pkt_write = 1
}pkt_chk_Typedef;
//消息下发和接收数据结构
typedef struct
{
	uint8_t MSG_PUSH_RID[RADIO_RID_LENGTH];//消息下发接收器ID
	uint8_t MSG_PUSH_TID[RADIO_TID_LENGTH];//消息下发标签ID
	//分包
	uint8_t T_MSG1_ONE_LEN;//标签消息1一条消息长度
	uint8_t R_PKT_PUSH_NUM;//记录读写器下发分包个数
	uint8_t R_PKT_PUSH_NUM_TEMP;//发送两遍，缓存R_PKT_PUSH_NUM
	uint8_t MSG_REPUSH_NUM;//重发次数
	uint8_t MSG_PUSH_SEQ;//下发的消息序号	
	uint8_t PKT_MAX_NUM;//最大分包编号
	uint8_t PKT_CUR_NUM;//当前分包编号
	
	uint8_t PKT_CHK[PKT_MAX_NUM_VALUE];//根据最大包编号数，检查包完整性，1表示收到该分包
	uint8_t PKT_PUSH_LEN[PKT_MAX_NUM_VALUE];//包0~14长度
	uint8_t PKT_PUSH_BUF[PKT_MAX_NUM_VALUE][PKT_ONE_MAX_VALUE];//包0~14数据
	
}Message_Typedef;
//消息错误类型
typedef enum 
{
	MSG_SUCCESS=0X00,
	MSG_PKT_SEQ_ERROR=0X02,//包序号出错
	MSG_REPEAT_ERROR=0X01,//消息重复
	MSG_CMDPARA_ERROR = 0X03, //命令参数错误
	MSG_ERROR =0X04
}ERROR_MSG_Typedef;


#define TIME_PARA_LEN							4//时间设置


extern ReaderPara_Typedef ReaderPara;//读写器模式
//函数
void SystemParaInit(void);
void UpdateRunPara(void);
uint16_t Read_Para(File_Typedef f1_para,uint8_t *para_src,uint8_t *para_dest);
uint16_t Write_Para(File_Typedef f1_para,uint8_t *para_src);
uint16_t Erase_Para(File_Typedef f1_para);
void UpdateDeviceID(uint8_t* SrcID);
void ResetDeviceID(void);
#if 0
uint8_t Read_Record(uint8_t *p_packet);
uint8_t Write_Record(uint8_t *p_packet);
#endif
void UpdateSendMode(uint8_t mode);//更新发射模式
#endif
