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

U8 SRCHeader = 0x23;
U8 SRCCID[] = {"SRC00000000000001"};
 unsigned int   Timer4_Count=1;

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


void SendAckData(U8 len, unsigned char *RES_DATA) {

	unsigned int i;

    for(i=0;i<len;i++)DATA[i]=RES_DATA[i];
			Send_Data(2,len);

}


unsigned char CheckBCC(unsigned char len, unsigned char *recv){
	  unsigned char bcc = 0x00;
		unsigned char i=0;
    for(i=0;i<len-1;i++)
    {
        bcc^=recv[i];
    };
    return bcc;

}




void ResponseData(unsigned char len,unsigned char *RES_DATA) {
	if(len <26){
		return ;
	}
	
	//У���
	if(CheckBCC(len, RES_DATA) == RES_DATA[len-1]){
		
		 unsigned int dataCmdFlag = (RES_DATA[2] << 8) | RES_DATA[3];//RES_DATA[2];         //�����ʶ
		 unsigned char dataCmdAck = RES_DATA[4];          //Ӧ���ʶ
		 unsigned char dataCid[17] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};   //Ψһ�豸��
		 unsigned char j=0;
		 unsigned char dataEncryptFlag = RES_DATA[22];    //���ܷ�ʽ
		 unsigned char dataUintLength = (RES_DATA[23] << 8) | RES_DATA[24];  //���ݳ���
		 unsigned char xdata dataTimestamp[6] = {0x00,0x00,0x00,0x00,0x00,0x00};  //ʱ������

	 //У��CID�Ƿ���ȷ
		 for(j=5;j<22;j++){
			  if(SRCCID[j-5] != RES_DATA[j]){
				 return;
			 }
		 }
		
		 //У�鳤���Ƿ���ȷ
		 if ((26 + dataUintLength) != len) {
				return ;
		 }
		 
		 //����ʱ��
		 for(j=0;j<6;j++){
			 dataTimestamp[j] = RES_DATA[25+j];
		 }
		 
		 if(dataCmdFlag == 0x8001){//������֤
			 
		 }else if(dataCmdFlag ==0x8002){//ʵʱ��Ϣ�����ϱ�
			 
		 }else if(dataCmdFlag ==0x8003){//����
			 
		 }else if(dataCmdFlag ==0x8004){//�豸�ǳ�
			 
		 }else if(dataCmdFlag ==0x8005){//����
			 
		 }else if(dataCmdFlag ==0x8006){//Զ�̿���

			 if(RES_DATA[31] == 0x02){//�������ݲ�ѯ
					unsigned char  light_status = LED ? 0x02 : 0x01;
					unsigned char buzzy_status = Buzzer ? 0x02 : 0x01;
					unsigned char xdata ds[37] = {0X23, 0X23, 0X10, 0X02, 0XFE, 0x53, 0x52, 0x43, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x01, 0x00, 0x0B, 0x14, 0x05, 0x18, 0x15, 0x24, 0x38, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
					unsigned char dslen =37;
				
					 	ds[2] = 0X10;
					ds[3] = 0X02;
						if(dataCmdAck == 0xFE){
						ds[4] = 0x01;//�ɹ�
						
					}
					ds[32] = DATA_Temphui[0];
					ds[33] = 	DATA_Temphui[1];
					ds[34] = light_status;
					ds[35] = buzzy_status;
					
				
				 ds[dslen-1] = CheckBCC(dslen, ds);
						SendAckData(dslen,ds);

				 
				 
			 }else if(RES_DATA[31] == 0x03){//��������
				 			 
					 unsigned char light = RES_DATA[32];
					 unsigned char buzzy = RES_DATA[33];
			 
					 if( light==0x02){
							Led_Actions_Status(0);
						}else if( light==0x01){
							Led_Actions_Status(1);
						}
					 
					 if( buzzy==0x02){
							Buzzer_Actions_Status(0);
					 }else if( buzzy==0x01){
							Buzzer_Actions_Status(1);
					 }
					 		RES_DATA[2] = 0X10;
					RES_DATA[3] = 0X02;

					if(dataCmdAck == 0xFE){
						RES_DATA[4] = 0x01;//�ɹ�
					
					}
						RES_DATA[len-1] = CheckBCC(len, RES_DATA);
						SendAckData(len,RES_DATA);

			 
			 }else if(RES_DATA[30] == 0x7F){//����
				 	IAP_CONTR = 0X20;
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
			if( p[0] ==SRCHeader &&  p[1] ==SRCHeader) {
					Timer4_Count = 1;
	       ResponseData(len, p);
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
