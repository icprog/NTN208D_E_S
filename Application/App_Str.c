#include "App_Str.h"  
  

/************************************************* 
@Description:对比字符串str1和str2
@Input:
*str1:字符串1指针 
*str2:字符串2指针
@Output:
@Return:0，相等;1，不相等;
*************************************************/ 
u8 app_strcmp(u8 *str1,u8 *str2)
{
	while(1)
	{
		if(*str1!=*str2)return 1;//不相等
		if(*str1=='\0')break;//对比完成了.
		str1++;
		str2++;
	}
	return 0;//两个字符串相等
}
	 
/************************************************* 
@Description:把str1的内容copy到str2，包含结束符
@Input:
*str1:字符串1指针 
*str2:字符串2指针
@Output:
@Return:
*************************************************/ 
void app_strcopy(u8*str1,u8 *str2)
{
	while(1)
	{										   
		*str2=*str1;	//拷贝
		if(*str1=='\0')break;//拷贝完成了.
		str1++;
		str2++;
	}
}

/************************************************* 
@Description:得到字符串的长度(字节)
@Input:
*str:字符串指针
@Output:
@Return:字符串的长度
*************************************************/    
u8 app_strlen(u8*str)
{
	u8 len=0;
	while(1)
	{							 
		if(*str=='\0')break;//拷贝完成了.
		len++;
		str++;
	}
	return len;
}

/************************************************* 
@Description:m^n函数
@Input:m,n
@Output:
@Return:m^n次方
*************************************************/  
u32 app_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}	    

/************************************************* 
@Description:把字符串转为数字
支持16进制转换,但是16进制字母必须是大写的,且格式为以0X开头的.
不支持负数 
@Input:
*str:数字字符串指针
*res:转换完的结果存放地址.
@Output:
@Return:
0，成功转换完成.其他,错误代码.
//1,数据格式错误.2,16进制位数为0.3,起始格式错误.4,十进制位数为0.
*************************************************/  
u8 app_str2num(u8*str,u32 *res)
{
	u32 t;
	u8 bnum=0;	//数字的位数
	u8 *p;		  
	u8 hexdec=10;//默认为十进制数据
	p=str;
	*res=0;//清零.
	while(1)
	{
		if((*p<='9'&&*p>='0')||(*p<='F'&&*p>='A')||(*p=='X'&&bnum==1))//参数合法
		{
			if(*p>='A')hexdec=16;	//字符串中存在字母,为16进制格式.
			bnum++;					//位数增加.
		}else if(*p=='\0')break;	//碰到结束符,退出.
		else return 1;				//不全是十进制或者16进制数据.
		p++; 
	} 
	p=str;			    //重新定位到字符串开始的地址.
	if(hexdec==16)		//16进制数据
	{
		if(bnum<3)return 2;			//位数小于3，直接退出.因为0X就占了2个,如果0X后面不跟数据,则该数据非法.
		if(*p=='0' && (*(p+1)=='X'))//必须以'0X'开头.
		{
			p+=2;	//偏移到数据起始地址.
			bnum-=2;//减去偏移量	 
		}else return 3;//起始头的格式不对
	}else if(bnum==0)return 4;//位数为0，直接退出.	  
	while(1)
	{
		if(bnum)bnum--;
		if(*p<='9'&&*p>='0')t=*p-'0';	//得到数字的值
		else t=*p-'A'+10;				//得到A~F对应的值	    
		*res+=t*app_pow(hexdec,bnum);		   
		p++;
		if(*p=='\0')break;//数据都查完了.	
	}
	return 0;//成功转换
}

/************************************************* 
@Description:把十进制转换成字符串格式，不包含结束符,最大65535
@Input:
@Output:
@Return:返回字符串长度
*************************************************/  
u8 DectoStr(u8*str,u16 dec)
{
	uint8_t	dlen,didx;
	if(dec>=10000) dlen=5;
	else if(dec>=1000) dlen=4;
	else if(dec>=100) dlen=3;
	else if(dec>=10) dlen=2;
	else if(dec) dlen=1;
	else dlen=0;
	if(dlen)
	{		
		for(didx=dlen;didx>0;didx--)
		{
			str[didx-1]=dec%10+'0';	//toCHAR
			dec/=10;
		}		
	}
	return dlen;
}






