#include "spi.h"
/*********************************************************************/
/* ���������ѧϰ�ο���δ�������������������κ���ҵ��;            */
/* FuIPרΪ��Ƭ�������Ƶ�TCP/IPЭ��ջ���������κ�8,16,32λ��Ƭ��     */
/* �����򣬿����ڿͻ��ˣ������    					            	 */
/* ������̳��www.fuhome.net/bbs/						             */
/* ��ֲ��ע�⣬�˳���ʹ��Ӳ��SPI��ģ��SPI�������𣬱�����Ĭ����ģ��  */
/* ���͸������ݱ�����ARP DHCP Ping TCP�������/�ͻ��ˣ� UDP        */
/* FuIP�����ڣ��ͻ�/���񣬲�������������ɽ���fuhome.net ƽ̨����Զ�̿���*/
/* �汾��v3.1 2018��7��23��						                     */
/* �����ܶ�	�˼�									       		     */
/* ���ڸ��£������ע�������ţ����������룬����	     			     */
/* Copyright fuhome.net δ��֮�� ʵ���ң��ÿƼ���������				 */
/*********************************************************************/

#define SPI_SELECT 0  //����˺��������Ҫ�ٶȻ���Ҫ�ռ�
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
	/*ʹ������(leanfo)�ṩ��SPI��д����        */
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
