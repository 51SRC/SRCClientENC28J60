#include "fu_ip.h"
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


U8 xdata IP_Head[20]={0};//IP�ײ�20�ֽ�
U16 xdata IP_Mark=0x1200;//IP�ײ��ı�ʾ�ֶ�

/*******************************************/
/* ���ܣ����IP�ײ�20�ֽ�      		       */
/* ���룺lx:0 UDP,1 TCP,2 ICMP,3 DNS       */
/* ���룺IP���ݱ�����UDP20+8+len ICMP20+len*/
/* �������							       */
/*******************************************/
void IP_Head_Copy(U8 Index,U8 lx,U16 IP_len)
{
	IP_Head[0]=0x45;//�汾 �ײ�����
	IP_Head[1]=0;//TOS һ�����
	IP_Head[2]=IP_len>>8;
	IP_Head[3]=IP_len;//len=20+8+UDP_len,16λIP���ܳ���,20+40ICMP_Len ,20+20+TCP_Len
	IP_Mark++;//��ʾ�ֶ�+1��Ψһ�ֶ�
	if(IP_Mark==0xffff)IP_Mark=0x1200;	//���Ϊ���ֵ�����0x1200��ʼ���¼���
	IP_Head[4]=IP_Mark>>8;
	IP_Head[5]=IP_Mark;
	IP_Head[6]=0;
	IP_Head[7]=0;//��־��ƫ�ƣ���Ƭ
	IP_Head[8]=FU_TTL;//��������
	
	if(lx==1)
		IP_Head[9]=0x06;//6 TCP���ݰ�
	else if(lx==2)
		IP_Head[9]=0x01;//01 ICMP
	else
		IP_Head[9]=0x11;//17 UDP���ݰ�
	
	IP_Head[10]=0;
	IP_Head[11]=0;//�����
	
	memcpy(&IP_Head[12],FU_LoaclIP,4);		//ԴIP

	if(lx==3)
		memcpy(&IP_Head[16],FU_GateWay,4);//Ŀ��IP������IP
	else	
		memcpy(&IP_Head[16],ReHost[Index].Re_IP,4);//Ŀ��IP
}

/********************************************/
/* ���ܣ�IP�ײ� ����ʹ�����           	*/
/* ���룺��									*/
/* �����16λUDP�����						*/
/********************************************/
U16 IP_Check_Code()
{
    U32 tem;

	/* �������� */
	tem=FU_Check_Sum((U16 xdata *)IP_Head,20);//IPͷ20
    return FU_Check_Code(tem);//�������λ
}

/********************************************/
/* ���ܣ�IP�ײ����                      	*/
/* ���룺IP������ 20+8+len					*/
/* �������					 				*/
/********************************************/
void IP_Send(U8 Index,U8 lx,U16 IP_len)
{
	U16 tem;
	IP_Head_Copy(Index,lx,IP_len);
	tem=IP_Check_Code();//IP�ײ������
	
	IP_Head[10]=tem>>8;
	IP_Head[11]=tem;

}

/********************************************/
/* ���ܣ�IP�ײ��������Pingר�ã�          	*/
/* ���룺IP������ 20+8+len					*/
/* �������         						*/
/********************************************/
void IP_Send_Ping_Back(U8 Index,U16 IP_len)
{
	U16 tem;
	IP_Head_Copy(Index,2,IP_len);//0 UDP,1 TCP,2 ICMP
	IP_Head[4]=DATA[18];
	IP_Head[5]=DATA[19];//��PingҪ����Pingһ��
	tem=IP_Check_Code();//IP�ײ������
	
	IP_Head[10]=tem>>8;
	IP_Head[11]=tem;

}								   

