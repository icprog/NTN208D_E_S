#ifndef __App_Common_H
#define __App_Common_H	   
#include "App_Sys.h"
#include "App_Var.h"
#include "App_Protocol.h"
void Com_ChannelConfigAck_To_Master(u8 *temp,u16 state);
void Com_GetVerAck_To_Master(u8 *temp,u8* tVerName);
void Com_Update_Ack_To_Master(u8 *temp,u8 state);
#endif
















