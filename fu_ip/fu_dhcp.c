#include "fu_dhcp.h"
#include <string.h>
#include "enc28j60.h"
#include "fu_config.h"
#include "fu_udp.h"
#include "fu_send.h"
#include "fu_receive.h"
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


//                      ���� 10M  ���� ���� xID1 xID2 xID3 xID4 SEC1 SEC2 �㲥 �㲥
U8 xdata DHCP_Data[34]={0x01,0x01,0x06,0x00,0x45,0x7C,0x33,0xF5,0x00,0x00,0x80,0x00};
U8 code DHCP_Zero[202]={0};

U8 code DHCP_Msg[24]={0x63,0x82,0x53,0x63,0x35,0x01,0x00,0x0c,0x0f,0x46,0x75,0x68,
0x6F,0x6D,0x65,0x2E,0x6E,0x65,0x74,0x2F,0x46,0x75,0x49,0x50};
U8 DHCP_Which=0;
U8 DHCP_Client[4]={0,0,0,0};
U8 DHCP_Server[4]={0,0,0,0};
U16 DHCP_Time=0;//DHCP��ʱ���ܣ�����ʱ���Զ���������

/*******************************************/
/* ���ܣ�1������discover��ͷ          	   */
/* ���룺��					               */
/* �������							       */
/* ��ע��                                  */
/*******************************************/
void DHCP_Head_Discover()
{
	DHCP_Which=1;//discover
	memcpy(&DHCP_Data[28],FU_MAC,6);//���Ʊ���MAC
}

/*******************************************/
/* ���ܣ�3������request��ͷ          	   */
/* ���룺��					               */
/* �������							       */
/* ��ע��                                  */
/*******************************************/

void DHCP_Head_Requsest()
{
  DHCP_Which=3;//request
  memcpy(&DHCP_Data[28],FU_MAC,6);//���Ʊ���MAC
}

/*******************************************/
/* ���ܣ�1������discover�����㲥       	   */
/* ���룺��					               */
/* �������							       */
/* ��ע��                                  */
/*******************************************/
void DHCP_Send_Discover()
{
	ReHost[0].Lc_PORT=68;//UDP���ض˿�
	ReHost[0].Re_PORT=67;//UDPԶ�̶˿�
	DHCP_Head_Discover();
	memcpy(DATA,DHCP_Data,34);//�����ݸ��Ƶ�������
	memcpy(&DATA[34],DHCP_Zero,202);//�����ݸ��Ƶ�������
	memcpy(&DATA[236],DHCP_Msg,24);//�����ݸ��Ƶ�������
	DATA[242]=DHCP_Which;
	DATA[260]=0xff;

	memcpy(ReHost[0].Re_MAC,FU_MACIP,6);//Ŀ��MAC
	ReHost[0].Re_IP[0]=ReHost[0].Re_IP[1]=ReHost[0].Re_IP[2]=ReHost[0].Re_IP[3]=0;//�˴�������0

	Send_UDP_Bag(0,261);//����UDP���ݰ�
}

/*******************************************/
/* ���ܣ�3������request��            	   */
/* ���룺IP					               */
/* �������							       */
/* ��ע��                                  */
/*******************************************/

void DHCP_Send_Request(void)
{
	ReHost[0].Lc_PORT=68;//UDP���ض˿�
	ReHost[0].Re_PORT=67;//UDPԶ�̶˿�
	DHCP_Head_Requsest();
	memcpy(DATA,DHCP_Data,34);//�����ݸ��Ƶ�������
	memcpy(&DATA[34],DHCP_Zero,202);//�����ݸ��Ƶ�������
	memcpy(&DATA[236],DHCP_Msg,24);//�����ݸ��Ƶ�������
	DATA[242]=DHCP_Which;
	DATA[260]=0x32;
	DATA[261]=0x04;
	memcpy(&DATA[262],DHCP_Client,4);//�����ݸ��Ƶ�������
	DATA[266]=0xff;

	memcpy(ReHost[0].Re_MAC,FU_MACIP,6);//Ŀ��MAC
	memcpy(ReHost[0].Re_IP,DHCP_Server,4);
	Send_UDP_Bag(0,267);//����UDP���ݰ�
}


