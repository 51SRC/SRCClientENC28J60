#include "fu_udp.h"
#include <string.h>
#include "fu_config.h"
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


U8 xdata UDP_False[12]={0};//UDPα�ײ���12�ֽ�
U8 xdata UDP_Head[8]={0};//UDP�ײ�������8�ֽ�

/*******************************************/
/* ���ܣ�UDP α�ײ���� 12�ֽ�      	   */
/* ���룺UDP���ݰ�����				       */
/* �������							       */
/*******************************************/
void UDP_Flase_Copy(U8 Index,U8 lx,U16 len)//len Ϊż��
{
	memcpy(UDP_False,FU_LoaclIP,4);		//ԴIP

	if(lx)//��DNS��
		memcpy(&UDP_False[4],FU_GateWay,4);	//Ŀ��IP
	else
		memcpy(&UDP_False[4],ReHost[Index].Re_IP,4);	//Ŀ��IP
	
	UDP_False[8]=0;
	UDP_False[9]=17;
	UDP_False[10]=(8+len)>>8;	//UDP�ײ���+���ݳ�
	UDP_False[11]=(8+len);		//UDP�ײ���+���ݳ�
}

/*******************************************/
/* ���ܣ�UDP �ײ����  8�ֽ�       		   */
/* ���룺UDP���ݰ�����                     */
/* �������							       */
/*******************************************/
void UDP_Head_Copy(U8 Index,U8 lx,U16 len)
{
	UDP_Head[0]=ReHost[Index].Lc_PORT>>8;//Դ�˿�
	UDP_Head[1]=ReHost[Index].Lc_PORT;
	if(lx){//��DNS��
		UDP_Head[2]=0;//Ŀ�Ķ˿ڹ̶�Ϊ53
		UDP_Head[3]=53;
	}else{
		UDP_Head[2]=ReHost[Index].Re_PORT>>8;//Ŀ�Ķ˿�
		UDP_Head[3]=ReHost[Index].Re_PORT;
	}
	UDP_Head[4]=(len+8)>>8;
	UDP_Head[5]=(len+8);//UDP����=UDP�ײ�+����
	UDP_Head[6]=0;
	UDP_Head[7]=0;//������Ժ󲹳�
}

/********************************************/
/* ���ܣ�����ʹ�����           			*/
/* ���룺��									*/
/* �����16λUDP�����						*/
/********************************************/
U16 UDP_Check_Code(U16 len)
{
    U32 tem;

	/* �������� */
	tem=FU_Check_Sum((U16 xdata *)UDP_False,12);//UDPα�ײ�12
	tem+=FU_Check_Sum((U16 xdata *)UDP_Head,8);//UDP�ײ�8
	tem+=FU_Check_Sum((U16 xdata *)DATA,len);//���ݳ���
    return FU_Check_Code(tem);//�������λ
}

/********************************************/
/* ���ܣ���UDP���ݰ�����           			*/
/* ���룺lx 1ΪDNS��0ΪUDP     			    */
/* ���룺UDP ���ݳ��� len      			    */
/* �������									*/
/********************************************/
void UDP_Send(U8 Index,U8 lx,U16 len)
{
	U16 tem;
	//���UDPα�ײ� 12�ֽ�
	UDP_Flase_Copy(Index,lx,len);//len Ϊż��
	//UDP �ײ����  8�ֽ�
	UDP_Head_Copy(Index,lx,len);
	
	tem=UDP_Check_Code(len);//�õ������

	UDP_Head[6]=tem>>8;
	UDP_Head[7]=tem;//������Ժ󲹳�
}

