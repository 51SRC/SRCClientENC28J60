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
/* ���������ѧϰ�ο���δ�������������������κ���ҵ��;            */
/* FuIPרΪ��Ƭ�������Ƶ�TCP/IPЭ��ջ���������κ�8,16,32λ��Ƭ��     */
/* �����򣬿����ڿͻ��ˣ������    					            	 */
/* ������̳��www.fuhome.net/bbs/						             */
/* ��ֲ��ע�⣬�˳���ʹ��Ӳ��SPI��ģ��SPI�������𣬱�����Ĭ����ģ��  */
/* ���͸������ݱ�����ARP DHCP Ping TCP�������/�ͻ��ˣ� UDP        */
/* FuIP�����ڣ��ͻ�/���񣬲�������������ɽ���fuhome.net ƽ̨����Զ�̿���*/
/* �汾��v3.1 2018��7��23��						                     */
/* �����ܶ�	�˼�									       		     */
/* ���ڸ��£������ע�������ţ����������룬����	     			     */
/* Copyright fuhome.net δ��֮�� ʵ���ң��ÿƼ���������				 */
/*********************************************************************/


/********************************************/
/* ���ܣ��������ݣ����뻺����           	*/
/* ���룺��								    */
/* �������־  								*/
/********************************************/
U8 Data_Receive(void)
{
	U16 len;
	U8 temp=0xFF;
	len=enc28j60PacketReceive(FU_LEN, DATA);	 //��������
	
	if(len>FU_LEN)return 0;	//�������ݳ���

	// �ж����ݰ�����Ȼ���ɴ���
	if(len>18)//ȷ�����°��˲Ž������д���18����С���ݰ� 14 ��̫���ײ�+4 CRC
	{ 
		if(DATA[13]==0x06)//ARP��
		{
	   		temp=ARP_Answer();//���ʣ��Է��ش�
		}
		if(DATA[13]==0x00)//IP��
	    {
		    temp=IP_Answer(len);//3 ICMP  4 TCP  5UDP 
		}
	}
   
	return temp;  
}


/*******************************************/
/* ���ܣ�����ARP���ݱ�      		       */
/* ���룺��					               */
/* �����0 ����ARP����ɹ��� 1Ӧ�����     */
/*******************************************/
U8 ARP_Answer(void)
{
	U8 flag=0,i;
	//����
	if(DATA[21]==1)
	{
		flag=memcmp(&DATA[38],FU_LoaclIP,4);//��0�����ʱ���IP
		
		//������ʱ�������Ӧ��
		if(flag==0)
		{   
			memcpy(ReHost[0].Re_MAC,&DATA[6],6);//����Զ��MAC����ReHost[0]����ʱʹ��

			memcpy(ReHost[0].Re_IP,&DATA[28],4);//����Զ��IP
		
			Answer_ARP_Bag();//����Ӧ�������ֹReHost[0]�е����ݱ����ͨ���޸�
			
			return 0x20;//�ش����
		}
	}
	
	//�õ��Է��ش�
	if(DATA[21]==2)
	{
		flag=memcmp(&DATA[38],FU_LoaclIP,4);	//��0�����ʱ���IP
		
		//����ǻش�������ģ�������MAC
		if(flag==0)
		{   
			if(0==memcmp(&DATA[28],FU_GateWay,4))//���Ϊ����IP
			{
				for(i=1;i<=FU_CONN;i++)
				{
					if((ReHost[i].Re_TYPE & CLIENT))//���Ϊ�ͻ���
					{
						if(ReHost[i].Stat==ARPGO)//������ARP�����ΪARPOK�����������������IP��ͬʱ��ֻ��Ҫһ��ARP
						{
							if(!Is_LAN(i))//���������
							{
								#if DEBUG
									Uart1_SendFlag(i,"ArpWayOK");
								#endif
								memcpy(ReHost[i].Re_MAC,&DATA[6],6);//�������ص�MAC��ַ
								ReHost[i].Stat=ARPOK;	//ARP����
							}
						}
						else if(ReHost[i].Stat==ARPDNS)
						{
							#if DEBUG
								Uart1_SendFlag(i,"ArpDnsOK");
							#endif
							memcpy(DNS_MAC,&DATA[6],6);//�������ص�MAC��ַ
							ReHost[i].OutTime=60;
							ReHost[i].Stat=DNSGO;//ARP����
						}
					}
				}
			}
			else
			{
				for(i=1;i<=FU_CONN;i++)		//����Զ��IP���ҵ�Index
				{
					if(ReHost[i].Re_TYPE & CLIENT)//���Ϊ�ͻ���
					{
						if(ReHost[i].Stat==ARPGO)
						{
							//�Ƚ�Զ��IP
							if(0==memcmp(&DATA[28],ReHost[i].Re_IP,4))//Զ��IP��MAC��ַ�Ѿ���ȡ��
							{
								#if DEBUG
									Uart1_SendFlag(i,"ArpLanOK");
								#endif
								memcpy(ReHost[i].Re_MAC,&DATA[6],6);
								ReHost[i].Stat=ARPOK;	//ARP����
							}
						}
					}
				}
			}

			return 0x21;//�õ��ش�
		}
	}
	return 2;
}

/*******************************************/
/* ���ܣ���ȡTCP���е�ID   		           */
/* ���룺ID����DATA��ƫ�Ƶ�ַ 			   */
/* �����U32�͵�ID��,���ڻظ����ͷ�        */
/*******************************************/
U32 GetID(U8 i)
{
	return *(U32 xdata *)&DATA[i];//�������
}

/*******************************************/
/* ���ܣ����Զ��ͨ�Ŷ˿��Ƿ�Ϊ���ض˿�    */
/* ���룺portΪҪ���Ķ˿ں�              */
/* ���룺statΪ��ǰ״̬����Ϊ0�����       */
/* ���������ͨ��ID��                      */
/*******************************************/
U8 CheckPort(U16 Lport,U16 Rport,U8 stat)
{
	U8 i;
	//��鱾�ض˿��Ƿ�Ϊ���õĶ˿�
	for(i=1;i<=FU_CONN;i++)
	{
		if(ReHost[i].Re_TYPE>AUTO && (stat==0 || ReHost[i].Stat==stat))	//�����������
		{
			if(ReHost[i].Lc_PORT==Lport && (Rport==0 || ReHost[i].Re_PORT==Rport))		//������ض˿���Զ�̶˿ڶ����䣬˵�����ҵ�
			{
				break;
			}
		}
	}
	if(i>FU_CONN)i=0;
	return i;
}

/*******************************************/
/* ���ܣ�����IP���ݱ�      		           */
/* ���룺DATA>54���������ݵ� 			   */
/* �����3 ICMP 4 TCP 5 UDP	����6          */
/*******************************************/
U8 IP_Answer(U16 len)
{
	U8 macFlag=0,ipFlag=0,id,i;
	U16 tRePort,tPort;
	
	if(memcmp(DATA,FU_MAC,6)==0)//����MAC��
	{
		macFlag=1;
	}
	if(macFlag==0)	//������籾��MAC
	{
		if(memcmp(DATA,FU_MACIP,6)==0)	//���Ϊ�㲥��������Ҫ����
		{
			macFlag=2;
		}
	}
	
	if(memcmp(&DATA[30],FU_LoaclIP,4)==0)//���ʱ���IP
	{
		ipFlag=1;
	}
	
	//IP������ݲ�����

	if(macFlag || ipFlag) 	//�Ǳ���������㲥��
	{
		tRePort=(DATA[34]<<8)|DATA[35];//Զ�̶˿�
		tPort=(DATA[36]<<8)|DATA[37];//���ض˿�
		memcpy(ReHost[0].Re_MAC,&DATA[6],6);	//����Զ��MAC�������ڷ���
		memcpy(ReHost[0].Re_IP,&DATA[26],4);	//����Զ��IP

		//3 ICMP(��ʶ��ping����) 
		if(0x01==DATA[23])
		{
			if(ipFlag==1 && macFlag==1 && DATA[34]==0x08)
			{
				//��Ӧping��Ӧ
				Send_ICMP_Ping_Back_Bag();
			}
			return 3;
		}

		//5 UDP 
		if(0x11==DATA[23])	////�ɹ�ȡ������
		{
			/* �ж��ǲ���DHCP��*/
			if(tRePort==67)//ΪDHCP��
			{
				len=(DATA[16]<<8)|DATA[17]+14;//�ܳ���

				//���MAC�Ƿ�Ϊ������������MAC28J60��ͬһ��·�����³�ͻ������
				if(memcmp(FU_MAC,&DATA[70],6))return 0x71;
				
				#if DEBUG
					Uart1_SendStr("Flag\r\n");
				#endif

				//�յ�OFFER
				if(ReHost[0].Stat==DHCPGO && DATA[284]==2)
				{ 
					ReHost[0].Stat=DHCPOFFER;
					
					memcpy(DHCP_Server,&DATA[26],4);	//Զ������(����)IP
					
					//��Ч���ݴ�DATA[42]��ʼ
					memcpy(DHCP_Client,&DATA[58],4);//���������IP��������ʱ�����У�����������ȷ�ϰ�������������ʽ�����IP
					
					#if DEBUG
						Uart1_SendStr("DhcpOFFER ");
						Uart1_SendIP(DATA[58],DATA[59],DATA[60],DATA[61]);
					#endif

					return 0x72;	
				}
				//�յ�ACK,��ʾ�ɹ����뵽IP��ַ,��������ʹ��
				if(ReHost[0].Stat=DHCPREQUEST && DATA[284]==5)
				{ 
				    ReHost[0].Stat=DHCPOK;
					
					memcpy(FU_GateWay,&DATA[26],4);	//Զ������(����)IP
				
					//��Ч���ݴ�DATA[42]��ʼ
					memcpy(FU_LoaclIP,&DATA[58],4);	//���������IP
					
					if(len>=309+4)memcpy(FU_SubnetMask,&DATA[309],4);//��������

					//memcpy(FU_DNS1,&DATA[299],4);//DNS1
					//memcpy(FU_DNS2,&DATA[303],4);//DNS2
					#if DEBUG
						Uart1_SendStr("DhcpOK ");
						Uart1_SendIP(DATA[58],DATA[59],DATA[60],DATA[61]);
					#endif

					return 0x73;		
				}
				//�յ�NSK,��ʾ���������ܷ���ԭ����IP,˵����Ҫ���·���Discove��
				if(ReHost[0].Stat=DHCPREQUEST && DATA[284]==6)
				{
					ReHost[0].Stat=DHCPGO;		//��������DHCP
					#if DEBUG
						Uart1_SendFlag(0,"DhcpREQUEST");
					#endif
					return 0x74;
				}
			
			  	return 0x75;
			}//DHCP����
			else if(tRePort==53)/* ��DNS�� */
			{
				for(i=1;i<=FU_CONN;i++)
				{
					if((ReHost[i].Re_TYPE & CLIENT))//���Ϊ�ͻ���
					{
						if(ReHost[i].Stat==DNSGO)//������ARP�����ΪARPOK�����������������IP��ͬʱ��ֻ��Ҫһ��ARP
						{
							if(0==memcmp(&ReHost[i].Re_DNS[1],&DATA[54],ReHost[i].Re_DNS[0]))//�������������ͬ
							{
								len=(DATA[16]<<8)|DATA[17];//�ܳ���
								len+=14;		   				//ʵ�����ݳ���
								memcpy(ReHost[i].Re_IP,&DATA[len-4],4);//�ɹ���ȡ��Զ��IP�����4λΪԶ��IP��ַ
								ReHost[i].Stat=DNSOK;//DNS����
								#if DEBUG
									Uart1_SendFlag(i,"DnsOK ");
									Uart1_SendIP(ReHost[i].Re_IP[0],ReHost[i].Re_IP[1],ReHost[i].Re_IP[2],ReHost[i].Re_IP[3]);
								#endif
								break;
							}
						}
					}
				}
			}//DNS������
			else
			{
				id=CheckPort(tPort,0,0);//��鱾�ض˿ڣ������Զ�̶˿�
				if(id>0)
				{
					if(ReHost[id].Re_TYPE==UDP_SERVER)	//���ΪUDP������
					{
						ReHost[id].Stat=CONNECTED;		//״̬��Ϊ������
						ReHost[id].Re_PORT=tRePort;		//����Զ�̶˿�
						memcpy(ReHost[id].Re_MAC,&DATA[6],6);	//����Զ��MAC�������ڷ���
						memcpy(ReHost[id].Re_IP,&DATA[26],4);	//����Զ��IP
					}
		
					len=(DATA[16]<<8)|DATA[17];		//�ܳ���
					len-=28;		   				//ʵ�����ݳ���
					App_Bag(id,&DATA[42],len);//����������������
				}
				return 0x05;
			}
		}//UDP����
		

		//4 TCP 
		if(0x06==DATA[23])		//TCP����ͣ�TCP_Port�ݲ�����
		{
			/* ����TCP��������Ĺ��� */
			//���������ְ�(1)
			if(DATA[47]==TCP_SYN)
			{
				id=CheckPort(tPort,0,TCPLISTEN);	//������������У�״̬Ϊ���������ӣ��ҵ�id>0
				if(id>0)			//�п��õļ������ӹ��ͻ��˽������ְ�
				{
					ReHost[id].Stat=TCPACCEPT;		//��Ϊ����״̬

					ReHost[id].TCP_Mark=0x1200;//���
					ReHost[id].TCP_CMark=GetID(38)+1;//ȷ�����+1
	
					ReHost[id].Re_PORT = tRePort;//Զ�̶˿�
					memcpy(ReHost[id].Re_MAC,&DATA[6],6);	//����Զ��MAC�������ڷ���
					memcpy(ReHost[id].Re_IP,&DATA[26],4);	//����Զ��IP

					//���͵�2�λ�Ӧ�˼ҵ����ְ�
					Send_TCP_Bag(id,TCP_ACK|TCP_SYN,0);
				}
				return 0x41;
			}
			else
			{
				id=CheckPort(tPort,tRePort,0);	//���˿ڣ����ض˿���Զ�̶˿ڶ�Ҫ����
			}

			if(id>0)//������ҵ��뱾ģ��Խӵ����Ӻ�
			{
				//�յ����������ְ��Ļ�Ӧ(3)
				if((ReHost[id].Stat==TCPACCEPT)&&(DATA[47]==TCP_ACK))
				{
					ReHost[id].TCP_Mark=GetID(42);//��� 
					ReHost[id].TCP_CMark=GetID(38);//ȷ������ճ�

					#if DEBUG
						Uart1_SendFlag(id,"TcpOK");
					#endif
					ReHost[id].Stat=TCPOK;

					//����3�㲻�����¸�ֵ�ˣ���Ϊǰ���Ѿ���ֵ����
					//ReHost[id].Re_PORT = tRePort;//Զ�̶˿�
					//memcpy(ReHost[id].Re_MAC,&DATA[6],6);	//����Զ��MAC�������ڷ���
					//memcpy(ReHost[id].Re_IP,&DATA[26],4);	//����Զ��IP
					
					return 0x43;
				}
				
	//			//�հ����������
	//			if(DATA[47]==TCP_URG)
	//			{
	//				Uart_Send_Byte('[');//���ڷ���״̬�������ڵ���
	//				Uart_Send_Byte('U');//���ڷ���״̬�������ڵ���
	//				Uart_Send_Byte('R');//���ڷ���״̬�������ڵ���
	//				Uart_Send_Byte('G');//���ڷ���״̬�������ڵ���
	//				Uart_Send_Byte(']');//���ڷ���״̬�������ڵ���
	//			}
				if(DATA[47]==(TCP_RST|TCP_ACK))		//�����ܵ���Է�,����tcpʱ������������
				{
					ReHost[id].TCP_Mark=GetID(42);		//��� 
					ReHost[id].TCP_CMark=GetID(38)+1;//ȷ�����=+1
				}
					 
				//��������Է�û�жϿ�,�ϴ�����û�Ͽ�,�Է��ֹ����˶Ͽ�
				if((ReHost[id].Stat==TCPGO)&&(DATA[47]==TCP_ACK))	//���Ϊ�������֣����Է��صĲ���TCP_SYN|TCP_ACK
				{
					//�������ֵ�ʱ��
					ReHost[id].TCP_Mark=GetID(42);		//��� 
					ReHost[id].TCP_CMark=GetID(38)+1;//ȷ�����=+1

					Send_TCP_Bag(id,TCP_ACK|TCP_FIN,0);//���ͶϿ����Ӱ�
					ReHost[id].Stat=TCPWAIT;//�ϴ�δ�Ͽ����ȴ�������������������
					ReHost[id].WaitTime=1;//����n�룬�����web���������������������Ҫ����120��
					
					#if DEBUG
						Uart1_SendFlag(id,"TcpFin");
					#endif

					return 0x4A;//
				}
				   
	//			//�������ְ�(1)
	//		 	TCP_Flag=TCP_SYN;
	//			Send_TCP_Bag(Index,0);//���ֵ�һ��(1)
	
				//�յ���Ӧ�������ְ�(2) 
				if(DATA[47] == (TCP_SYN|TCP_ACK))
				{
					
					ReHost[id].TCP_Mark=GetID(42);		//��� 
					ReHost[id].TCP_CMark=GetID(38)+1;//ȷ�����=+1
	
					Send_TCP_Bag(id,TCP_ACK,0);//���ֵ�����
	
					//���͵�3�����ֵİ�
					ReHost[id].Stat=TCPOK;
					#if DEBUG
						Uart1_SendFlag(id,"TcpOK");
					#endif

					return 0x42;
				} 
				
				//�յ��Ͽ�����(6)
				if((DATA[47]&TCP_FIN)==TCP_FIN) 
				{
					ReHost[id].TCP_Mark=GetID(42);//��� 
					ReHost[id].TCP_CMark=GetID(38)+1;//ȷ�����
	
					if(ReHost[id].Stat==CLOSEMY)//���������Ͽ���
					{
						ReHost[id].Stat=CLOSE;		//�ɹ��Ͽ�����

						#if DEBUG
							Uart1_SendFlag(id,"MyClose");
						#endif
						Send_TCP_Bag(id,TCP_ACK,0);//���ͶϿ����ֵİ�
					}
					else//�Է��Ͽ�
					{
						if(ReHost[id].Stat!=TCPGO)	//��������ʱ������Է�����û����Ҳ���յ��Ͽ�����
						{
							ReHost[id].Stat=CLOSERE;
							ReHost[id].WaitTime=2;//�ȴ��Ͽ���ʱ����
						}
						Send_TCP_Bag(id,TCP_ACK|TCP_FIN,0);

						#if DEBUG
							Uart1_SendFlag(id,"RemoteClose");
						#endif
					}
					return 0x46;
				}
		  
				//��������(0)
				if((DATA[47]&TCP_RST)==TCP_RST)
				{
					ReHost[id].OutTime=60;
					ReHost[id].Stat=CLOSE;		//�ɹ��Ͽ�����
					
					ReHost[id].TCP_Mark=GetID(42);//��� 
					ReHost[id].TCP_CMark=0;//ȷ�����

					#if DEBUG
						Uart1_SendFlag(id,"ReConn");
					#endif
					return 0x40;
				}
			
				//�յ����ݰ�(4)		//���յ����������ݵĻ�Ӧ(5)
				if((DATA[47]&TCP_PSH) || (DATA[47]&TCP_ACK))
				{
					if(ReHost[id].Stat==CLOSERE)//�յ��Է��Ͽ�ȷ��
					{
						ReHost[id].TCP_Mark=GetID(42);//��� 
						ReHost[id].TCP_CMark=GetID(38)+1;//ȷ�����=���ֳ�
						ReHost[id].Stat=CLOSE;	
					}
					else
					{
						len=(DATA[16]<<8)|DATA[17];
						len-=40;
						ReHost[id].TCP_Mark=GetID(42);//��� 
						ReHost[id].TCP_CMark=GetID(38)+len;//ȷ�����=���ֳ�

						//�ظ������Բ��õ�������ֱ�ӷ�TCP_ACK|TCP_PSH���ظ�+����һ�𷢹�ȥ
						//���ǣ������ʱ��û�лظ���ȥ���յ������ݾͻ��ظ������������Ȼظ�һ��
						if(ReHost[id].Stat!=CLOSEMY)Send_TCP_Bag(id,TCP_ACK,0);//���ͻظ���
	
						/* ����յ������ݴ���0 */
						if(len>0)App_Bag(id,&DATA[54],len);//����������������
					}
					return 0x44;
				}
		
//				//���յ����������ݵĻ�Ӧ(5)
//				if((DATA[47]&TCP_ACK))		
//				{
//					
//					ReHost[id].TCP_Mark=GetID(42);//��� 
//					ReHost[id].TCP_CMark=GetID(38);//ȷ������ճ�
//					
//	//				if(TCP_Buf==0)//�����ҳ���ӣ��ҶԷ��յ����һ������
//	//				{
//	//					TCP_Flag=(TCP_ACK|TCP_FIN);//���һ���������Ͽ�
//	//					ReHost[id].Stat==CLOSING;
//	//					Send_TCP_Bag(id,0);
//	//				}
//					
//	//				TCP_Send_Over(pSend,0);
//					
//					return 0x45;
//				}
			}  
		}//TCP����

		return 0x04;//���� TCP UDP ICMP_Ping
	}
	return 0x06;//���Ǹ��Լ��İ�
}


/*******************************************/
/* ����:�̶�������:��ҳ���ݷ�����������*/
/* ���룺0 �˴�����Ҫ������N<255����       */
/* �����                                  */
/*******************************************/

//void TCP_Send_Over(char* p,U16 len)
//{	 
//	
//	if(len==0)//����ʣ������
//	{
//		if(TCP_Buf==0)
//			return ;//ֱ���˳�
//		
//		TCP_Flag=(TCP_ACK|TCP_PSH);
//		
//		//������Ȳ��㣬ֱ�ӷ�
//		if(TCP_Buf<=254)
//		{
//			
//			memcpy(DATA,p+TCP_Haved,TCP_Buf);//�������ݵ���������
//			Send_TCP_Bag(TCP_Buf);
//			
//			TCP_Haved+=TCP_Buf;
//			TCP_Buf=0;
//		}
//		else	//���ȴ���
//		{
//			memcpy(DATA,p+TCP_Haved,254);//�������ݵ���������
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
//		//������Ȳ��㣬ֱ�ӷ�
//		if(len<=254)
//		{
//			
//			memcpy(DATA,p,len);//�������ݵ���������
//			Send_TCP_Bag(len);
//			
//			TCP_Haved=len;
//			TCP_Buf=0;			 
//		}
//		else	//���ȴ���
//		{
//			memcpy(DATA,p,254);//�������ݵ���������
//			Send_TCP_Bag(254);	
//			TCP_Haved+=254;
//			TCP_Buf=len-254;
//		}
//	}
//}
