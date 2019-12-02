#ifndef __fu_config_H__
#define __fu_config_H__
#include "mcuinit.h"
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

/*IP ��ز��� �궨������*/

#define DEBUG 0//�Ƿ���������Ϣ��������Ϣ����com1�ڷ�������
#define NET_DHCP 0//�Ƿ�����DHCP��̬��ȡIP����
#define DHCP_TIME 1800//DHCP��Լʱ�䣬1800�룬30����
#define NET_TCP  1//�Ƿ�����TCPͨ�ù���
#define NET_DNS	 1//�Ƿ�����DNS������������
#define FU_LEN 500//������������ݻ�����������������Լ���Ƭ�����ڴ�������������Ҫ��313�ֽڣ�������DHCP��̬��ȡIP��
                  //��Ȼ���ڴ�ʵ�ڲ�������ֵҪ���С��313������£�����ָ��IP��ַ��UDPͨ�Ż�TCP���ǿ��Ե�
#define FU_CONN	 2//����Զ����������������Խ�࣬ӰӦԽ������ռ���ڴ�ҲԽ�࣬ÿ����1����������Ҫ��ռ��18~46�ֽ��ڴ�

//������������
enum NET_TYPE
{
	AUTO,			//0���Զ������ڽ���PING
	UDP_CLIENT,		//1��UDP�ͻ��ˣ�������
	UDP_SERVER,		//2��UDP����ˣ��ӻ���
	TCP_CLIENT=4,	//TCP�ͻ��ˣ���������
	TCP_SERVER=8	//TCP����ˣ����ӻ���
};
#define UDP (UDP_CLIENT | UDP_SERVER)	//UDPЭ��
#define TCP (TCP_CLIENT | TCP_SERVER)	//TCPЭ��
#define CLIENT (UDP_CLIENT | TCP_CLIENT)//�ͻ���
#define SERVER (UDP_SERVER | TCP_SERVER)//�����

//��������״̬
enum NET_STAT
{
	UDPWAIT,		//UDP������ʱ��UDP�ȴ�
	CONNECTED,		//������
	CLOSEMY,		//�����Ͽ�
	CLOSERE,		//�����Ͽ�
	CLOSE,			//�ѹر�
  	DHCPGO,			//DHCP��ʼ
	DHCPOFFER,
	DHCPREQUEST,
	DHCPOK,			//DHCP�ɹ�
	ARPDNS,			//ARP��DNS
	DNSGO,			//��ʼDNS
	DNSOK,			//DNS�ɹ�
	ARPGO,			//ARP��ʼ
	ARPOK,			//ARP�ɹ�
	TCPGO,			//��ʼTCP����
	TCPLISTEN,		//����TCP����
	TCPACCEPT,		//TCP��������
	TCPOK,			//TCP���ӳɹ�
	TCPWAIT,		//�ȴ�����
};

#define FU_TTL 	255	//IP���ݱ���������

extern U8 code FU_MACIP[6];//�㲥��ַ
extern U8 xdata FU_LoaclIP[4];//����IP��ַ
extern U8 xdata FU_SubnetMask[4];//��������
extern U8 xdata FU_GateWay[4]; //Ĭ������
extern U8 xdata FU_MAC[6];//����MAC
extern U8 xdata DATA[FU_LEN];//�շ�����

struct _ReHost	//Զ�������ṹ
{
	enum NET_TYPE Re_TYPE;		//ʹ��Э�飬��TCP����UDP
	U8 Re_IP[4];	//Զ��IP
	U8 Re_MAC[6];	//Զ��MAC
	U16 Lc_PORT;	//���ض˿�
	U16 Re_PORT;	//Զ�̶˿�
	enum NET_STAT Stat;//����״̬
	U8 OutTime;		//״̬��ʱ
	U8 WaitTime;	//�ȴ�ʱ��
	#if NET_TCP
	U32 TCP_Mark;//ȷ��ID��
	U32 TCP_CMark;//�ظ�ID��
	#endif
	#if NET_DNS
	U8 Re_DNS[20];	//����
	#endif
};
extern xdata struct _ReHost ReHost[FU_CONN+1];//���ٶ���һ�����ʽṹ��ReHost[0]����DHCP��Ping��
//���Ӧ�ó�������ҪDHCP�����PING�Ĺ��ܣ���ҲҪ��ReHost[0]�ճ���

U32 FU_Check_Sum(U16 xdata *p,U16 len);
U16 FU_Check_Code(U32 sum);

/*******************************************/
/* ���ܣ������ʼ��                		   */
/* ���룺��					               */
/* �������   				               */
/*******************************************/
void net_init(void);

/*******************************************/
/* ���ܣ������������              		   */
/* ���룺���Ӻţ����ͣ��������˿�          */
/* �������   				               */
/*******************************************/
void Set_Re_HOST(U8 Index,U8 Type,U8 *p,U16 Port);

/********************************************/
/* ���ܣ���������״̬                       */
/* ���룺����ID��                           */
/* �������                                 */
/********************************************/
void SetConnState(U8 Index);

/*******************************************/
/* ���ܣ����Զ��IP�Ƿ�Ϊ������            */
/* ���룺��					               */
/* �����1Ϊ��������0Ϊ����                */
/* ��ע���漰������Զ��������IP��ַ		   */
/*******************************************/
U8 Is_LAN(U8 Index);

/********************************************/
/* ���ܣ���������ʱ                         */
/* ���룺��                           */
/* �������                                 */
/********************************************/
void enc28j60_runtime();

#endif 
