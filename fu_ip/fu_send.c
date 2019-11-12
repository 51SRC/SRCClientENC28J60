#include "fu_send.h"
#include <string.h>
#include "enc28j60.h"
#include "fu_config.h"
#include "fu_enthernet.h"
#include "fu_arp.h"
#include "fu_icmp.h"
#include "fu_ip.h"
#include "fu_udp.h"
#include "fu_tcp.h"
#include "fu_dhcp.h"
#include "fu_dns.h"
#include "fu_receive.h"
#include "timer.h"
#if DEBUG
	#include "uart.h"
#endif
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


/*******************************************/
/* 功能：发送数据                          */
/* 输入：无					               */
/* 输出：无							       */
/* 备注：发送数据，自动识别出是UDP还是TCP  */
/*******************************************/
void Send_Data(U8 Index,U16 len)
{
	if(Index>FU_CONN)return;//如果Index大于缓存，则直接退出

	if(ReHost[Index].Re_TYPE & UDP)
	{
		if(ReHost[Index].Stat==CONNECTED)Send_UDP_Bag(Index,len);
	}
	else if(ReHost[Index].Re_TYPE & TCP)
	{
		if(ReHost[Index].Stat==CONNECTED)Send_TCP_Bag(Index,TCP_ACK|TCP_PSH,len);//数据包(5)
	}
}

/*******************************************/
/* 功能：发送APR包            		       */
/* 输入：无					               */
/* 输出：无							       */
/* 备注：涉及被呼叫远程主机的IP地址		   */
/*******************************************/
void Send_ARP_Bag(U8 Index)
{
	if(Is_LAN(Index))//内网
	{
		//目的IP
		memcpy(&ARP_Data[24],ReHost[Index].Re_IP,4);
	}
	else//外网，或者本地IP为0.0.0.0
	{
		//目的IP
		memcpy(&ARP_Data[24],FU_GateWay,4);
	}

	//目的MAC地址FF 广播
	memcpy(EN_Head,FU_MACIP,6);

	ARP_Ask(1);//组ARP包 //1请求	2应答

	EN_Head_Copy(1);//EN以太网头，1 ARP类型

	ARP_Packet_Send();//发送ARP包
}


/*******************************************/
/* 功能：发送APR应答包            		   */
/* 输入：无					               */
/* 输出：无							       */
/* 备注：涉及被呼叫远程主机的IP地址		   */
/*******************************************/
void Answer_ARP_Bag(void)
{
	//目的MAC
	memcpy(EN_Head,ReHost[0].Re_MAC,6);	//固定Index为0，不要把别的远程MAC搞乱

	ARP_Ask(2);//1组ARP请求包 2应答包

	EN_Head_Copy(1);//EN以太网头，1 ARP类型
	ARP_Packet_Send();//发送ARP包
}

/*******************************************/
/* 描述: arp根据IP找到mac地址              */
/* 功能：主动ARP                		   */
/* 输入：无  					           */
/* 输出：状态值	21成功				       */
/*******************************************/
void ARP(U8 Index)
{
	if(ReHost[Index].OutTime!=Tim_Sec)//每秒只发送1个arp数据包
	{
		#if NET_DHCP//是否动态获取IP
			if(ReHost[0].Stat==CLOSE)//已成功DHCP
			{
				ReHost[Index].OutTime=Tim_Sec;
				Send_ARP_Bag(Index);
			}
		#else
			ReHost[Index].OutTime=Tim_Sec;
			#if DEBUG
				Uart1_SendFlag(Index,"Arp");
			#endif
			Send_ARP_Bag(Index);
		#endif
	}
}

/*******************************************/
/* 描述: 根据域名找到IP                    */
/* 功能：主动DNS                		   */
/* 输入：无  					           */
/* 输出：无                                */
/*******************************************/
void Send_DNS_Bag(U8 Index)
{
	U8 len;

	len=DNS_Request_Copy(Index);//复制域名数据

	//目的MAC地址
	memcpy(EN_Head,DNS_MAC,6);
	EN_Head_Copy(0);//EN以太网头，0 IP类型 1 ARP

	IP_Send(Index,3,28+len);//0 UDP,1 TCP,2 ICMP,3 DNS 组IP头,len

	UDP_Send(Index,1,len);//组UDP包

	UDP_Packet_Send(len);//发送UDP包
}

/*******************************************/
/* 描述: dns把域名解析为IP地址             */
/* 功能：主动发送DNS包                     */
/* 输入：无  					           */
/* 输出：状态值	21成功				       */
/*******************************************/
void DNS(U8 Index)
{
	if(ReHost[Index].OutTime!=Tim_Sec)//每秒只发送1个arp数据包
	{
		#if NET_DHCP	//是否动态获取IP
			if(ReHost[0].Stat==CLOSE)//已成功DHCP
			{
				ReHost[Index].OutTime=Tim_Sec;
				Send_DNS_Bag(Index);
			}
		#else
			ReHost[Index].OutTime=Tim_Sec;
			#if DEBUG
				Uart1_SendFlag(Index,"Dns");
			#endif
			Send_DNS_Bag(Index);
		#endif
	}
}

/******************************************************************************/
/* 描述: DHCP分4个步骤,网络中没有DHCP服务器,则IP变为169.254.0.0,5分钟循环广播 */
/*       1.发送Discove广播包,没有回复则间隔1,2,4,8,16秒重发,后转失败          */
/*       2.服务器回复OFFER,里面有IP信息,但此时服务器不是真正分配IP,只是暂保存 */
/*       3.发送申请包Request,4.回复ACK包,IP成功获得                           */
/* 功能：主动DHCP                		                                      */
/* 输入：目的IP					                                              */
/* 输出：状态值	21成功				                                          */
/******************************************************************************/
void DHCP(void)
{
	if(ReHost[0].Stat==DHCPGO && ReHost[0].OutTime!=Tim_Sec)	//1秒发送一次Discove包
	{
		ReHost[0].OutTime=Tim_Sec;//重新记时
		#if DEBUG
			Uart1_SendFlag(0,"DhcpGO");
		#endif
		DHCP_Send_Discover();	//发送广播包
	}
	else if(ReHost[0].Stat==DHCPOFFER)//收到Discover包
	{
		ReHost[0].Stat=DHCPREQUEST;
		ReHost[0].OutTime=Tim_Sec;
		#if DEBUG
			Uart1_SendFlag(0,"DhcpSendQUEST");
		#endif
		DHCP_Send_Request();		
	}
	else if(ReHost[0].Stat==DHCPREQUEST && ReHost[0].OutTime!=Tim_Sec)//1秒发送一次包,表示超时
	{
		ReHost[0].OutTime=Tim_Sec;
		#if DEBUG
			Uart1_SendFlag(0,"DhcpSendQUEST");
		#endif
		DHCP_Send_Request();		
	}
}

/********************************************/
/* 功能：等待断开连接                       */
/* 输入：连接ID号                           */
/* 输出：无                                 */
/********************************************/
void CloseWait(U8 Index)
{
	if(ReHost[Index].OutTime!=Tim_Sec)//得到回答
	{
		ReHost[Index].OutTime=Tim_Sec;
		if(ReHost[Index].WaitTime)ReHost[Index].WaitTime--;//2秒内没有收到关闭连接的包，也要关闭
		else
		{
			ReHost[Index].Stat=CLOSE;
			#if DEBUG
				Uart1_SendFlag(Index,"Close");
			#endif
		}
	}
}

/********************************************/
/* 功能：主动断开连接                       */
/* 输入：连接ID号                           */
/* 输出：无                                 */
/********************************************/
void CloseTCP(U8 Index)
{
	ReHost[Index].Stat=CLOSEMY;//主动断开
	ReHost[Index].WaitTime=2;//等待断开超时秒数
	#if DEBUG
		Uart1_SendFlag(Index,"MyClosing");
	#endif
	Send_TCP_Bag(Index,TCP_ACK|TCP_FIN,0);//最后一包，主动断开
}

/*******************************************/
/* 功能：发送TCP包            		       */
/* 输入：数据长度len	         	       */
/* 输出：无							       */
/* 备注：涉及远程IP,远程MAC,远程,本地端口号*/
/*******************************************/
void Send_TCP_Bag(U8 Index,U8 lx,U16 len)
{	
	//目标MAC
	memcpy(EN_Head,ReHost[Index].Re_MAC,6);
	EN_Head_Copy(0);//EN以太网头，0 IP类型 1 ARP

	TCP_Send(Index,lx,len);//组TCP包数据长度,远程端口
	IP_Send(Index,1,40+len);//0 UDP,1 TCP,2 ICMP组IP头， 20+20+len
	TCP_Packet_Send(len);//发送TCP包
}

/*******************************************/
/* 功能：主动TCP连接               		   */
/* 输入：无     					       */
/* 输出：无							       */
/*******************************************/
void TCP_Connect(U8 Index)
{
	//主动握手包(1)
	if(ReHost[Index].Stat==TCPGO && ReHost[Index].OutTime!=Tim_Sec)	//1秒发送一次Discove包
	{	
		#if DEBUG
			Uart1_SendFlag(Index,"TcpConn");
		#endif
		ReHost[Index].OutTime=Tim_Sec;
		Send_TCP_Bag(Index,TCP_SYN,0);//握手第一次(1)
	}
}

/*******************************************/
/* 功能：发送UDP包            		       */
/* 输入：数据长度	   		               */
/* 输出：无							       */
/* 备注：涉及远程IP,远程MAC,远程,本地端口号*/
/*******************************************/
void Send_UDP_Bag(U8 Index,U16 len)
{	
    //目的MAC地址
	memcpy(EN_Head,ReHost[Index].Re_MAC,6);
	EN_Head_Copy(0);//EN以太网头，0 IP类型 1 ARP

	IP_Send(Index,0,28+len);//0 UDP,1 TCP,2 ICMP,3 DNS组IP头,len

	UDP_Send(Index,0,len);//组UDP包

	UDP_Packet_Send(len);//发送UDP包
}

///*******************************************/
///* 功能：发送ICMP_Ping主包，主动ping       */
///* 输入：无					               */
///* 输出：无							       */
///* 备注：涉及远程IP,远程MAC,远程,本地端口号*/
///*******************************************/
//void Send_ICMP_Ping_Bag(void)
//{	
//	U8 i;
//	//目的IP
//    //目的MAC地址
//
//    if(Net_Stat>=ARPOK)//如果ARP远程MAC成功
//	{
//		//目的MAC地址
//		for(i=0;i<6;i++)
//		{
//			EN_Head[i]=Re_MAC[i];
//		}
//		ICMP_Ping_Send();//组ICMP_Ping包
//		IP_Send(0,2,60);//0 UDP,1 TCP,2 ICMP组IP头
//		EN_Head_Copy(0);//EN以太网头，0 IP类型 1ARP
//		
//		ICMP_Ping_Packet_Send();//发送ICMP_Ping包	
//	}
//}


/*******************************************/
/* 功能：发送ICMP_Ping回包     		       */
/* 输入：无					               */
/* 输出：无							       */
/* 备注：涉及远程IP,远程MAC,远程,本地端口号*/
/*******************************************/
void Send_ICMP_Ping_Back_Bag(void)
{	
	//目的IP

    //目的MAC地址
	memcpy(EN_Head,ReHost[0].Re_MAC,6);
	EN_Head_Copy(0);//EN以太网头，0 IP类型 1ARP

	ICMP_Ping_Answer_Send();//组ICMP_Ping包
	IP_Send_Ping_Back(0,60);//组IP头
	
	ICMP_Ping_Packet_Send();//发送ICMP_Ping包
}

