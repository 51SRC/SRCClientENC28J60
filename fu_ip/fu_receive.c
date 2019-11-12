#include "fu_receive.h"
#include "string.h"
#include "enc28j60.h"
#include "fu_config.h"
#include "fu_arp.h"
#include "fu_send.h"
#include "fu_icmp.h"
#include "fu_udp.h"
#include "fu_tcp.h"
#include "fu_app.h"
#include "fu_http.h"
#include "fu_dhcp.h"
#include "fu_dns.h"
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


/********************************************/
/* 功能：接收数据，存入缓冲区           	*/
/* 输入：无								    */
/* 输出：标志  								*/
/********************************************/
U8 Data_Receive(void)
{
	U16 len;
	U8 temp=0xFF;
	len=enc28j60PacketReceive(FU_LEN, DATA);	 //接收数据
	
	if(len>FU_LEN)return 0;	//接收数据出错

	// 判断数据包类型然后交由处理
	if(len>18)//确保更新包了才进行下列处理，18是最小数据包 14 以太网首部+4 CRC
	{ 
		if(DATA[13]==0x06)//ARP包
		{
	   		temp=ARP_Answer();//被问，对方回答
		}
		if(DATA[13]==0x00)//IP包
	    {
		    temp=IP_Answer(len);//3 ICMP  4 TCP  5UDP 
		}
	}
   
	return temp;  
}


/*******************************************/
/* 功能：处理ARP数据报      		       */
/* 输入：无					               */
/* 输出：0 本机ARP请求成功了 1应答别人     */
/*******************************************/
U8 ARP_Answer(void)
{
	U8 flag=0,i;
	//被问
	if(DATA[21]==1)
	{
		flag=memcmp(&DATA[38],FU_LoaclIP,4);//非0不是问本机IP
		
		//如果是问本机，则应答
		if(flag==0)
		{   
			memcpy(ReHost[0].Re_MAC,&DATA[6],6);//复制远程MAC放在ReHost[0]中临时使用

			memcpy(ReHost[0].Re_IP,&DATA[28],4);//复制远程IP
		
			Answer_ARP_Bag();//马上应答掉，防止ReHost[0]中的数据被别的通道修改
			
			return 0x20;//回答别人
		}
	}
	
	//得到对方回答
	if(DATA[21]==2)
	{
		flag=memcmp(&DATA[38],FU_LoaclIP,4);	//非0不是问本机IP
		
		//如果是回答给本机的，存起来MAC
		if(flag==0)
		{   
			if(0==memcmp(&DATA[28],FU_GateWay,4))//如果为网关IP
			{
				for(i=1;i<=FU_CONN;i++)
				{
					if((ReHost[i].Re_TYPE & CLIENT))//如果为客户端
					{
						if(ReHost[i].Stat==ARPGO)//且正在ARP，则改为ARPOK，多个连接是外网且IP不同时，只需要一次ARP
						{
							if(!Is_LAN(i))//如果是外网
							{
								#if DEBUG
									Uart1_SendFlag(i,"ArpWayOK");
								#endif
								memcpy(ReHost[i].Re_MAC,&DATA[6],6);//保存网关的MAC地址
								ReHost[i].Stat=ARPOK;	//ARP结束
							}
						}
						else if(ReHost[i].Stat==ARPDNS)
						{
							#if DEBUG
								Uart1_SendFlag(i,"ArpDnsOK");
							#endif
							memcpy(DNS_MAC,&DATA[6],6);//保存网关的MAC地址
							ReHost[i].OutTime=60;
							ReHost[i].Stat=DNSGO;//ARP结束
						}
					}
				}
			}
			else
			{
				for(i=1;i<=FU_CONN;i++)		//查找远程IP，找到Index
				{
					if(ReHost[i].Re_TYPE & CLIENT)//如果为客户端
					{
						if(ReHost[i].Stat==ARPGO)
						{
							//比较远程IP
							if(0==memcmp(&DATA[28],ReHost[i].Re_IP,4))//远程IP的MAC地址已经获取到
							{
								#if DEBUG
									Uart1_SendFlag(i,"ArpLanOK");
								#endif
								memcpy(ReHost[i].Re_MAC,&DATA[6],6);
								ReHost[i].Stat=ARPOK;	//ARP结束
							}
						}
					}
				}
			}

			return 0x21;//得到回答
		}
	}
	return 2;
}

/*******************************************/
/* 功能：读取TCP包中的ID   		           */
/* 输入：ID所在DATA的偏移地址 			   */
/* 输出：U32型的ID号,用于回复发送方        */
/*******************************************/
U32 GetID(U8 i)
{
	return *(U32 xdata *)&DATA[i];//读出序号
}

/*******************************************/
/* 功能：检测远程通信端口是否为本地端口    */
/* 输入：port为要检查的端口号              */
/* 输入：stat为当前状态，或为0不检测       */
/* 输出：返回通信ID号                      */
/*******************************************/
U8 CheckPort(U16 Lport,U16 Rport,U8 stat)
{
	U8 i;
	//检查本地端口是否为设置的端口
	for(i=1;i<=FU_CONN;i++)
	{
		if(ReHost[i].Re_TYPE>AUTO && (stat==0 || ReHost[i].Stat==stat))	//如果类型批配
		{
			if(ReHost[i].Lc_PORT==Lport && (Rport==0 || ReHost[i].Re_PORT==Rport))		//如果本地端口与远程端口都批配，说明已找到
			{
				break;
			}
		}
	}
	if(i>FU_CONN)i=0;
	return i;
}

/*******************************************/
/* 功能：处理IP数据报      		           */
/* 输入：DATA>54就是有数据的 			   */
/* 输出：3 ICMP 4 TCP 5 UDP	其他6          */
/*******************************************/
U8 IP_Answer(U16 len)
{
	U8 macFlag=0,ipFlag=0,id,i;
	U16 tRePort,tPort;
	
	if(memcmp(DATA,FU_MAC,6)==0)//本机MAC包
	{
		macFlag=1;
	}
	if(macFlag==0)	//如果不早本机MAC
	{
		if(memcmp(DATA,FU_MACIP,6)==0)	//如果为广播包，则需要接收
		{
			macFlag=2;
		}
	}
	
	if(memcmp(&DATA[30],FU_LoaclIP,4)==0)//是问本机IP
	{
		ipFlag=1;
	}
	
	//IP检验和暂不检验

	if(macFlag || ipFlag) 	//是本机包，或广播包
	{
		tRePort=(DATA[34]<<8)|DATA[35];//远程端口
		tPort=(DATA[36]<<8)|DATA[37];//本地端口
		memcpy(ReHost[0].Re_MAC,&DATA[6],6);	//复制远程MAC，有利于发送
		memcpy(ReHost[0].Re_IP,&DATA[26],4);	//复制远程IP

		//3 ICMP(仅识别ping请求) 
		if(0x01==DATA[23])
		{
			if(ipFlag==1 && macFlag==1 && DATA[34]==0x08)
			{
				//回应ping响应
				Send_ICMP_Ping_Back_Bag();
			}
			return 3;
		}

		//5 UDP 
		if(0x11==DATA[23])	////成功取到数据
		{
			/* 判断是不是DHCP包*/
			if(tRePort==67)//为DHCP包
			{
				len=(DATA[16]<<8)|DATA[17]+14;//总长度

				//检查MAC是否为本机，解决多个MAC28J60在同一个路由器下冲突的问题
				if(memcmp(FU_MAC,&DATA[70],6))return 0x71;
				
				#if DEBUG
					Uart1_SendStr("Flag\r\n");
				#endif

				//收到OFFER
				if(ReHost[0].Stat==DHCPGO && DATA[284]==2)
				{ 
					ReHost[0].Stat=DHCPOFFER;
					
					memcpy(DHCP_Server,&DATA[26],4);	//远程主机(网关)IP
					
					//有效数据从DATA[42]开始
					memcpy(DHCP_Client,&DATA[58],4);//分配给本机IP，放入临时变量中，接下来发送确认包，服务器才正式分配此IP
					
					#if DEBUG
						Uart1_SendStr("DhcpOFFER ");
						Uart1_SendIP(DATA[58],DATA[59],DATA[60],DATA[61]);
					#endif

					return 0x72;	
				}
				//收到ACK,表示成功申请到IP地址,可以正常使用
				if(ReHost[0].Stat=DHCPREQUEST && DATA[284]==5)
				{ 
				    ReHost[0].Stat=DHCPOK;
					
					memcpy(FU_GateWay,&DATA[26],4);	//远程主机(网关)IP
				
					//有效数据从DATA[42]开始
					memcpy(FU_LoaclIP,&DATA[58],4);	//分配给本机IP
					
					if(len>=309+4)memcpy(FU_SubnetMask,&DATA[309],4);//子网掩码

					//memcpy(FU_DNS1,&DATA[299],4);//DNS1
					//memcpy(FU_DNS2,&DATA[303],4);//DNS2
					#if DEBUG
						Uart1_SendStr("DhcpOK ");
						Uart1_SendIP(DATA[58],DATA[59],DATA[60],DATA[61]);
					#endif

					return 0x73;		
				}
				//收到NSK,表示服务器不能分配原来的IP,说明需要重新发送Discove包
				if(ReHost[0].Stat=DHCPREQUEST && DATA[284]==6)
				{
					ReHost[0].Stat=DHCPGO;		//重新申请DHCP
					#if DEBUG
						Uart1_SendFlag(0,"DhcpREQUEST");
					#endif
					return 0x74;
				}
			
			  	return 0x75;
			}//DHCP结束
			else if(tRePort==53)/* 是DNS包 */
			{
				for(i=1;i<=FU_CONN;i++)
				{
					if((ReHost[i].Re_TYPE & CLIENT))//如果为客户端
					{
						if(ReHost[i].Stat==DNSGO)//且正在ARP，则改为ARPOK，多个连接是外网且IP不同时，只需要一次ARP
						{
							if(0==memcmp(&ReHost[i].Re_DNS[1],&DATA[54],ReHost[i].Re_DNS[0]))//如果返回域名相同
							{
								len=(DATA[16]<<8)|DATA[17];//总长度
								len+=14;		   				//实际数据长度
								memcpy(ReHost[i].Re_IP,&DATA[len-4],4);//成功获取到远程IP，最后4位为远程IP地址
								ReHost[i].Stat=DNSOK;//DNS结束
								#if DEBUG
									Uart1_SendFlag(i,"DnsOK ");
									Uart1_SendIP(ReHost[i].Re_IP[0],ReHost[i].Re_IP[1],ReHost[i].Re_IP[2],ReHost[i].Re_IP[3]);
								#endif
								break;
							}
						}
					}
				}
			}//DNS包结束
			else
			{
				id=CheckPort(tPort,0,0);//检查本地端口，不检查远程端口
				if(id>0)
				{
					if(ReHost[id].Re_TYPE==UDP_SERVER)	//如果为UDP服务器
					{
						ReHost[id].Stat=CONNECTED;		//状态改为已连接
						ReHost[id].Re_PORT=tRePort;		//更改远程端口
						memcpy(ReHost[id].Re_MAC,&DATA[6],6);	//复制远程MAC，有利于发送
						memcpy(ReHost[id].Re_IP,&DATA[26],4);	//复制远程IP
					}
		
					len=(DATA[16]<<8)|DATA[17];		//总长度
					len-=28;		   				//实际数据长度
					App_Bag(id,&DATA[42],len);//处理数据做出动作
				}
				return 0x05;
			}
		}//UDP结束
		

		//4 TCP 
		if(0x06==DATA[23])		//TCP检验和，TCP_Port暂不检验
		{
			/* 处理TCP各个情况的过程 */
			//跟本机握手包(1)
			if(DATA[47]==TCP_SYN)
			{
				id=CheckPort(tPort,0,TCPLISTEN);	//检查所有连接中，状态为监听的连接，找到id>0
				if(id>0)			//有可用的监听连接供客户端建立握手包
				{
					ReHost[id].Stat=TCPACCEPT;		//改为接受状态

					ReHost[id].TCP_Mark=0x1200;//序号
					ReHost[id].TCP_CMark=GetID(38)+1;//确认序号+1
	
					ReHost[id].Re_PORT = tRePort;//远程端口
					memcpy(ReHost[id].Re_MAC,&DATA[6],6);	//复制远程MAC，有利于发送
					memcpy(ReHost[id].Re_IP,&DATA[26],4);	//复制远程IP

					//发送第2次回应人家的握手包
					Send_TCP_Bag(id,TCP_ACK|TCP_SYN,0);
				}
				return 0x41;
			}
			else
			{
				id=CheckPort(tPort,tRePort,0);	//检查端口，本地端口与远程端口都要批配
			}

			if(id>0)//如果已找到与本模块对接的连接号
			{
				//收到跟本机握手包的回应(3)
				if((ReHost[id].Stat==TCPACCEPT)&&(DATA[47]==TCP_ACK))
				{
					ReHost[id].TCP_Mark=GetID(42);//序号 
					ReHost[id].TCP_CMark=GetID(38);//确认序号照抄

					#if DEBUG
						Uart1_SendFlag(id,"TcpOK");
					#endif
					ReHost[id].Stat=TCPOK;

					//下面3点不用重新赋值了，因为前面已经赋值过了
					//ReHost[id].Re_PORT = tRePort;//远程端口
					//memcpy(ReHost[id].Re_MAC,&DATA[6],6);	//复制远程MAC，有利于发送
					//memcpy(ReHost[id].Re_IP,&DATA[26],4);	//复制远程IP
					
					return 0x43;
				}
				
	//			//空包，不与理会
	//			if(DATA[47]==TCP_URG)
	//			{
	//				Uart_Send_Byte('[');//串口发送状态量，用于调试
	//				Uart_Send_Byte('U');//串口发送状态量，用于调试
	//				Uart_Send_Byte('R');//串口发送状态量，用于调试
	//				Uart_Send_Byte('G');//串口发送状态量，用于调试
	//				Uart_Send_Byte(']');//串口发送状态量，用于调试
	//			}
				if(DATA[47]==(TCP_RST|TCP_ACK))		//包不能到达对方,主动tcp时会出现这种情况
				{
					ReHost[id].TCP_Mark=GetID(42);		//序号 
					ReHost[id].TCP_CMark=GetID(38)+1;//确认序号=+1
				}
					 
				//特殊情况对方没有断开,上次连接没断开,对方手工点了断开
				if((ReHost[id].Stat==TCPGO)&&(DATA[47]==TCP_ACK))	//如果为主动握手，但对方回的不是TCP_SYN|TCP_ACK
				{
					//主动握手的时候
					ReHost[id].TCP_Mark=GetID(42);		//序号 
					ReHost[id].TCP_CMark=GetID(38)+1;//确认序号=+1

					Send_TCP_Bag(id,TCP_ACK|TCP_FIN,0);//发送断开联接包
					ReHost[id].Stat=TCPWAIT;//上次未断开，等待服务器空闲下来再连
					ReHost[id].WaitTime=1;//空闲n秒，如果连web服务器出现这种情况，需要空闲120秒
					
					#if DEBUG
						Uart1_SendFlag(id,"TcpFin");
					#endif

					return 0x4A;//
				}
				   
	//			//主动握手包(1)
	//		 	TCP_Flag=TCP_SYN;
	//			Send_TCP_Bag(Index,0);//握手第一次(1)
	
				//收到回应本机握手包(2) 
				if(DATA[47] == (TCP_SYN|TCP_ACK))
				{
					
					ReHost[id].TCP_Mark=GetID(42);		//序号 
					ReHost[id].TCP_CMark=GetID(38)+1;//确认序号=+1
	
					Send_TCP_Bag(id,TCP_ACK,0);//握手第三次
	
					//发送第3次握手的包
					ReHost[id].Stat=TCPOK;
					#if DEBUG
						Uart1_SendFlag(id,"TcpOK");
					#endif

					return 0x42;
				} 
				
				//收到断开连接(6)
				if((DATA[47]&TCP_FIN)==TCP_FIN) 
				{
					ReHost[id].TCP_Mark=GetID(42);//序号 
					ReHost[id].TCP_CMark=GetID(38)+1;//确认序号
	
					if(ReHost[id].Stat==CLOSEMY)//本机主动断开的
					{
						ReHost[id].Stat=CLOSE;		//成功断开连接

						#if DEBUG
							Uart1_SendFlag(id,"MyClose");
						#endif
						Send_TCP_Bag(id,TCP_ACK,0);//发送断开握手的包
					}
					else//对方断开
					{
						if(ReHost[id].Stat!=TCPGO)	//正在连接时，如果对服务器没开，也会收到断开连接
						{
							ReHost[id].Stat=CLOSERE;
							ReHost[id].WaitTime=2;//等待断开超时秒数
						}
						Send_TCP_Bag(id,TCP_ACK|TCP_FIN,0);

						#if DEBUG
							Uart1_SendFlag(id,"RemoteClose");
						#endif
					}
					return 0x46;
				}
		  
				//重置连接(0)
				if((DATA[47]&TCP_RST)==TCP_RST)
				{
					ReHost[id].OutTime=60;
					ReHost[id].Stat=CLOSE;		//成功断开连接
					
					ReHost[id].TCP_Mark=GetID(42);//序号 
					ReHost[id].TCP_CMark=0;//确认序号

					#if DEBUG
						Uart1_SendFlag(id,"ReConn");
					#endif
					return 0x40;
				}
			
				//收到数据包(4)		//或收到发出的数据的回应(5)
				if((DATA[47]&TCP_PSH) || (DATA[47]&TCP_ACK))
				{
					if(ReHost[id].Stat==CLOSERE)//收到对方断开确认
					{
						ReHost[id].TCP_Mark=GetID(42);//序号 
						ReHost[id].TCP_CMark=GetID(38)+1;//确认序号=加字长
						ReHost[id].Stat=CLOSE;	
					}
					else
					{
						len=(DATA[16]<<8)|DATA[17];
						len-=40;
						ReHost[id].TCP_Mark=GetID(42);//序号 
						ReHost[id].TCP_CMark=GetID(38)+len;//确认序号=加字长

						//回复包可以不用单独发，直接发TCP_ACK|TCP_PSH，回复+数据一起发过去
						//但是，如果长时间没有回复过去，收到的数据就会重复，所以这里先回复一下
						if(ReHost[id].Stat!=CLOSEMY)Send_TCP_Bag(id,TCP_ACK,0);//发送回复包
	
						/* 如果收到的数据大于0 */
						if(len>0)App_Bag(id,&DATA[54],len);//处理数据做出动作
					}
					return 0x44;
				}
		
//				//或收到发出的数据的回应(5)
//				if((DATA[47]&TCP_ACK))		
//				{
//					
//					ReHost[id].TCP_Mark=GetID(42);//序号 
//					ReHost[id].TCP_CMark=GetID(38);//确认序号照抄
//					
//	//				if(TCP_Buf==0)//如果网页连接，且对方收到最后一包数据
//	//				{
//	//					TCP_Flag=(TCP_ACK|TCP_FIN);//最后一包，主动断开
//	//					ReHost[id].Stat==CLOSING;
//	//					Send_TCP_Bag(id,0);
//	//				}
//					
//	//				TCP_Send_Over(pSend,0);
//					
//					return 0x45;
//				}
			}  
		}//TCP结束

		return 0x04;//不是 TCP UDP ICMP_Ping
	}
	return 0x06;//不是给自己的包
}


/*******************************************/
/* 功能:固定数据如:网页数据发送完成与否函数*/
/* 输入：0 此处不需要加塞，N<255加塞       */
/* 输出：                                  */
/*******************************************/

//void TCP_Send_Over(char* p,U16 len)
//{	 
//	
//	if(len==0)//发送剩余数据
//	{
//		if(TCP_Buf==0)
//			return ;//直接退出
//		
//		TCP_Flag=(TCP_ACK|TCP_PSH);
//		
//		//如果长度不足，直接发
//		if(TCP_Buf<=254)
//		{
//			
//			memcpy(DATA,p+TCP_Haved,TCP_Buf);//复制数据到发送数组
//			Send_TCP_Bag(TCP_Buf);
//			
//			TCP_Haved+=TCP_Buf;
//			TCP_Buf=0;
//		}
//		else	//长度大于
//		{
//			memcpy(DATA,p+TCP_Haved,254);//复制数据到发送数组
//			Send_TCP_Bag(254);
//			
//			TCP_Haved+=254;
//			TCP_Buf-=254;
//		}
//	}
//	else
//	{ 
//		TCP_Buf=len;
//		TCP_Haved=0;
//		
//		TCP_Flag=(TCP_ACK|TCP_PSH);
//		
//		
//		//如果长度不足，直接发
//		if(len<=254)
//		{
//			
//			memcpy(DATA,p,len);//复制数据到发送数组
//			Send_TCP_Bag(len);
//			
//			TCP_Haved=len;
//			TCP_Buf=0;			 
//		}
//		else	//长度大于
//		{
//			memcpy(DATA,p,254);//复制数据到发送数组
//			Send_TCP_Bag(254);	
//			TCP_Haved+=254;
//			TCP_Buf=len-254;
//		}
//	}
//}
