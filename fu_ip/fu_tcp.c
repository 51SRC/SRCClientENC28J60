#include "fu_TCP.h"
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


U8  xdata TCP_False[12]={0};//TCPα�ײ���12�ֽ�
U8  xdata TCP_Head[20]={0};//TCP�ײ�������20�ֽ�

/*******************************************/
/* ���ܣ�TCP α�ײ���� 12�ֽ�      	   */
/* ���룺TCP���ݰ�����				       */
/* �������							       */
/*******************************************/
void TCP_Flase_Copy(U8 Index,U16 len)//len Ϊż��
{
	memcpy(TCP_False,FU_LoaclIP,4);		//ԴIP
	
	memcpy(&TCP_False[4],ReHost[Index].Re_IP,4);	//Ŀ��IP

	TCP_False[8]=0;
	TCP_False[9]=6;//6 TCP
	TCP_False[10]=(20+len)>>8;//TCP�ײ���+���ݳ�
	TCP_False[11]=(20+len);//TCP�ײ���+���ݳ�
}

/*******************************************/
/* ���ܣ�TCP �ײ����  20�ֽ�       	   */
/* ���룺TCP���ݰ�����                     */
/* �������							       */
/*******************************************/
void TCP_Head_Copy(U8 Index,U8 Flag)
{
	TCP_Head[0]=ReHost[Index].Lc_PORT>>8;
	TCP_Head[1]=ReHost[Index].Lc_PORT;//Դ�˿�
	TCP_Head[2]=ReHost[Index].Re_PORT>>8;
	TCP_Head[3]=ReHost[Index].Re_PORT;//Ŀ�Ķ˿�
	memcpy(&TCP_Head[4],(U8 xdata *)&ReHost[Index].TCP_Mark,4);//TCP���
	memcpy(&TCP_Head[8],(U8 xdata *)&ReHost[Index].TCP_CMark,4);//ȷ�����
	TCP_Head[12]=0x50;//TCP�ײ���
	TCP_Head[13]=Flag;//TCP6��λ��־
	TCP_Head[14]=(FU_LEN-58)>>8;
	TCP_Head[15]=(FU_LEN-58);//TCP���ڴ�С FU_LEN-14-20-20-4=������յ�TCP����
	TCP_Head[16]=0;
	TCP_Head[17]=0;//�����
	TCP_Head[18]=0;
	TCP_Head[19]=0;//����ָ��
}

/********************************************/
/* ���ܣ�����ʹ�����           			*/
/* ���룺��									*/
/* �����16λTCP�����						*/
/********************************************/

U16 TCP_Check_Code(U16 len)
{
    U32 tem;

	/* �������� */
	tem=FU_Check_Sum((U16 xdata *)TCP_False,12);//TCPα�ײ�12
	tem+=FU_Check_Sum((U16 xdata *)TCP_Head,20);//TCP�ײ�20
	tem+=FU_Check_Sum((U16 xdata *)DATA,len);//TCP�ײ�20
    return FU_Check_Code(tem);//�������λ
}

/********************************************/
/* ���ܣ���TCP���ݰ�����           			*/
/* ���룺TCP ���ݳ��� len 		 			*/
/* �������									*/
/********************************************/
void TCP_Send(U8 Index,U8 lx,U16 len)
{
	U16 tem;
	//���TCPα�ײ� 12�ֽ�
	TCP_Flase_Copy(Index,len);//len Ϊż��
	//TCP �ײ����  20�ֽ�
	TCP_Head_Copy(Index,lx);
	
	tem=TCP_Check_Code(len);//�õ������
	
	TCP_Head[16]=tem>>8;
	TCP_Head[17]=tem;//������Ժ󲹳�
}

