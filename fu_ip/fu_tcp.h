#ifndef __fu_TCP_H__
#define __fu_TCP_H__
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

#define TCP_URG 0x20
#define TCP_ACK 0x10	//表示响应
#define TCP_PSH 0x08	//表示有 DATA数据传输
#define TCP_RST 0x04	//表示连接重置
#define TCP_SYN 0x02	//表示建立连接
#define TCP_FIN 0x01	//表示关闭连接

extern U8 xdata TCP_False[12];//TCP伪首部，12字节
extern U8 xdata TCP_Head[20];//TCP首部，长度20字节

/*******************************************/
/* 功能：TCP 首部填充  20字节       	   */
/* 输入：TCP数据包长度                     */
/* 输出：无							       */
/*******************************************/
void TCP_Head_Copy(U8 Index,U8 Flag);

/*******************************************/
/* 功能：填充TCP伪首部 12字节      		   */
/* 输入：数据数组长度				       */
/* 输出：无							       */
/*******************************************/
void TCP_Flase_Copy(U8 Index,U16 len);//len 为偶数

/********************************************/
/* 功能：检验和处理函数           			*/
/* 输入：无									*/
/* 输出：16位TCP检验和						*/
/********************************************/
U16 TCP_Check_Code(U16 len);

/********************************************/
/* 功能：组TCP数据包函数           			*/
/* 输入：无									*/
/* 输出：无									*/
/********************************************/
void TCP_Send(U8 Index,U8 lx,U16 len);

#endif