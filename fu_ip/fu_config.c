#include "fu_config.h"
#include <string.h>
#include "fu_send.h"
#include "fu_receive.h"
#include "fu_dns.h"
#include "fu_dhcp.h"
#include "timer.h"
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


xdata struct _ReHost ReHost[FU_CONN+1];//通信连接数

/* 相关参数设置,指定IP */
U8 code FU_MACIP[6]={255,255,255,255,0xff,0xff};//广播地址
U8 xdata FU_MAC[6]={0x00,0x12,0X34,0X56,0X78,0X90};//本地MAC

#if NET_DHCP
	/* 相关参数设置,自动获取IP */
	U8 xdata FU_LoaclIP[4]={0,0,0,0};//本地IP地址
	U8 xdata FU_SubnetMask[4]={255,255,255,0};//子网掩码
	U8 xdata FU_GateWay[4]={255,255,255,255}; //默认网关
#else
	U8 xdata FU_LoaclIP[4]={192,168,0,101};//本地IP地址133/101
	U8 xdata FU_SubnetMask[4]={255,255,255,0};//子网掩码
	U8 xdata FU_GateWay[4]={192,168,0,1}; //默认网关
#endif

U8 xdata DATA[FU_LEN]={0};//收发数据缓冲区

/*******************************************/
/* 功能：网络初始化                		   */
/* 输入：无					               */
/* 输出：无   				               */
/*******************************************/
void net_init(void)
{
	U8 i;
	
	Set_Re_HOST(0,AUTO,"255.255.255.255",0);	//Re_HOST[0]用于响应ping

	if(NET_DHCP)			//如果需要DHCP，则第一步肯定是DHCP
	{
		ReHost[0].Stat=DHCPGO;
	}
	else ReHost[0].Stat=CLOSE;
	
	for(i=1;i<=FU_CONN;i++)		//初始化其它连接为默认值
	{
		ReHost[i].Re_TYPE=AUTO;	//初始类型为AUTO
		ReHost[i].Stat=CLOSE;  	//初始状态为CLOSE
	}

	//初始化FUIP的使用场景，要与FU_CONN配合使用，不要超出FU_CONN指定的连接数
	Set_Re_HOST(1,TCP_SERVER,"0.0.0.0",80);//Web服务器，用于设置IP，以后版本加上此功能
	//Set_Re_HOST(2,TCP_CLIENT,"192.168.0.106",4001);
	Set_Re_HOST(2,TCP_CLIENT,"47.104.19.111",4001);//47.104.19.111
//	Set_Re_HOST(2,UDP_CLIENT,"0.0.0.0",123);	//远程NTP服务器
//	Set_Re_HOST(2,TCP_SERVER,"0.0.0.0",8080);
//	Set_Re_HOST(3,TCP_SERVER,"0.0.0.0",8080);	//两个TCP服务器的本地端口相同，表示允许同时连接两个客户端，多写几行表示允许更多的客户端
//	Set_Re_HOST(4,UDP_CLIENT,"192.168.0.123",8080);
//	Set_Re_HOST(5,UDP_SERVER,"0.0.0.0",8080);
}


/*******************************************/
/* 功能：获取本地端口，自动加1处理         */
/* 输入：无                                */
/* 输出：未被其它连接占用的本地端口        */
/*******************************************/
U16 GetLocalPort()
{
	static U16 tPort=0;
	U16 i;
	if(++tPort>20000)//0~19999之间变化
	{
		tPort=0;		//如果第一轮IP已经分配完成
		while(1)
		{
			for(i=1;i<=FU_CONN;i++)	//检查新的IP是否已经存在
			{
				if(ReHost[i].Lc_PORT==tPort+10000)break;	//已存在，那i<=FU_CONN
			}
			if(i>FU_CONN)break;	//不存在，则直接退出
			else tPort++;		//存在，则加1，再退出
		}
	}
	return 10000+tPort;
}

/********************************************/
/* 功能：更改连接状态                       */
/* 输入：连接ID号                           */
/* 输出：无                                 */
/********************************************/
void SetConnState(U8 Index)
{
	switch(ReHost[Index].Re_TYPE)
	{
		case UDP_CLIENT:
		case TCP_CLIENT:
			if(0==ReHost[Index].Re_IP[0])//如果远程主机是域名而不是IP
			{
				ReHost[Index].Stat=ARPDNS;//开始DNS的ARP
			}
			else{
				ReHost[Index].Stat=ARPGO;//为客户端，则需要ARP
			}
		break;

		case UDP_SERVER://为服务器，则等待
			ReHost[Index].Stat=UDPWAIT;
		break;

		case TCP_SERVER://为TCP服务器，则监听
			ReHost[Index].Stat=TCPLISTEN;
		break;
	}
}

/********************************************/
/* 功能：处理输入字符串，否则复制到.Re_DNS  */
/* 输入：连接ID号，输入字符串，可为域名或IP */
/* 输出：无                                 */
/********************************************/
void DnsToIP(U8 Index,U8 *p)
{
	U8 *s=p;
	U8 d=0;
	U8 ip[4]={0,0,0,0};
	while(*s)//遍历字符串
	{
		if(*s=='.')//如果为'.'，段数+1
		{
			d++;
		}
		else
		{
			if(*s>='0' && *s<='9')//检查每1位上，都为0~9的数字
			{
				ip[d]=ip[d]*10+(*s-'0');
			}
			else//并非IP地址，而是域名
			{
				DNS_Copy(Index,p);//把域名复制到对应的缓存中，注意域名长度不要超过缓存区的长度-3
				ReHost[Index].Re_IP[0]=0;//目的IP1
				ReHost[Index].Re_IP[1]=0;//目的IP2
				ReHost[Index].Re_IP[2]=0;//目的IP3
				ReHost[Index].Re_IP[3]=0;//目的IP4
				return;//退出
			}
		}
		s++;
	}

	//IP地址正确
	ReHost[Index].Re_IP[0]=ip[0];//目的IP1
	ReHost[Index].Re_IP[1]=ip[1];//目的IP2
	ReHost[Index].Re_IP[2]=ip[2];//目的IP3
	ReHost[Index].Re_IP[3]=ip[3];//目的IP4
	ReHost[Index].Re_DNS[0]=0;
}

/*******************************************/
/* 功能：设置远程主机通信参数              */
/* 输入：Index通信ID号                     */
/*       Type:此ID做为TCP还是UDP           */
/*       p:域名"fuhome.net"或“192.168.0.4” */
/*       Port:远程或本地端口               */
/* 输出：无							       */
/*******************************************/
void Set_Re_HOST(U8 Index,U8 Type,U8 *p,U16 Port)
{
	ReHost[Index].TCP_Mark=ReHost[Index].TCP_CMark=0;
	ReHost[Index].OutTime=60;//超时时间
	ReHost[Index].Re_TYPE=Type;//连接类型
	DnsToIP(Index,p);//赋值IP地址

	if(Type & CLIENT) 		//如果为客户端，则Port为远程端口，本地端口不能重复，如果重复，则会找不到通信ID
	{
		ReHost[Index].Lc_PORT=GetLocalPort();	//取10000以上不重复端口
		ReHost[Index].Re_PORT=Port;		//远程端口
	}
	
	if(Type & SERVER)		//如果为服务器，则远程端口为0，等待连接时，再更改远程端口号
	{
		ReHost[Index].Lc_PORT=Port;		//本地端口
		ReHost[Index].Re_PORT=0;		//远程端口
	}

	SetConnState(Index);//设置初始通信状态
}

/********************************************/
/* 功能：检验和累加函数           			*/
/* 输入：字符数组与长度						*/
/* 输出：32位累加和					        */
/********************************************/
U32 FU_Check_Sum(U16 xdata *p,U16 len)
{
//考虑到有些单片机没有CY寄存器,不好移植,固还是选用这段代码
	U8 i;
	U16 l2=len>>1;//除以2
	U32 checkcode=0;//检验和

	//计算检验和 
	if(len==0)return 0;//长度为0直接返回0

	for(i=0;i<l2;i++)//循环总长度除以2次,如果为寄数,则少循环一次
	{
		checkcode+=*p++;
	}
	if((len&0x01)==0x01)//如果长度为奇数，则后边补0
	{
		l2=*(U8 xdata *)p;
		l2<<=8;
		checkcode+=l2;
	}

	return checkcode;
}

/********************************************/
/* 功能：处理累加和的溢出位函数				*/
/* 输入：32位累加和							*/
/* 输出：16位和校验值				        */
/********************************************/
U16 FU_Check_Code(U32 sum)
{
	U16 tem;
	//如果进位就需要加到尾部 
	while(sum>0xFFFF)
	{
		tem=sum>>16;//得到高位
		sum&=0xFFFF;//去掉高位
	    sum+=tem;//加到低位
	}
	tem=sum;
	return ~tem;
}

/*******************************************/
/* 功能：检测远程IP是否为局域网            */
/* 输入：无					               */
/* 输出：1为局域网，0为外网                */
/* 备注：涉及被呼叫远程主机的IP地址		   */
/*******************************************/
U8 Is_LAN(U8 Index)
{
	U32 SubnetMask,LocalIP,ReIP;
	SubnetMask=*(U32 *)FU_SubnetMask;//
	LocalIP=*(U32 *)FU_LoaclIP;//
	ReIP=*(U32 *)ReHost[Index].Re_IP;//

	if((LocalIP & SubnetMask)==(ReIP & SubnetMask))return 1;else return 0;
}

/********************************************/
/* 功能：网络运行时，处理各种状态           */
/* 输入：无                                 */
/* 输出：无                                 */
/********************************************/
void enc28j60_runtime()
{
	U8 i;
	#if NET_DHCP//如果允许DHCP功能
		if(ReHost[0].Stat==CLOSE)//如果DHCP已经连通
		{
			if(ReHost[0].OutTime!=Tim_Sec)//已过1秒
			{
				if(++DHCP_Time>=DHCP_TIME)
				{
					DHCP_Time=DHCP_TIME;//保存最大时间
					ReHost[0].Stat=DHCPGO;//重新DHCP
				}
				else ReHost[0].OutTime=Tim_Sec;
			}
		}
	#endif

	for(i=0;i<=FU_CONN;i++)
	{
		switch(ReHost[i].Stat)
		{
			case CONNECTED:break;//成功连接，什么都不用做，直接返回

			case CLOSE:		//关闭后，重新设置为就绪状态
				if(i>0)SetConnState(i);
			break;

			case DHCPGO:	//需要DHCP
			case DHCPOFFER:
			case DHCPREQUEST:
				DHCP();
			break;
	
			case DHCPOK:	//申请DHCP成功
				ReHost[0].Stat=CLOSE;
				DHCP_Time=0;//重新记时
			break;

			case ARPDNS://获取DNS的MAC
				ARP(i);
			break;

			case DNSGO://开始DNS
				DNS(i);
			break;

			case DNSOK://成功获取到IP地址
				ReHost[i].OutTime=60;
				ReHost[i].Stat=ARPGO;//不需要break;退出，马上处理ARP数据
			case ARPGO://如果需要ARP，则
				ARP(i);
			break;
	
			case ARPOK:		//ARP成功，则说明是工作站，服务器不需要ARP
				if(ReHost[i].Re_TYPE==UDP_CLIENT)//如果是UDP站，则转为连通状态
				{
					ReHost[i].Stat=CONNECTED;
					break;
				}
				else if(ReHost[i].Re_TYPE==TCP_CLIENT)	//和TCP服务器主动握手
				{
					ReHost[i].OutTime=60;
					ReHost[i].Stat=TCPGO;//不需要brack;马上处理Tcp连接
				}
			case TCPGO:
				TCP_Connect(i);
			break;
	
			case TCPOK://TCP连接成功
				ReHost[i].Stat=CONNECTED;
			break;

			case CLOSEMY://断开中
			case CLOSERE:
				CloseWait(i);
			break;

			case TCPWAIT://有时连Web服务器时，前一次服务器端没有正常断开，再连就连不进了，需要等待120秒
				if(ReHost[i].OutTime!=Tim_Sec)//每秒计时
				{
					ReHost[i].OutTime=Tim_Sec;
					if(ReHost[i].WaitTime)
					{
						ReHost[i].WaitTime--;
					}
					else ReHost[i].Stat=CLOSE;//重头开始
				}
			break;
		}
	}

	Data_Receive();	   		//接收数据
}
