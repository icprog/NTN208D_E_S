 /* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef __App_UART_H
#define __App_UART_H

/*lint ++flb "Enter library region" */

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
使用说明
需要先初始化串口
当然需要定义LOG_ON
//调用debug_printf，需要
#define ENABLE_DEBUG_PRINT

*******************************************************************************/
#ifndef __app_debug_h__
#define __app_debug_h__
#include <stdbool.h>
#define LOG_ON

#ifdef LOG_ON
#define ENABLE_UART_LOG
//#define ENABLE_LOG_PRINT
//#define ENABLE_DEBUG_PRINT
#define ENABLE_UART_PRINT
#endif


#if defined(ENABLE_UART_LOG)
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "App_Sys.h"

uint8_t simple_uart_get(void);
bool simple_uart_get_with_timeout(int32_t timeout_ms, uint8_t *rx_data);
void simple_uart_put(uint8_t cr);
void simple_uart_putstring(const uint8_t *str);
void simple_uart_config(uint8_t rts_pin_number, uint8_t txd_pin_number, uint8_t cts_pin_number, uint8_t rxd_pin_number, bool hwfc);
void UART_Send(uint8_t *buf,uint16_t size);


static char uart_buffer[256];
static void uart_printf(const char *format, ...)
{
	va_list args;
	int char_num = 0;
	
	if (!format)
		return;

	va_start(args, format);
	//char_num = vsiprintf(uart_buffer, format, args);
	//char_num = _vsprintf(uart_buffer, format, args);
	char_num = vsprintf(uart_buffer, format, args);
	va_end(args);

	UART_Send((uint8_t*)uart_buffer, (uint16_t)char_num);
}
#else	// LCD

#endif


// must be call before system_clock_init() on stm32f4
bool	debug_log_init(void);


#if defined(ENABLE_UART_LOG)
#	define debug_log(...)	do { uart_printf(__VA_ARGS__); } while(0)
#	define debug_log_1(...)	do { uart_printf(__VA_ARGS__); } while(0)
#	define debug_log_2(...)	do { uart_printf(__VA_ARGS__); } while(0)
#	define debug_log_3(...)	do { uart_printf(__VA_ARGS__); } while(0)
#	define debug_log_4(...)	do { uart_printf(__VA_ARGS__); } while(0)
#else
#	define  debug_log(...)    do { } while(0)
#	define  debug_log_1(...)    do { } while(0)
#	define  debug_log_2(...)    do { } while(0)
#	define  debug_log_3(...)    do { } while(0)
#	define  debug_log_4(...)    do { } while(0)
#endif


#if defined(ENABLE_LOG_PRINT)
#	define log_printf(...)	do { debug_log(__VA_ARGS__); } while(0)
#else
#	define log_printf(...)	do { } while(0) // empty
#endif

#if defined(ENABLE_DEBUG_PRINT)
#	define debug_printf(...)	do { debug_log(__VA_ARGS__); } while(0)
#	define debug_printf_1(...)	do { debug_log_1(__VA_ARGS__); } while(0)
#	define debug_printf_2(...)	do { debug_log_2(__VA_ARGS__); } while(0)
#	define debug_printf_3(...)	do { debug_log_3(__VA_ARGS__); } while(0)
#	define debug_printf_4(...)	do { debug_log_4(__VA_ARGS__); } while(0)
#else
#	define debug_printf(...)	do { } while(0) // empty
#	define debug_printf_1(...)	do { } while(0) // empty
#	define debug_printf_2(...)	do { } while(0) // empty
#	define debug_printf_3(...)	do { } while(0) // empty
#	define debug_printf_4(...)	do { } while(0) // empty
#endif			


#if defined(ENABLE_UART_PRINT)
#	define uart_printf(...)	do { uart_printf(__VA_ARGS__); } while(0)
#	define uart_printf_1(...)	do { uart_printf(__VA_ARGS__); } while(0)
#	define uart_printf_2(...)	do { uart_printf(__VA_ARGS__); } while(0)
#	define uart_printf_3(...)	do { uart_printf(__VA_ARGS__); } while(0)
#	define uart_printf_4(...)	do { uart_printf(__VA_ARGS__); } while(0)
#else
#	define debug_printf(...)	do { } while(0) // empty
#	define debug_printf_1(...)	do { } while(0) // empty
#	define debug_printf_2(...)	do { } while(0) // empty
#	define debug_printf_3(...)	do { } while(0) // empty
#	define debug_printf_4(...)	do { } while(0) // empty
#endif			



#endif	// header guard



/*lint --flb "Leave library region" */
#endif
