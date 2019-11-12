#include "spi.h"
/*********************************************************************/
/* 本程序仅供学习参考，未经作者允许，不得用于任何商业用途            */
/* FuIP专为单片机而定制的TCP/IP协议栈，适用于任何8,16,32位单片机     */
/* 主程序，可用于客户端，服务端    					            	 */
/* 技术论坛：www.fuhome.net/bbs/						             */
/* 移植请注意，此程序使用硬件SPI，模拟SPI会有区别，本程序默认是模拟  */
/* 发送各种数据报程序，ARP DHCP Ping TCP（服务端/客户端） UDP        */
/* FuIP可用于，客户/服务，不限连接数，亦可接入fuhome.net 平台进行远程控制*/
/* 版本：v3.1 2018年7月23日						                     */
/* 龙剑奋斗	乘简									       		     */
/* 定期更新，敬请关注——开放，交流，深入，进步	     			     */
/* Copyright fuhome.net 未来之家 实验室，让科技融入生活				 */
/*********************************************************************/

#define SPI_SELECT 0  //加入此宏的作用是要速度还是要空间
#if SPI_SELECT==1

	void WriteByte(U8 temp)
	{
		U8 i;
		
		for(i=0;i<8;i++)
		{
			SCKN=0;//Delay_10us();
			SIN = temp & 0x80;
			SCKN=1;//Delay_10us();
			temp <<=1;
		}
		SCKN=0;
		
	}
	
	U8 ReadByte(void)
	{
		U8 i,dat1;
		
		SCKN=0;dat1=0;
		for(i=0;i<8;i++)
		{	
			SCKN=1;//Delay_10us();
			dat1 <<=1;
			dat1 |= SON; 
			SCKN=0;	//Delay_10us();
		}
		
		return dat1;
	}
	
#else
	/*******************************************/
	/*使用网友(leanfo)提供的SPI读写程序        */
	/*******************************************/

	unsigned char bdata temp_bit;
	sbit temp_bit0 = temp_bit^0;
	sbit temp_bit1 = temp_bit^1;
	sbit temp_bit2 = temp_bit^2;
	sbit temp_bit3 = temp_bit^3;
	sbit temp_bit4 = temp_bit^4;
	sbit temp_bit5 = temp_bit^5;
	sbit temp_bit6 = temp_bit^6;
	sbit temp_bit7 = temp_bit^7;

	void WriteByte(U8 temp)
	{
	    temp_bit = temp;
	    SIN=temp_bit7;		SCKN=1;		SCKN=0;
	    SIN=temp_bit6;		SCKN=1;		SCKN=0;
	    SIN=temp_bit5;		SCKN=1;		SCKN=0;
	    SIN=temp_bit4;		SCKN=1;		SCKN=0;
	    SIN=temp_bit3;		SCKN=1;		SCKN=0;
	    SIN=temp_bit2;		SCKN=1;		SCKN=0;
	    SIN=temp_bit1;		SCKN=1;		SCKN=0;
	    SIN=temp_bit0;		SCKN=1;		SCKN=0;
	}

	U8 ReadByte(void)
	{
		SCKN=1;		temp_bit7=SON;		SCKN=0;
		SCKN=1;		temp_bit6=SON;		SCKN=0;
		SCKN=1;		temp_bit5=SON;		SCKN=0;
		SCKN=1;		temp_bit4=SON;		SCKN=0;
		SCKN=1;		temp_bit3=SON;		SCKN=0;
		SCKN=1;		temp_bit2=SON;		SCKN=0;
		SCKN=1;		temp_bit1=SON;		SCKN=0;
		SCKN=1;		temp_bit0=SON;		SCKN=0;
		return temp_bit;
	}

#endif
