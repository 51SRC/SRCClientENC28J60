#include "mcuinit.h"
#include "timer.h"
#include "enc28j60.h"
#include "fu_send.h"
#include "fu_config.h"
#include "fu_receive.h"
#include "fu_app.h"
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

//���ж˿ڳ�ʼ����Ϊͨ��IO
void GPIOInit()
{
	#if MCU_TYPE==STC15W
		P0M0=0x00;
		P0M1=0x00;
		P1M0=0x00;
		P1M1=0x00;
		P2M0=0x00;
		P2M1=0x00;
		P3M0=0x00;
		P3M1=0x00;
		P4M0=0x00;
		P4M1=0x00;
		P5M0=0x00;
		P5M1=0x00;
		P6M0=0x00;
		P6M1=0x00;
		P7M0=0x00;
		P7M1=0x00;
	#endif
}

void main(void)
{
	unsigned char t=0,i=0;

	GPIOInit();
	#if DEBUG
		Uart1_Init();
	#endif
	Timer_Init();

	/* Ӳ����ʼ�� */
	enc28j60_init();//����Ҫ����Timer_Init()���棬Ҫ����ʼ�����ɹ��ͻῨ��

	/* �����ʼ�� */
	net_init();

	while(1)
	{
		//���д��룬��÷ŵ�while(1)�е���ǰ��
		enc28j60IsRst(5);	//���enc28j60�Ƿ���������������λ

		if(t!=Tim_Sec)
		{
			t=Tim_Sec;
			if(ReHost[3].Stat==CONNECTED)
			{
				i++;
				DATA[0]='0'+i/10%10;
				DATA[1]='0'+i%10;
				Send_Data(3,2);
			}
		}

		enc28j60_runtime();
	}
}

