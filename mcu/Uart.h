#ifndef __UART_H__
#define __UART_H__
#include "mcuinit.h"
#include "fu_config.h"

#if DEBUG
	#define UART1 1			//默认开启串口1，请自行改动
#else
	#define UART1 0//不开启
#endif

#define UART1_485 0		//串口1是否485通信
#if UART1_485
	sbit RS485_1=P3^6;	//定义485的使能脚
//	sbit RS485_1=P3^4;	//老卡
//	sbit RS485_1=P3^3;	//温湿度卡
#endif
#define BAUD1 115200		//串口1波特率，即每秒发送BAUD个位，如果两串口都用独立波特率发生器，则波特率要设成一致
#define UART1_NONS 1	//异步发送，与同步发送二者选其一
#define UART1_SEND 0	//同步发送，两个同时为1，为异步发送
#define UART1_RECV 0	//允许接收
#define UART1_MAXRI 50	//串口1最大接收缓存区
#define UART1_MAXTI 500	//串口1最大接收缓存区

#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7

#if UART1
	extern U8 xdata Uart1RBuf[UART1_MAXRI];		//接收缓存区
	#if UART1_MAXRI>255
		extern U16 xdata Uart1Ri,Uart1Ri2;		//Uart1Ri为剩余接收个数，已接收个数
	#else
		extern U8 xdata Uart1Ri,Uart1Ri2;		//Uart1Ri为剩余接收个数，已接收个数
	#endif
	extern U8 xdata Uart1TBuf[UART1_MAXTI];		//发送缓存区
	#if UART1_MAXTI>255
		extern U16 xdata Uart1Ti,Uart1Ti2;		//发送记数
	#else
		extern U8 xdata Uart1Ti,Uart1Ti2;		//发送记数
	#endif
	void Uart1_Init(void);	//串口1初始化
	#define TING1 (Uart1Ti!=Uart1Ti2)		//如果有字符可发送
	void Uart1_SendStr(U8 *send);	//实时发送一外字符
	void Uart1_Send(U8 send);	//实时发送一外字符
	void Uart1_SendHex(U8 send);//转成16进制发送一个字符
	void Uart1_SendNum(U8 send);
	#define Uart1_SendFlag(i,s) {Uart1_SendNum(i);Uart1_SendStr(s);Uart1_SendStr("\r\n");}
	#define Uart1_SendIP(i1,i2,i3,i4) {Uart1_SendStr("IP:");Uart1_SendNum(i1);Uart1_Send('.');Uart1_SendNum(i2);Uart1_Send('.');Uart1_SendNum(i3);Uart1_Send('.');Uart1_SendNum(i4);Uart1_SendStr("\r\n");}
	#define RING1 (Uart1Ri!=Uart1Ri2)		//已经接收到数据，则用下面的函数接收
	U8 Uart1_Recv();	//接收一个字符
#endif

#endif
