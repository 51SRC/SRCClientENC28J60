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


/*******************************************/
/* ���ܣ���������                          */
/* ���룺��					               */
/* �������							       */
/* ��ע���������ݣ��Զ�ʶ�����UDP����TCP  */
/*******************************************/
void Send_Data(U8 Index,U16 len)
{
	if(Index>FU_CONN)return;//���Index���ڻ��棬��ֱ���˳�

	if(ReHost[Index].Re_TYPE & UDP)
	{
		if(ReHost[Index].Stat==CONNECTED)Send_UDP_Bag(Index,len);
	}
	else if(ReHost[Index].Re_TYPE & TCP)
	{
		if(ReHost[Index].Stat==CONNECTED)Send_TCP_Bag(Index,TCP_ACK|TCP_PSH,len);//���ݰ�(5)
	}
}

/*******************************************/
/* ���ܣ�����APR��            		       */
/* ���룺��					               */
/* �������							       */
/* ��ע���漰������Զ��������IP��ַ		   */
/*******************************************/
void Send_ARP_Bag(U8 Index)
{
	if(Is_LAN(Index))//����
	{
		//Ŀ��IP
		memcpy(&ARP_Data[24],ReHost[Index].Re_IP,4);
	}
	else//���������߱���IPΪ0.0.0.0
	{
		//Ŀ��IP
		memcpy(&ARP_Data[24],FU_GateWay,4);
	}

	//Ŀ��MAC��ַFF �㲥
	memcpy(EN_Head,FU_MACIP,6);

	ARP_Ask(1);//��ARP�� //1����	2Ӧ��

	EN_Head_Copy(1);//EN��̫��ͷ��1 ARP����

	ARP_Packet_Send();//����ARP��
}


/*******************************************/
/* ���ܣ�����APRӦ���            		   */
/* ���룺��					               */
/* �������							       */
/* ��ע���漰������Զ��������IP��ַ		   */
/*******************************************/
void Answer_ARP_Bag(void)
{
	//Ŀ��MAC
	memcpy(EN_Head,ReHost[0].Re_MAC,6);	//�̶�IndexΪ0����Ҫ�ѱ��Զ��MAC����

	ARP_Ask(2);//1��ARP����� 2Ӧ���

	EN_Head_Copy(1);//EN��̫��ͷ��1 ARP����
	ARP_Packet_Send();//����ARP��
}

/*******************************************/
/* ����: arp����IP�ҵ�mac��ַ              */
/* ���ܣ�����ARP                		   */
/* ���룺��  					           */
/* �����״ֵ̬	21�ɹ�				       */
/*******************************************/
void ARP(U8 Index)
{
	if(ReHost[Index].OutTime!=Tim_Sec)//ÿ��ֻ����1��arp���ݰ�
	{
		#if NET_DHCP//�Ƿ�̬��ȡIP
			if(ReHost[0].Stat==CLOSE)//�ѳɹ�DHCP
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
/* ����: ���������ҵ�IP                    */
/* ���ܣ�����DNS                		   */
/* ���룺��  					           */
/* �������                                */
/*******************************************/
void Send_DNS_Bag(U8 Index)
{
	U8 len;

	len=DNS_Request_Copy(Index);//������������

	//Ŀ��MAC��ַ
	memcpy(EN_Head,DNS_MAC,6);
	EN_Head_Copy(0);//EN��̫��ͷ��0 IP���� 1 ARP

	IP_Send(Index,3,28+len);//0 UDP,1 TCP,2 ICMP,3 DNS ��IPͷ,len

	UDP_Send(Index,1,len);//��UDP��

	UDP_Packet_Send(len);//����UDP��
}

/*******************************************/
/* ����: dns����������ΪIP��ַ             */
/* ���ܣ���������DNS��                     */
/* ���룺��  					           */
/* �����״ֵ̬	21�ɹ�				       */
/*******************************************/
void DNS(U8 Index)
{
	if(ReHost[Index].OutTime!=Tim_Sec)//ÿ��ֻ����1��arp���ݰ�
	{
		#if NET_DHCP	//�Ƿ�̬��ȡIP
			if(ReHost[0].Stat==CLOSE)//�ѳɹ�DHCP
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
/* ����: DHCP��4������,������û��DHCP������,��IP��Ϊ169.254.0.0,5����ѭ���㲥 */
/*       1.����Discove�㲥��,û�лظ�����1,2,4,8,16���ط�,��תʧ��          */
/*       2.�������ظ�OFFER,������IP��Ϣ,����ʱ������������������IP,ֻ���ݱ��� */
/*       3.���������Request,4.�ظ�ACK��,IP�ɹ����                           */
/* ���ܣ�����DHCP                		                                      */
/* ���룺Ŀ��IP					                                              */
/* �����״ֵ̬	21�ɹ�				                                          */
/******************************************************************************/
void DHCP(void)
{
	if(ReHost[0].Stat==DHCPGO && ReHost[0].OutTime!=Tim_Sec)	//1�뷢��һ��Discove��
	{
		ReHost[0].OutTime=Tim_Sec;//���¼�ʱ
		#if DEBUG
			Uart1_SendFlag(0,"DhcpGO");
		#endif
		DHCP_Send_Discover();	//���͹㲥��
	}
	else if(ReHost[0].Stat==DHCPOFFER)//�յ�Discover��
	{
		ReHost[0].Stat=DHCPREQUEST;
		ReHost[0].OutTime=Tim_Sec;
		#if DEBUG
			Uart1_SendFlag(0,"DhcpSendQUEST");
		#endif
		DHCP_Send_Request();		
	}
	else if(ReHost[0].Stat==DHCPREQUEST && ReHost[0].OutTime!=Tim_Sec)//1�뷢��һ�ΰ�,��ʾ��ʱ
	{
		ReHost[0].OutTime=Tim_Sec;
		#if DEBUG
			Uart1_SendFlag(0,"DhcpSendQUEST");
		#endif
		DHCP_Send_Request();		
	}
}

/********************************************/
/* ���ܣ��ȴ��Ͽ�����                       */
/* ���룺����ID��                           */
/* �������                                 */
/********************************************/
void CloseWait(U8 Index)
{
	if(ReHost[Index].OutTime!=Tim_Sec)//�õ��ش�
	{
		ReHost[Index].OutTime=Tim_Sec;
		if(ReHost[Index].WaitTime)ReHost[Index].WaitTime--;//2����û���յ��ر����ӵİ���ҲҪ�ر�
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
/* ���ܣ������Ͽ�����                       */
/* ���룺����ID��                           */
/* �������                                 */
/********************************************/
void CloseTCP(U8 Index)
{
	ReHost[Index].Stat=CLOSEMY;//�����Ͽ�
	ReHost[Index].WaitTime=2;//�ȴ��Ͽ���ʱ����
	#if DEBUG
		Uart1_SendFlag(Index,"MyClosing");
	#endif
	Send_TCP_Bag(Index,TCP_ACK|TCP_FIN,0);//���һ���������Ͽ�
}

/*******************************************/
/* ���ܣ�����TCP��            		       */
/* ���룺���ݳ���len	         	       */
/* �������							       */
/* ��ע���漰Զ��IP,Զ��MAC,Զ��,���ض˿ں�*/
/*******************************************/
void Send_TCP_Bag(U8 Index,U8 lx,U16 len)
{	
	//Ŀ��MAC
	memcpy(EN_Head,ReHost[Index].Re_MAC,6);
	EN_Head_Copy(0);//EN��̫��ͷ��0 IP���� 1 ARP

	TCP_Send(Index,lx,len);//��TCP�����ݳ���,Զ�̶˿�
	IP_Send(Index,1,40+len);//0 UDP,1 TCP,2 ICMP��IPͷ�� 20+20+len
	TCP_Packet_Send(len);//����TCP��
}

/*******************************************/
/* ���ܣ�����TCP����               		   */
/* ���룺��     					       */
/* �������							       */
/*******************************************/
void TCP_Connect(U8 Index)
{
	//�������ְ�(1)
	if(ReHost[Index].Stat==TCPGO && ReHost[Index].OutTime!=Tim_Sec)	//1�뷢��һ��Discove��
	{	
		#if DEBUG
			Uart1_SendFlag(Index,"TcpConn");
		#endif
		ReHost[Index].OutTime=Tim_Sec;
		Send_TCP_Bag(Index,TCP_SYN,0);//���ֵ�һ��(1)
	}
}

/*******************************************/
/* ���ܣ�����UDP��            		       */
/* ���룺���ݳ���	   		               */
/* �������							       */
/* ��ע���漰Զ��IP,Զ��MAC,Զ��,���ض˿ں�*/
/*******************************************/
void Send_UDP_Bag(U8 Index,U16 len)
{	
    //Ŀ��MAC��ַ
	memcpy(EN_Head,ReHost[Index].Re_MAC,6);
	EN_Head_Copy(0);//EN��̫��ͷ��0 IP���� 1 ARP

	IP_Send(Index,0,28+len);//0 UDP,1 TCP,2 ICMP,3 DNS��IPͷ,len

	UDP_Send(Index,0,len);//��UDP��

	UDP_Packet_Send(len);//����UDP��
}

///*******************************************/
///* ���ܣ�����ICMP_Ping����������ping       */
///* ���룺��					               */
///* �������							       */
///* ��ע���漰Զ��IP,Զ��MAC,Զ��,���ض˿ں�*/
///*******************************************/
//void Send_ICMP_Ping_Bag(void)
//{	
//	U8 i;
//	//Ŀ��IP
//    //Ŀ��MAC��ַ
//
//    if(Net_Stat>=ARPOK)//���ARPԶ��MAC�ɹ�
//	{
//		//Ŀ��MAC��ַ
//		for(i=0;i<6;i++)
//		{
//			EN_Head[i]=Re_MAC[i];
//		}
//		ICMP_Ping_Send();//��ICMP_Ping��
//		IP_Send(0,2,60);//0 UDP,1 TCP,2 ICMP��IPͷ
//		EN_Head_Copy(0);//EN��̫��ͷ��0 IP���� 1ARP
//		
//		ICMP_Ping_Packet_Send();//����ICMP_Ping��	
//	}
//}


/*******************************************/
/* ���ܣ�����ICMP_Ping�ذ�     		       */
/* ���룺��					               */
/* �������							       */
/* ��ע���漰Զ��IP,Զ��MAC,Զ��,���ض˿ں�*/
/*******************************************/
void Send_ICMP_Ping_Back_Bag(void)
{	
	//Ŀ��IP

    //Ŀ��MAC��ַ
	memcpy(EN_Head,ReHost[0].Re_MAC,6);
	EN_Head_Copy(0);//EN��̫��ͷ��0 IP���� 1ARP

	ICMP_Ping_Answer_Send();//��ICMP_Ping��
	IP_Send_Ping_Back(0,60);//��IPͷ
	
	ICMP_Ping_Packet_Send();//����ICMP_Ping��
}

