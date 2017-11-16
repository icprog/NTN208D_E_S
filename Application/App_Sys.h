#ifndef __SYS_H
#define __SYS_H	
#include "nrf.h"

#define TRUE 1
#define FALSE 0
#define NULL 0
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int8_t   int8;
typedef int16_t int16;
typedef int32_t int32; 
typedef float float32_t;

typedef __IO uint32_t  vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t  vu8;

void copybuf(u8 *dest,const u8 *str,u16 size);
void my_memset(u8 *dest,u8 ch,u16 size);
void *my_memcpy(void* dest,const void* src,u16 size);
uint8_t Get_Xor(u8 *src,u16 size);
uint8_t CheckSum(u8 *src,u16 size);
uint8_t GetSum(u8 *src,u16 size);
#endif
