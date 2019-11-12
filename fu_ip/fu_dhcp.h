#ifndef __fu_dhcp_H__
#define __fu_dhcp_H__
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

#include "fu_config.h"
#include "mcuinit.h"

extern U8 xdata DHCP_Data[34];
extern U8 code DHCP_Zero[202];
extern U8 DHCP_Flag;//四个阶段1 发出请求discover  2收到应答提供offer IP 3发出选择 request 4收到确认信息ack
extern U16 DHCP_Time;//30分钟一次request
extern U8 DHCP_Client[4];
extern U8 DHCP_Server[4];


/*******************************************/
/* 功能：1、发送discover包          	   */
/* 输入：无					               */
/* 输出：无							       */
/* 备注：                                  */
/*******************************************/

void DHCP_Send_Discover();

/*******************************************/
/* 功能：3、发送request包            	   */
/* 输入：IP					               */
/* 输出：无							       */
/* 备注：                                  */
/*******************************************/

void DHCP_Send_Request(void);


#endif 