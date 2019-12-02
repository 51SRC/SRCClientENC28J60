#include "fu_config.h"
#include <string.h>
#include "fu_send.h"
#include "fu_receive.h"
#include "fu_dns.h"
#include "fu_dhcp.h"
#include "timer.h"
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


xdata struct _ReHost ReHost[FU_CONN+1];//ͨ��������

/* ��ز�������,ָ��IP */
U8 code FU_MACIP[6]={255,255,255,255,0xff,0xff};//�㲥��ַ
U8 xdata FU_MAC[6]={0x00,0x12,0X34,0X56,0X78,0X90};//����MAC

#if NET_DHCP
	/* ��ز�������,�Զ���ȡIP */
	U8 xdata FU_LoaclIP[4]={0,0,0,0};//����IP��ַ
	U8 xdata FU_SubnetMask[4]={255,255,255,0};//��������
	U8 xdata FU_GateWay[4]={255,255,255,255}; //Ĭ������
#else
	U8 xdata FU_LoaclIP[4]={192,168,0,101};//����IP��ַ133/101
	U8 xdata FU_SubnetMask[4]={255,255,255,0};//��������
	U8 xdata FU_GateWay[4]={192,168,0,1}; //Ĭ������
#endif

U8 xdata DATA[FU_LEN]={0};//�շ����ݻ�����

/*******************************************/
/* ���ܣ������ʼ��                		   */
/* ���룺��					               */
/* �������   				               */
/*******************************************/
void net_init(void)
{
	U8 i;
	
	Set_Re_HOST(0,AUTO,"255.255.255.255",0);	//Re_HOST[0]������Ӧping

	if(NET_DHCP)			//�����ҪDHCP�����һ���϶���DHCP
	{
		ReHost[0].Stat=DHCPGO;
	}
	else ReHost[0].Stat=CLOSE;
	
	for(i=1;i<=FU_CONN;i++)		//��ʼ����������ΪĬ��ֵ
	{
		ReHost[i].Re_TYPE=AUTO;	//��ʼ����ΪAUTO
		ReHost[i].Stat=CLOSE;  	//��ʼ״̬ΪCLOSE
	}

	//��ʼ��FUIP��ʹ�ó�����Ҫ��FU_CONN���ʹ�ã���Ҫ����FU_CONNָ����������
	Set_Re_HOST(1,TCP_SERVER,"0.0.0.0",80);//Web����������������IP���Ժ�汾���ϴ˹���
	//Set_Re_HOST(2,TCP_CLIENT,"192.168.0.106",4001);
	Set_Re_HOST(2,TCP_CLIENT,"47.104.19.111",4001);//47.104.19.111
//	Set_Re_HOST(2,UDP_CLIENT,"0.0.0.0",123);	//Զ��NTP������
//	Set_Re_HOST(2,TCP_SERVER,"0.0.0.0",8080);
//	Set_Re_HOST(3,TCP_SERVER,"0.0.0.0",8080);	//����TCP�������ı��ض˿���ͬ����ʾ����ͬʱ���������ͻ��ˣ���д���б�ʾ�������Ŀͻ���
//	Set_Re_HOST(4,UDP_CLIENT,"192.168.0.123",8080);
//	Set_Re_HOST(5,UDP_SERVER,"0.0.0.0",8080);
}


/*******************************************/
/* ���ܣ���ȡ���ض˿ڣ��Զ���1����         */
/* ���룺��                                */
/* �����δ����������ռ�õı��ض˿�        */
/*******************************************/
U16 GetLocalPort()
{
	static U16 tPort=0;
	U16 i;
	if(++tPort>20000)//0~19999֮��仯
	{
		tPort=0;		//�����һ��IP�Ѿ��������
		while(1)
		{
			for(i=1;i<=FU_CONN;i++)	//����µ�IP�Ƿ��Ѿ�����
			{
				if(ReHost[i].Lc_PORT==tPort+10000)break;	//�Ѵ��ڣ���i<=FU_CONN
			}
			if(i>FU_CONN)break;	//�����ڣ���ֱ���˳�
			else tPort++;		//���ڣ����1�����˳�
		}
	}
	return 10000+tPort;
}

/********************************************/
/* ���ܣ���������״̬                       */
/* ���룺����ID��                           */
/* �������                                 */
/********************************************/
void SetConnState(U8 Index)
{
	switch(ReHost[Index].Re_TYPE)
	{
		case UDP_CLIENT:
		case TCP_CLIENT:
			if(0==ReHost[Index].Re_IP[0])//���Զ������������������IP
			{
				ReHost[Index].Stat=ARPDNS;//��ʼDNS��ARP
			}
			else{
				ReHost[Index].Stat=ARPGO;//Ϊ�ͻ��ˣ�����ҪARP
			}
		break;

		case UDP_SERVER://Ϊ����������ȴ�
			ReHost[Index].Stat=UDPWAIT;
		break;

		case TCP_SERVER://ΪTCP�������������
			ReHost[Index].Stat=TCPLISTEN;
		break;
	}
}

/********************************************/
/* ���ܣ����������ַ����������Ƶ�.Re_DNS  */
/* ���룺����ID�ţ������ַ�������Ϊ������IP */
/* �������                                 */
/********************************************/
void DnsToIP(U8 Index,U8 *p)
{
	U8 *s=p;
	U8 d=0;
	U8 ip[4]={0,0,0,0};
	while(*s)//�����ַ���
	{
		if(*s=='.')//���Ϊ'.'������+1
		{
			d++;
		}
		else
		{
			if(*s>='0' && *s<='9')//���ÿ1λ�ϣ���Ϊ0~9������
			{
				ip[d]=ip[d]*10+(*s-'0');
			}
			else//����IP��ַ����������
			{
				DNS_Copy(Index,p);//���������Ƶ���Ӧ�Ļ����У�ע���������Ȳ�Ҫ�����������ĳ���-3
				ReHost[Index].Re_IP[0]=0;//Ŀ��IP1
				ReHost[Index].Re_IP[1]=0;//Ŀ��IP2
				ReHost[Index].Re_IP[2]=0;//Ŀ��IP3
				ReHost[Index].Re_IP[3]=0;//Ŀ��IP4
				return;//�˳�
			}
		}
		s++;
	}

	//IP��ַ��ȷ
	ReHost[Index].Re_IP[0]=ip[0];//Ŀ��IP1
	ReHost[Index].Re_IP[1]=ip[1];//Ŀ��IP2
	ReHost[Index].Re_IP[2]=ip[2];//Ŀ��IP3
	ReHost[Index].Re_IP[3]=ip[3];//Ŀ��IP4
	ReHost[Index].Re_DNS[0]=0;
}

/*******************************************/
/* ���ܣ�����Զ������ͨ�Ų���              */
/* ���룺Indexͨ��ID��                     */
/*       Type:��ID��ΪTCP����UDP           */
/*       p:����"fuhome.net"��192.168.0.4�� */
/*       Port:Զ�̻򱾵ض˿�               */
/* �������							       */
/*******************************************/
void Set_Re_HOST(U8 Index,U8 Type,U8 *p,U16 Port)
{
	ReHost[Index].TCP_Mark=ReHost[Index].TCP_CMark=0;
	ReHost[Index].OutTime=60;//��ʱʱ��
	ReHost[Index].Re_TYPE=Type;//��������
	DnsToIP(Index,p);//��ֵIP��ַ

	if(Type & CLIENT) 		//���Ϊ�ͻ��ˣ���PortΪԶ�̶˿ڣ����ض˿ڲ����ظ�������ظ�������Ҳ���ͨ��ID
	{
		ReHost[Index].Lc_PORT=GetLocalPort();	//ȡ10000���ϲ��ظ��˿�
		ReHost[Index].Re_PORT=Port;		//Զ�̶˿�
	}
	
	if(Type & SERVER)		//���Ϊ����������Զ�̶˿�Ϊ0���ȴ�����ʱ���ٸ���Զ�̶˿ں�
	{
		ReHost[Index].Lc_PORT=Port;		//���ض˿�
		ReHost[Index].Re_PORT=0;		//Զ�̶˿�
	}

	SetConnState(Index);//���ó�ʼͨ��״̬
}

/********************************************/
/* ���ܣ�������ۼӺ���           			*/
/* ���룺�ַ������볤��						*/
/* �����32λ�ۼӺ�					        */
/********************************************/
U32 FU_Check_Sum(U16 xdata *p,U16 len)
{
//���ǵ���Щ��Ƭ��û��CY�Ĵ���,������ֲ,�̻���ѡ����δ���
	U8 i;
	U16 l2=len>>1;//����2
	U32 checkcode=0;//�����

	//�������� 
	if(len==0)return 0;//����Ϊ0ֱ�ӷ���0

	for(i=0;i<l2;i++)//ѭ���ܳ��ȳ���2��,���Ϊ����,����ѭ��һ��
	{
		checkcode+=*p++;
	}
	if((len&0x01)==0x01)//�������Ϊ���������߲�0
	{
		l2=*(U8 xdata *)p;
		l2<<=8;
		checkcode+=l2;
	}

	return checkcode;
}

/********************************************/
/* ���ܣ������ۼӺ͵����λ����				*/
/* ���룺32λ�ۼӺ�							*/
/* �����16λ��У��ֵ				        */
/********************************************/
U16 FU_Check_Code(U32 sum)
{
	U16 tem;
	//�����λ����Ҫ�ӵ�β�� 
	while(sum>0xFFFF)
	{
		tem=sum>>16;//�õ���λ
		sum&=0xFFFF;//ȥ����λ
	    sum+=tem;//�ӵ���λ
	}
	tem=sum;
	return ~tem;
}

/*******************************************/
/* ���ܣ����Զ��IP�Ƿ�Ϊ������            */
/* ���룺��					               */
/* �����1Ϊ��������0Ϊ����                */
/* ��ע���漰������Զ��������IP��ַ		   */
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
/* ���ܣ���������ʱ���������״̬           */
/* ���룺��                                 */
/* �������                                 */
/********************************************/
void enc28j60_runtime()
{
	U8 i;
	#if NET_DHCP//�������DHCP����
		if(ReHost[0].Stat==CLOSE)//���DHCP�Ѿ���ͨ
		{
			if(ReHost[0].OutTime!=Tim_Sec)//�ѹ�1��
			{
				if(++DHCP_Time>=DHCP_TIME)
				{
					DHCP_Time=DHCP_TIME;//�������ʱ��
					ReHost[0].Stat=DHCPGO;//����DHCP
				}
				else ReHost[0].OutTime=Tim_Sec;
			}
		}
	#endif

	for(i=0;i<=FU_CONN;i++)
	{
		switch(ReHost[i].Stat)
		{
			case CONNECTED:break;//�ɹ����ӣ�ʲô����������ֱ�ӷ���

			case CLOSE:		//�رպ���������Ϊ����״̬
				if(i>0)SetConnState(i);
			break;

			case DHCPGO:	//��ҪDHCP
			case DHCPOFFER:
			case DHCPREQUEST:
				DHCP();
			break;
	
			case DHCPOK:	//����DHCP�ɹ�
				ReHost[0].Stat=CLOSE;
				DHCP_Time=0;//���¼�ʱ
			break;

			case ARPDNS://��ȡDNS��MAC
				ARP(i);
			break;

			case DNSGO://��ʼDNS
				DNS(i);
			break;

			case DNSOK://�ɹ���ȡ��IP��ַ
				ReHost[i].OutTime=60;
				ReHost[i].Stat=ARPGO;//����Ҫbreak;�˳������ϴ���ARP����
			case ARPGO://�����ҪARP����
				ARP(i);
			break;
	
			case ARPOK:		//ARP�ɹ�����˵���ǹ���վ������������ҪARP
				if(ReHost[i].Re_TYPE==UDP_CLIENT)//�����UDPվ����תΪ��ͨ״̬
				{
					ReHost[i].Stat=CONNECTED;
					break;
				}
				else if(ReHost[i].Re_TYPE==TCP_CLIENT)	//��TCP��������������
				{
					ReHost[i].OutTime=60;
					ReHost[i].Stat=TCPGO;//����Ҫbrack;���ϴ���Tcp����
				}
			case TCPGO:
				TCP_Connect(i);
			break;
	
			case TCPOK://TCP���ӳɹ�
				ReHost[i].Stat=CONNECTED;
			break;

			case CLOSEMY://�Ͽ���
			case CLOSERE:
				CloseWait(i);
			break;

			case TCPWAIT://��ʱ��Web������ʱ��ǰһ�η�������û�������Ͽ����������������ˣ���Ҫ�ȴ�120��
				if(ReHost[i].OutTime!=Tim_Sec)//ÿ���ʱ
				{
					ReHost[i].OutTime=Tim_Sec;
					if(ReHost[i].WaitTime)
					{
						ReHost[i].WaitTime--;
					}
					else ReHost[i].Stat=CLOSE;//��ͷ��ʼ
				}
			break;
		}
	}

	Data_Receive();	   		//��������
}
