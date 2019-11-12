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

	/* 硬件初始化 */
	enc28j60_init();//必须要放在Timer_Init()后面，要不初始化不成功就会卡死

	/* 网络初始化 */
	net_init();

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

		enc28j60_runtime();
	}
}

