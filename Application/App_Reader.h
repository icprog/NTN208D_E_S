#ifndef _APP_READER_
#define _APP_READER_
typedef enum
{
	Idle,
	File_Deal,//平时发射带接收窗的文件处理
	Reader_File_Deal,
	Msg_Deal,
	Time_Set,
	Alarm_Clr,
	#if 0
	Device_Test,
	Change_Report,
	Search_Ack,
	Auto_Reoprt,
	Tag_Report
	#endif
}Work_Mode_Typedef;

extern void app_process(void);
#endif

