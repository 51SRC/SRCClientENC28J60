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


unsigned char RES_DATA[]= { 0X23, 0X23, 0X02, 0XFE, 0x53, 0x52, 0x43, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x01, 0x00, 0x0B, 0x14, 0x05, 0x18, 0x15, 0x24, 0x38, 0x02, 0X23, 0X24, 0X02, 0X02, 0xB0};
unsigned char RES_LEN= 36;

//U8 SRCCID[] = {"SRC00000000000001"};// 0x52, 0x43, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31

void Timer4Init(void)		//5毫秒@11.0592MHz
{
	T4T3M |= 0x20;		//定时器时钟1T模式
	T4L = 0x00;		//设置定时初值
	T4H = 0x28;		//设置定时初值
	T4T3M |= 0x80;		//定时器4开始计时
		IE2 |= 0x40;		//开定时器4中断
		EA=1; 	//总中断开启
}


//中断服务程序
void Timer4_interrupt() interrupt 20           //中断入口
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
					DATA_Temphui[2]=0;//复位将其  用于检测是否收到数据
					
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



//所有端口初始化成为通用IO
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

	/* 硬件初始化 */
	enc28j60_init();//必须要放在Timer_Init()后面，要不初始化不成功就会卡死

	/* 网络初始化 */
	net_init();

	Buzzer = 0;//关闭蜂鸣器
  LED = 0;
	while(1)
	{
		//此行代码，最好放到while(1)中的最前面
		enc28j60IsRst(5);	//检查enc28j60是否死机，如死机则复位

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
		
				if(DHT11_Read_Data(&DATA_Temphui[0],&DATA_Temphui[1])==0)//温湿度检测
			{
				
				 DATA_Temphui[2]=1;
				 
						
				 
			}
		

		enc28j60_runtime();
	}
}

