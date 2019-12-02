#include "fu_app.h"
#include "string.h"
#include "fu_config.h"
#include "fu_udp.h"
#include "fu_tcp.h"
#include "fu_send.h"
#include "fu_receive.h"
#include "fu_dhcp.h"
#include "fu_http.h"
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

U8 DATA_LENGTH = 9;
U8 DATA_GET[]=  { 0x7E, 0, 0, 0, 0, 0, 0, 0, 0x7E};
 U8 SRCHeader = 0x7E;
 U8 SRCTail = 0x7E;
 U8 SRCDeviceID = 0x01;
 U8 SRCCommunicationType = 0x01;

sbit Buzzer    =  P5 ^ 4;           	// ������
sbit LED      =  P3 ^ 2;         		  // LED��

///LED��˸  ����  ʱ��
void Led_Actions_NumAndMS(unsigned char  number,unsigned char ms){
	
	if(number>0 && ms>0){
	
		while(number){
				number--;
				LED = 1;
				DELAY_MS(ms);
				LED = 0;
				if(number){
				   DELAY_MS(ms);
				}
		}
	}
}

void Led_Actions_Status(unsigned char status){

	if(status){
		LED = 0;
	}else{
		LED = 1;
	}

}
void Buzzer_Actions_Status(unsigned char status){

	if(status){
		Buzzer = 0;
	}else{
		Buzzer = 1;
	}

}

///У������׼ȷ�� ��CRCУ��
unsigned char CheckData(unsigned char *mes){
    unsigned char crc = 0;
    unsigned char len = 6;
    unsigned char i=0;
    unsigned char cs=0;
    unsigned char message[] = {0,0,0,0,0,0};
    unsigned char *s = message;
    for( cs=0;cs<len;cs++){
        
        s[cs] = mes[cs+1];
    }
    
    
    while(len--)
    {
        crc ^= *s++;
        for(i = 0;i < 8;i++)
        {
            if(crc & 0x01)
            {
                crc = (crc >> 1) ^ 0x8c;
            }
            else crc >>= 1;
        }
    }
    return crc;
}



void SendAckData(unsigned char *RES_DATA) {

    unsigned char DATA_SEND[]= { 0x7E, 0x00,0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x7E};
	unsigned int i;

    DATA_SEND[0]= SRCHeader;
    DATA_SEND[1]= SRCDeviceID;
    DATA_SEND[2]= SRCCommunicationType;
    DATA_SEND[3]= RES_DATA[3];
    DATA_SEND[5]= RES_DATA[5];
    DATA_SEND[6]= RES_DATA[6];
    DATA_SEND[DATA_LENGTH-1]= SRCTail;
    DATA_SEND[7]= CheckData(DATA_SEND);

    for(i=0;i<DATA_LENGTH;i++)DATA[i]=DATA_SEND[i];
			Send_Data(2,DATA_LENGTH);

}

void ResponseData(unsigned char *RES_DATA) {
	

	if(RES_DATA[1]== SRCDeviceID &&  RES_DATA[2]== 0x01){
		
		if(  RES_DATA[4]== 0x01 && (CheckData(RES_DATA) == RES_DATA[DATA_LENGTH-2])) {
				switch(RES_DATA[3]){
					case 0x00:{//������
						if( RES_DATA[5]==0x00 && RES_DATA[6]==0x00){
								if(DATA_Temphui[2]==1)
								{
										DATA_Temphui[2]=0;//��λ����  ���ڼ���Ƿ��յ�����
										
										RES_DATA[3]=0x04;//����λ���� 4������ʪ��ָ��
										RES_DATA[5]= DATA_Temphui[0];//����λ����
										RES_DATA[6]= DATA_Temphui[1];//����ת��  ����λ����λ
								}	
																
							SendAckData(RES_DATA);
							Led_Actions_NumAndMS(1,80);
						}
						

						break;
					};
					case 0x01:{break;};
					case 0x02:{//����
						if( RES_DATA[6]==0x02){
							 Buzzer_Actions_Status(0);
						}else if( RES_DATA[6]==0x01){
							 Buzzer_Actions_Status(1);
						}
						SendAckData(RES_DATA);
						break;
					};
					case 0x03:{//��
						if( RES_DATA[6]==0x02){
							Led_Actions_Status(0);
						}else if( RES_DATA[6]==0x01){
							Led_Actions_Status(1);
						}
						SendAckData(RES_DATA);
						break;
					};
					case 0xFF:{//����
				  	SendAckData(RES_DATA);
						IAP_CONTR = 0X60;
						break;
					};

					default:
						break;
					
				}
			}
		
		
		
		
		
	}
		

}




/*******************************************/
/* ���ܣ�������յ������ݰ�                */
/* ���룺��					               */
/* �������							       */
/* ��ע����Ч���ݴ�DATA[42]��ʼ	    	   */
/*******************************************/
void App_Bag(U8 Index,U8 xdata *p,U16 len)
{
	unsigned int i;
	if(Index==2)
	{
			if(len == DATA_LENGTH && p[0] ==0x7E &&  p[8] ==0x7E) {
	       ResponseData(p);
	   }

		for(i=0;i<len;i++)
		{
			#if DEBUG
				Uart1_Send(p[i]);
			#endif
		}
	}
	else{
		for(i=0;i<len;i++)DATA[i]=p[i];
		//Send_Data(2,len);
		
	}
}


/*******************************************/
/* ���ܣ�����UDP��               		   */
/* ���룺��					               */
/* �������							       */
/* ��ע����Ч���ݴ�DATA[42]��ʼ	    	   */
/*******************************************/
//void App_UDP_Bag(void)
//{
//	//����ʶ��
//	if (memcmp(&DATA[42],"open",4) ==0)
//	{
////		if(DATA[46]=='1')
////		led0=0;
////		if(DATA[46]=='2')
////		led1=0;
////		if(DATA[46]=='3')
////		led2=0;
////		if(DATA[46]=='4')
////		led3=0;
//		
//		memcpy(DATA,"opened!",7);
//		Send_UDP_Bag(7);//����UDP���ݰ� 
//	}
//
//	if (memcmp(&DATA[42],"close",5) ==0)
//	{
//		
////		if(DATA[47]=='1')
////		led0=1;
////		if(DATA[47]=='2')
////		led1=1;
////		if(DATA[47]=='3')
////		led2=1;
////		if(DATA[47]=='4')
////		led3=1; 
//		
//		memcpy(DATA,"closed!",7);
//		Send_UDP_Bag(7);//����UDP���ݰ�
//	}
//	
//}

//*******************************************/
/* ���ܣ�����TCP��               		   */
/* ���룺��					               */
/* �������							       */
/* ��ע����Ч���ݴ�DATA[54]��ʼ  		   */
/*******************************************/
//void App_TCP_Bag(void)
//{
//	
//		char datlen=0;
//
//	 /* ��ͨTCP���� */
//	 if (memcmp(&DATA[54],"open",4) ==0)	
//	   {
//
////		  if(DATA[58]=='1')
////		    led0=0;
////		  if(DATA[58]=='2')
////		    led1=0;
////		  if(DATA[58]=='3')
////		    led2=0;
////		  if(DATA[58]=='4')
////		    led3=0;
//		    
//			TCP_Flag=(TCP_ACK|TCP_PSH);//��λ��ʶ
//			 memcpy(DATA,"opened!",7);//�������ݵ���������
//		   	Send_TCP_Bag(7);	
//		 
//		}
//		
//	   if (memcmp(&DATA[54],"close",5) ==0)
//	   {
//
////		  if(DATA[59]=='1')
////		    led0=1;
////		  if(DATA[59]=='2')
////		    led1=1;
////		  if(DATA[59]=='3')
////		    led2=1;
////		  if(DATA[59]=='4')
////		    led3=1;
//			
//			TCP_Flag=(TCP_ACK|TCP_PSH);//��λ��ʶ
//			 memcpy(DATA,"closed!",7);//�������ݵ���������
//		   	Send_TCP_Bag(7); 
//		}
//
//	   	/* ��ҳweb����ʶ�� */
//	   //index
//	   if (memcmp(&DATA[54],"GET / ",6) ==0)
//	   {
//		 if(TCP_Buf==0)//�ϴ��Ѿ��������Ӧ
//		 {
//		 pSend=fu_index;
//		 TCP_Send_Over(pSend,253);
//		 }
//	   }
//       //open
//	   else if (memcmp(&DATA[54],"GET /open",9) ==0)
//	   {
////		 led1=0;
////		 led2=0;
//		 if(TCP_Buf==0)
//		 {
//		 pSend=fu_open;
//		 TCP_Send_Over(pSend,120);
//		 }
//	   }
//	   //close
//	   else if (memcmp(&DATA[54],"GET /close",10) ==0)
//	   {
////		 led1=1;
////		 led2=1;
//		 if(TCP_Buf==0)
//		 {
//		  pSend=fu_close;
//		 TCP_Send_Over(pSend,119);
//		 }
//	   }
//}
//
