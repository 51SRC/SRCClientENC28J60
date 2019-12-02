#ifndef __fu_config_H__
#define __fu_config_H__
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

/*IP 相关参数 宏定义设置*/

#define DEBUG 0//是否开启调试信息，调试信息采用com1口发送数据
#define NET_DHCP 0//是否启用DHCP动态获取IP功能
#define DHCP_TIME 1800//DHCP续约时间，1800秒，30分钟
#define NET_TCP  1//是否启动TCP通用功能
#define NET_DNS	 1//是否启用DNS域名解析功能
#define FU_LEN 500//发送与接收数据缓冲区，这里根据你自己单片机的内存做调整，最少要在313字节，否则不能DHCP动态获取IP，
                  //当然，内存实在不够，此值要设成小于313的情况下，可以指定IP地址，UDP通信或TCP都是可以的
#define FU_CONN	 2//允许远程主机数量，数量越多，影应越慢，且占用内存也越多，每增加1个连接数，要多占用18~46字节内存

//定义网络类型
enum NET_TYPE
{
	AUTO,			//0：自动，用于接收PING
	UDP_CLIENT,		//1：UDP客户端（主机）
	UDP_SERVER,		//2：UDP服务端（从机）
	TCP_CLIENT=4,	//TCP客户端，（主机）
	TCP_SERVER=8	//TCP服务端，（从机）
};
#define UDP (UDP_CLIENT | UDP_SERVER)	//UDP协议
#define TCP (TCP_CLIENT | TCP_SERVER)	//TCP协议
#define CLIENT (UDP_CLIENT | TCP_CLIENT)//客户端
#define SERVER (UDP_SERVER | TCP_SERVER)//服务端

//定义连接状态
enum NET_STAT
{
	UDPWAIT,		//UDP服务器时，UDP等待
	CONNECTED,		//已联接
	CLOSEMY,		//主动断开
	CLOSERE,		//被动断开
	CLOSE,			//已关闭
  	DHCPGO,			//DHCP开始
	DHCPOFFER,
	DHCPREQUEST,
	DHCPOK,			//DHCP成功
	ARPDNS,			//ARP到DNS
	DNSGO,			//开始DNS
	DNSOK,			//DNS成功
	ARPGO,			//ARP开始
	ARPOK,			//ARP成功
	TCPGO,			//开始TCP连接
	TCPLISTEN,		//开启TCP监听
	TCPACCEPT,		//TCP接受连接
	TCPOK,			//TCP连接成功
	TCPWAIT,		//等待连接
};

#define FU_TTL 	255	//IP数据报生存周期

extern U8 code FU_MACIP[6];//广播地址
extern U8 xdata FU_LoaclIP[4];//本地IP地址
extern U8 xdata FU_SubnetMask[4];//子网掩码
extern U8 xdata FU_GateWay[4]; //默认网关
extern U8 xdata FU_MAC[6];//本地MAC
extern U8 xdata DATA[FU_LEN];//收发缓存

struct _ReHost	//远程主机结构
{
	enum NET_TYPE Re_TYPE;		//使用协议，是TCP还是UDP
	U8 Re_IP[4];	//远程IP
	U8 Re_MAC[6];	//远程MAC
	U16 Lc_PORT;	//本地端口
	U16 Re_PORT;	//远程端口
	enum NET_STAT Stat;//连接状态
	U8 OutTime;		//状态超时
	U8 WaitTime;	//等待时间
	#if NET_TCP
	U32 TCP_Mark;//确认ID号
	U32 TCP_CMark;//回复ID号
	#endif
	#if NET_DNS
	U8 Re_DNS[20];	//域名
	#endif
};
extern xdata struct _ReHost ReHost[FU_CONN+1];//至少定义一个访问结构，ReHost[0]用于DHCP与Ping，
//如果应用场景不需要DHCP与接受PING的功能，则也要把ReHost[0]空出来

U32 FU_Check_Sum(U16 xdata *p,U16 len);
U16 FU_Check_Code(U32 sum);

/*******************************************/
/* 功能：网络初始化                		   */
/* 输入：无					               */
/* 输出：无   				               */
/*******************************************/
void net_init(void);

/*******************************************/
/* 功能：设置网络参数              		   */
/* 输入：连接号，类型，域名，端口          */
/* 输出：无   				               */
/*******************************************/
void Set_Re_HOST(U8 Index,U8 Type,U8 *p,U16 Port);

/********************************************/
/* 功能：更改连接状态                       */
/* 输入：连接ID号                           */
/* 输出：无                                 */
/********************************************/
void SetConnState(U8 Index);

/*******************************************/
/* 功能：检测远程IP是否为局域网            */
/* 输入：无					               */
/* 输出：1为局域网，0为外网                */
/* 备注：涉及被呼叫远程主机的IP地址		   */
/*******************************************/
U8 Is_LAN(U8 Index);

/********************************************/
/* 功能：网络运行时                         */
/* 输入：无                           */
/* 输出：无                                 */
/********************************************/
void enc28j60_runtime();

#endif 
