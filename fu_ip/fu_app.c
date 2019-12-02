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
/* 本程序仅供学习参考，未经作者允许，不得用于任何商业用途            */
/* FuIP专为单片机而定制的TCP/IP协议栈，适用于任何8,16,32位单片机     */
/* 主程序，可用于客户端，服务端    					            	 */
/* 技术论坛：www.fuhome.net/bbs/						             */
/* 移植请注意，此程序使用硬件SPI，模拟SPI会有区别，本程序默认是模拟  */
/* 发送各种数据报程序，ARP DHCP Ping TCP（服务端/客户端） UDP        */
/* FuIP可用于，客户/服务，不限连接数，亦可接入fuhome.net 平台进行远程控制*/
/* 版本：v3.1 2018年7月23日						                     */
/* 龙剑奋斗	乘简									       		     */
/* 定期更新，敬请关注——开放，交流，深入，进步	     			     */
/* Copyright fuhome.net 未来之家 实验室，让科技融入生活				 */
/*********************************************************************/

U8 DATA_LENGTH = 9;
U8 DATA_GET[]=  { 0x7E, 0, 0, 0, 0, 0, 0, 0, 0x7E};
 U8 SRCHeader = 0x7E;
 U8 SRCTail = 0x7E;
 U8 SRCDeviceID = 0x01;
 U8 SRCCommunicationType = 0x01;

sbit Buzzer    =  P5 ^ 4;           	// 蜂鸣器
sbit LED      =  P3 ^ 2;         		  // LED灯

///LED闪烁  次数  时间
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

///校验数据准确性 做CRC校验
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
					case 0x00:{//心跳包
						if( RES_DATA[5]==0x00 && RES_DATA[6]==0x00){
								if(DATA_Temphui[2]==1)
								{
										DATA_Temphui[2]=0;//复位将其  用于检测是否收到数据
										
										RES_DATA[3]=0x04;//高两位数据 4代表温湿度指令
										RES_DATA[5]= DATA_Temphui[0];//高两位数据
										RES_DATA[6]= DATA_Temphui[1];//进制转换  低两位数据位
								}	
																
							SendAckData(RES_DATA);
							Led_Actions_NumAndMS(1,80);
						}
						

						break;
					};
					case 0x01:{break;};
					case 0x02:{//喇叭
						if( RES_DATA[6]==0x02){
							 Buzzer_Actions_Status(0);
						}else if( RES_DATA[6]==0x01){
							 Buzzer_Actions_Status(1);
						}
						SendAckData(RES_DATA);
						break;
					};
					case 0x03:{//灯
						if( RES_DATA[6]==0x02){
							Led_Actions_Status(0);
						}else if( RES_DATA[6]==0x01){
							Led_Actions_Status(1);
						}
						SendAckData(RES_DATA);
						break;
					};
					case 0xFF:{//重启
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
/* 功能：处理接收到的数据包                */
/* 输入：无					               */
/* 输出：无							       */
/* 备注：有效数据从DATA[42]开始	    	   */
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
/* 功能：处理UDP包               		   */
/* 输入：无					               */
/* 输出：无							       */
/* 备注：有效数据从DATA[42]开始	    	   */
/*******************************************/
//void App_UDP_Bag(void)
//{
//	//命令识别
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
//		Send_UDP_Bag(7);//发送UDP数据包 
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
//		Send_UDP_Bag(7);//发送UDP数据包
//	}
//	
//}

//*******************************************/
/* 功能：处理TCP包               		   */
/* 输入：无					               */
/* 输出：无							       */
/* 备注：有效数据从DATA[54]开始  		   */
/*******************************************/
//void App_TCP_Bag(void)
//{
//	
//		char datlen=0;
//
//	 /* 普通TCP数据 */
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
//			TCP_Flag=(TCP_ACK|TCP_PSH);//置位标识
//			 memcpy(DATA,"opened!",7);//复制数据到发送数组
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
//			TCP_Flag=(TCP_ACK|TCP_PSH);//置位标识
//			 memcpy(DATA,"closed!",7);//复制数据到发送数组
//		   	Send_TCP_Bag(7); 
//		}
//
//	   	/* 网页web命令识别 */
//	   //index
//	   if (memcmp(&DATA[54],"GET / ",6) ==0)
//	   {
//		 if(TCP_Buf==0)//上次已经发完才响应
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
