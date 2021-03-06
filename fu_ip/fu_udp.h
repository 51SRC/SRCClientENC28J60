#ifndef __fu_udp_H__
#define __fu_udp_H__
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

extern U8 xdata UDP_False[12];//UDP伪首部，12字节
extern U8 xdata UDP_Head[8];//UDP首部，长度8字节


/*******************************************/
/* 功能：UDP 首部填充  8字节       		   */
/* 输入：数据数组地址，长度                */
/* 输出：无							       */
/*******************************************/
void UDP_Head_Copy(U8 Index,U8 lx,U16 len);

/*******************************************/
/* 功能：填充UDP伪首部 12字节      		   */
/* 输入：数据数组长度				       */
/* 输出：无							       */
/*******************************************/
void UDP_Flase_Copy(U8 Index,U8 lx,U16 len);//len 为偶数

/********************************************/
/* 功能：组UDP数据包函数           			*/
/* 输入：lx:1为DNS，0为UDP                  */
/* 输出：无									*/
/********************************************/
void UDP_Send(U8 Index,U8 lx,U16 len);

#endif