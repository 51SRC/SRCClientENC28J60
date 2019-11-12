#ifndef __SPI_H_
#define __SPI_H_
#include "mcuinit.h"
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


/* 关于单片机 I/O口的设定*/
sbit CSN  = P1^5 ;// enc28j60-- CS
sbit SIN  = P1^4 ;// MOSI
sbit SON  = P1^3 ;// MISO
sbit SCKN = P1^6 ;// SCK

////呼叫卡端口
//sbit CSN  = P4^1 ;// enc28j60-- CS
//sbit SIN  = P4^2 ;// MOSI
//sbit SON  = P4^4 ;// MISO
//sbit SCKN = P4^3 ;// SCK
//*/

//sbit RSTN = P1^4 ;// RST 自行决定复位方式

void WriteByte(U8 temp);
U8 ReadByte(void);

#endif
