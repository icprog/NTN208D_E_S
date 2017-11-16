#ifndef __APP_MSG
#define __APP_MSG
#include "App_Sys.h"

/**********************************************
每个存储区消息存储格式
消息序号+消息长度+消息内容
消息序号取值范围1~15
消息长度取值范围0~128
**********************************************/
#define MSG1_IDX_MAX_Value 2
#define MSG1_SEQ_IDX   0
#define MSG1_LEN_IDX   1
#define MSG1_STORE_IDX 2
#define MSG1_PKT_MAX_LEN 242 //15包，每包最大16字节，消息头2字节
#define MSG1_MAX_LEN 240
#define MSG1_FLASH_HEAD_LEN 2
#define MSG1_SEQ_MAX_NUM 16   //消息编号为0~15，因此为16
#define MSG1_FLASH_NUM 3		//最多存储3条消息
#define MSG1_PACKET_MAX_VALUE 16   //射频下发一包消息最大值
#define MSG1_PACKET_OFFSET    4  //消息长度
#define MSG2_LEN 6

//消息索引号
typedef enum
{
	MSG1_IDX=0,
	MSG2_IDX=1,
	MSG3_IDX=2
}MSG_IDX_Typedef;
//消息状态
typedef enum
{
	MSG_IDLE=0,
	MSG_WAIT=1,
	MSG_Packet0=2,
	MSG_Packet1=3,
	MSG_Packet2=4,
	MSG_Packet3=5,
	MSG_End=6,
	MSG_START=7
}MSG_PUSH_STATE_Typedef;

typedef struct
{
	uint32_t MSG1_ROM;//消息1地址0x3e800
	uint32_t MSG2_ROM;//消息2地址0x3e400
	uint32_t MSG3_ROM;//消息3地址0x3e000
	uint32_t NEW_MSG_ROM;//指示最新消息0x3dc00,记录消息FLASH索引
	uint32_t *MSG_ADDR[3];
	uint8_t MSG1_IDX; //消息索引0~2，用来指示FLASH地址
	uint8_t MSG_Num; //FLASH中存储的消息数量
	uint8_t Tag_Msg_Num; //缓存中消息数量
	//FLASH中消息存储
	uint8_t T_MSG1_BUFF_IDX;//1类消息T_MSG1_BUFF中的缓冲索引号
	uint8_t T_MSG1_BUFF[MSG1_PKT_MAX_LEN];	//1类消息数据缓存，消息序号+消息长度+消息内容
	uint8_t R_FLASH_MSG_BUFF[MSG1_PKT_MAX_LEN];  //读取读写器中FLASH中的消息数据
	uint8_t T_MSG1_Seq;//标签消息1序号，更新传感数据
	uint8_t R_MSG1_Seq;//读写器消息1序号，更新传感数据
	uint8_t R_MSG1_Seq_Pre;//缓存上次读写器下发的消息序号
	uint8_t T_MSG2_Seq;//标签消息2序号，更新消息2
	uint8_t R_MSG2_Seq;//读写器消息2序号，更新消息2
//	uint8_t MSG_Len;//一条消息长度
	
	
	//标签-消息显示
	uint8_t Tag_Msg_Buf[MSG1_FLASH_NUM][MSG1_PKT_MAX_LEN];//消息缓冲区  0最新消息 2最旧消息
	uint8_t New_Msg_Flag;//新消息标志位
}MSG_Store_Typedef;



u16 Message_Deal(uint8_t *p_mpacket);
void MSG_Addr_Init(void);
void MSG_Erase_ALL(void);
void MSG_Find_New(void);
void MSG_Write(uint8_t idx,u8* buff);
void Tag_Message_Get(void);
uint8_t Reader_Msg1_Get(uint8_t msg1_seq,uint8_t* tid_src);
uint8_t Reader_Msg2_Get(uint8_t msg2_seq);
void Radio_MSG_Start(uint8_t *msg_buf,uint8_t* src);
void MSG_NEXT_PKT(void);
void Radio_MSG_Push(uint8_t* tid_src);
void MSG_Packet_ReSet(void);
#endif
