#ifndef __STC_H__
#define __STC_H__
#include <intrins.h>
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

typedef char I8;
typedef int I16;
typedef long I32;
typedef unsigned char U8; 
typedef unsigned int U16;
typedef unsigned long int U32;
typedef volatile char V8;
typedef volatile int V16;
typedef volatile long V32;
typedef volatile unsigned char VU8;
typedef volatile unsigned int VU16;
typedef volatile unsigned long VU32;

#define STC89C  0			//89C52单片机
#define STC12C  1			//12C5A单片机
#define STC15W  2			//15W4K单片机
#define MCU_TYPE STC15W		//这里更改单片机类型

#define FOSC    11059200L		//晶振频率，每秒振荡次数
//#define FOSC    22118400L		//晶振频率，每秒振荡次数
//#define FOSC	30000000L		//晶振频率，每秒振荡次数

#define FOSC_12 FOSC / 12		//定时器0 晶振12分频

#if MCU_TYPE==STC89C
	#include <REG52.H>
#endif

#if MCU_TYPE==STC12C
	#include <STC/STC12C5A60S2.H>
#endif

#if MCU_TYPE==STC15W
	#include <STC/stc15.h>
#endif
	 
#endif
