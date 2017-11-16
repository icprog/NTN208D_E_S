#include "App_Sys.h"

/*--------------清空BUFF-----------------*/
void my_memset(u8 *dest,u8 ch,u16 size)
{
	u16 i;
	for(i=0;i<size;i++)
	{
		*(dest+i) = ch;
	}
}

/*
Description:拷贝
Input:state : 
Output:
Return:无
*/
//void my_memcpy(u8 *dest,const u8 * src,u16 size)
//{
//	u16 i;
//	for(i = 0 ; i < size ; i++)
//	{
//		*(dest+i) = *(src+i);
//	}
//}
void *my_memcpy(void* dest,const void* src,u16 size)
{
	char *tmp = dest;
	const char *s = src;
	while(size--)
		*tmp++ = *s++;
	return dest;
}

/*
@Description:得到异或值
@Input:src:原数组，长度
@Output:
@Return:无
*/
uint8_t Get_Xor(u8 *src,u16 size)
{
	uint8_t temp = 0,i;
	for(i = 0;i<size;i++)
	{
		temp ^= src[i];
	}
	return temp;
}

/************************************************* 
@Description:校验和检验
@Input:src:原数组，长度
@Output:
@Return:1:成功
*************************************************/  
uint8_t CheckSum(u8 *src,u16 size)
{
	uint8_t temp = 0,i;
	for(i = 0;i<size-1;i++)
	{
		temp += src[i];
	}
	if(temp == src[i])
		return 1;
	else 
		return 0;
}
/************************************************* 
@Description:得到校验和
@Input:src:原数组，长度
@Output:
@Return:1:成功
*************************************************/  
uint8_t GetSum(u8 *src,u16 size)
{
	uint8_t temp = 0,i;
	for(i = 0;i<size;i++)
	{
		temp += src[i];
	}
	return temp;
}

/*---------------比较函数---------------------*/
/*
if Return value < 0 then it indicates str1 is less than str2.
if Return value > 0 then it indicates str2 is less than str1.
if Return value = 0 then it indicates str1 is equal to str2.
*/
//int memcmp(const void *str1, const void *str2, size_t n)
//{
//		const char* pSrc1 = (char*)src1;
//		const char* pSrc2 = (char*)src2;
//		while (len-- > 0)
//		{
//			if (*pSrc1++ != *pSrc2++) 
//			{
//				return *pSrc1 < *pSrc2 ? -1 : 1;
//			}
//		}
//		return 0; 
//}
//not equal return 1
uint32_t my_memcmp_const(int32 *src,int32 const_value,u32 size)
{
	u32 i;
	for(i=0;i<size;i++)
	{
		if(const_value != *(src++))
		{
			return 1;
		}
	}
	return 0;
}


