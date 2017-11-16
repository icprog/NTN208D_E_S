#ifndef _APP_LIBRARIES_H
#define	_APP_LIBRARIES_H

#ifdef App_Uart
	#if BOARD_NTN208D_E_S
		#include "App_Uart.h"
	#endif
#endif

#ifdef App_Crc16
	#if BOARD_NTN208D_E_S
		#include "App_Crc16.h"
	#endif
#endif

#ifdef App_Init
	#if BOARD_NTN208D_E_S
		#include "App_Init.h"
	#endif
#endif

#ifdef App_Sys
	#if BOARD_NTN208D_E_S
		#include "App_Sys.h"
	#endif
#endif

#ifdef App_Common
	#if BOARD_NTN208D_E_S
		#include "App_Common.h"
	#endif
#endif

#ifdef App_Protocol
	#if BOARD_NTN208D_E_S
		#include "App_Protocol.h"
	#endif
#endif

#ifdef App_Var
	#if BOARD_NTN208D_E_S
		#include "App_Var.h"
	#endif
#endif

#ifdef App_Msg
	#if BOARD_NTN208D_E_S
		#include "App_Msg.h"
	#endif
#endif

#ifdef App_NVMC
	#if BOARD_NTN208D_E_S
		#include "nrf_nvmc.h"
	#endif
#endif

#ifdef App_Radio
	#if BOARD_NTN208D_E_S
		#include "App_Radio.h"
	#endif
#endif

#ifdef App_Reader
	#if BOARD_NTN208D_E_S
		#include "App_Reader.h"
	#endif
#endif


#ifdef App_Rtc
	#if BOARD_NTN208D_E_S
		#include "App_Rtc.h"
	#endif
#endif

#ifdef App_Str
	#if BOARD_NTN208D_E_S
		#include "App_Str.h"
	#endif
#endif

//#ifdef App_Delay
//	#if BOARD_NTN208D_E_S
//		#include "nrf_delay.h"
//	#endif
//#endif

#endif
