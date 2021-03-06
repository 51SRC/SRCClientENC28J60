#ifndef __fu_app_H__
#define __fu_app_H__
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

/*******************************************/
/* 功能：处理UDP包               		   */
/* 输入：无					               */
/* 输出：无							       */
/* 备注：有效数据已经转存到DATA[]		   */
/*******************************************/
void App_UDP_Bag(void);

/*******************************************/
/* 功能：处理TCP包               		   */
/* 输入：无					               */
/* 输出：无							       */
/* 备注：有效数据已经转存到DATA[]		   */
/*******************************************/
void App_TCP_Bag(void);

void App_Bag(U8 Index,U8 xdata *p,U16 len);


void SendAckData(U8 len, unsigned char *RES_DATA);
unsigned char CheckBCC(unsigned char len, unsigned char *recv);

extern  unsigned int   Timer4_Count;
sbit Buzzer    =  P5 ^ 4;           	// 蜂鸣器
sbit LED      =  P3 ^ 2;         		  // LED灯
//U8 xdata SRCCID[17] = {"SRC00000000000001"};
#endif 