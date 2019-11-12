#include "fu_arp.h"
#include <string.h>
#include "fu_enthernet.h"
#include "fu_config.h"
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


//硬件类型01 协议类型0x0800 硬件地址长度6 IP长度4 操作请求0x0001 应答2
U8 xdata ARP_Data[28]={0,1,8,0,6,4,0,1};
//U8 ARP_Flag=0;

/*******************************************/
/* 功能：填充ARP请求/应答数据报      	   */
/* 输入：leixing 1 请求 2应答              */
/* 输出：无							       */
/*******************************************/
void ARP_Ask(U8 leixing)
{
	ARP_Data[7]=leixing;//1请求	2应答

	//发送端MAC
	memcpy(&ARP_Data[8],FU_MAC,6);
	//发送端IP
	memcpy(&ARP_Data[14],FU_LoaclIP,4);

	if(leixing==1)
	{
		ARP_Data[18]=ARP_Data[19]=ARP_Data[20]=ARP_Data[21]=ARP_Data[22]=ARP_Data[23]=0;
	}
	else if(leixing==2)//解决多个enc28j60模块之间的通讯不正常问题
	{
		memcpy(&ARP_Data[18],ReHost[0].Re_MAC,6);//从临时保存的数据中马上回应出去
		//目的IP
		memcpy(&ARP_Data[24],ReHost[0].Re_IP,4);
	}
}

