#include "mcuinit.h"
#include "timer.h"
#include "enc28j60.h"
#include "fu_send.h"
#include "fu_config.h"
#include "fu_receive.h"
#include "fu_app.h"
#include "DHT11.h"

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


unsigned char RES_DATA[]= { 0X23, 0X23, 0X02, 0XFE, 0x53, 0x52, 0x43, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x01, 0x00, 0x0B, 0x14, 0x05, 0x18, 0x15, 0x24, 0x38, 0x02, 0X23, 0X24, 0X02, 0X02, 0xB0};
unsigned char RES_LEN= 36;

//U8 SRCCID[] = {"SRC00000000000001"};// 0x52, 0x43, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31

void Timer4Init(void)		//5����@11.0592MHz
{
	T4T3M |= 0x20;		//��ʱ��ʱ��1Tģʽ
	T4L = 0x00;		//���ö�ʱ��ֵ
	T4H = 0x28;		//���ö�ʱ��ֵ
	T4T3M |= 0x80;		//��ʱ��4��ʼ��ʱ
		IE2 |= 0x40;		//����ʱ��4�ж�
		EA=1; 	//���жϿ���
}


//�жϷ������
void Timer4_interrupt() interrupt 20           //�ж����
{
		
		if(Timer4_Count>=2000){
			
				
			unsigned char  light_status = LED ? 0x02 : 0x01;
			unsigned char buzzy_status = Buzzer ? 0x02 : 0x01;
		unsigned char j = 4;
		
//			U8 *SRCCID = "SRC00000000000001";
//		for(j=4;j<=21;j++){
//			RES_DATA[j] = SRCCID[j-4];
//		}		
		
			Timer4_Count = 1;
			

			if(DATA_Temphui[2]==1)
			{
					DATA_Temphui[2]=0;//��λ����  ���ڼ���Ƿ��յ�����
					
			}

			RES_DATA[31] = DATA_Temphui[0];
			RES_DATA[32] = 	DATA_Temphui[1];
			RES_DATA[33] = light_status;
			RES_DATA[34] = buzzy_status,
			RES_DATA[RES_LEN-1] = CheckBCC(RES_LEN, RES_DATA);
					
			SendAckData(RES_LEN,RES_DATA);
			
		}else{
			
		Timer4_Count++;
		}
}



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
Timer4Init();

	/* Ӳ����ʼ�� */
	enc28j60_init();//����Ҫ����Timer_Init()���棬Ҫ����ʼ�����ɹ��ͻῨ��

	/* �����ʼ�� */
	net_init();

	Buzzer = 0;//�رշ�����
  LED = 0;
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
		
				if(DHT11_Read_Data(&DATA_Temphui[0],&DATA_Temphui[1])==0)//��ʪ�ȼ��
			{
				
				 DATA_Temphui[2]=1;
				 
						
				 
			}
		

		enc28j60_runtime();
	}
}

