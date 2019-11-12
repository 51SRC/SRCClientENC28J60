#include "Uart.h"
#include <string.h>

#if UART1

	#if UART1_RECV
		U8 xdata Uart1RBuf[UART1_MAXRI];		//接收缓存区
		#if UART1_MAXRI>255
			U16 xdata Uart1Ri=0,Uart1Ri2=0;		//Uart1Ri为剩余接收个数，已接收个数
		#else
			U8 xdata Uart1Ri=0,Uart1Ri2=0;		//Uart1Ri为剩余接收个数，已接收个数
		#endif
	#endif
	#if UART1_NONS
		U8 xdata Uart1TBuf[UART1_MAXTI];		//发送缓存区
		#if UART1_MAXTI>255
			U16 xdata Uart1Ti=0,Uart1Ti2=0;
		#else
			U8 xdata Uart1Ti=0,Uart1Ti2=0;
		#endif
		U8 xdata Uart1Busy=0;				//发送不忙
	#endif
	
	void Uart1_Init(void)	//串口1初始化函数
	{
		//89C单片机只能以定时器1做为波特率发生器，所以定时器1不能再使用了
		#if MCU_TYPE==STC89C
			SCON = 0x50;		//8位数据,可变波特率，原本是0x50，但必须要加0x02，因为要把TI初始化成1

			TMOD &= 0x0F;
			TMOD |= 0x20;		//Set Timer1 as 8-bit auto reload mode
			TH1 = TL1 = -(FOSC/32/12/BAUD1);		//8位定时器自动重装
			TR1 = 1;			//Timer1 start run
		#endif

		#if MCU_TYPE==STC12C
			//以独立波特率初始化的好处是，定时器0与1都可以使用
			PCON &= 0x7F;		//波特率不倍速
			SCON = 0x50;		//8位数据,可变波特率，原本是0x50，但必须要加0x02，因为要把TI初始化成1
//			AUXR &= 0xFB;		//独立波特率发生器时钟为Fosc/12,即12T
//			BRT = 256-(FOSC/32/12/BAUD1);		//设定独立波特率发生器重装值，如果AUXR设为1T模式时/12就不用了
			AUXR |= 0x04;		//独立波特率发生器时钟为1T模式为0x04，Fosc不用/12，1T模式下支持更高的波特率
			//15系列单片机不支持独立波特率
			BRT = 256-(FOSC/32/BAUD1);		//设定独立波特率发生器重装值，如果AUXR设为1T模式时/12就不用了
			AUXR |= 0x01;		//串口1选择独立波特率发生器为波特率发生器
			AUXR |= 0x10;		//启动独立波特率发生器
		#endif

		#if MCU_TYPE==STC15W
		    P_SW1 &= 0x3F;  //(P3.0/RxD, P3.1/TxD)	//端口切换
		    
		//  P_SW1 &= 3F;			//~(S1_S0 | S1_S1)=0x3F
		//  P_SW1 |= S1_S0;			//(P3.6/RxD_2, P3.7/TxD_2)

		//  P_SW1 &= 3F;			//S1_S0=0 S1_S1=1
		//  P_SW1 |= S1_S1;			//(P1.6/RxD_3, P1.7/TxD_3)

//			SCON = 0x50;		//8位数据,可变波特率
//			AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
//			AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
//			TMOD &= 0x0F;		//设定定时器1为16位自动重装方式
//			TL1 = (65536 - (FOSC/4/BAUD1));		//设定定时初值
//			TH1 = (65536 - (FOSC/4/BAUD1))>>8;		//设定定时初值
//			ET1 = 0;		//禁止定时器1中断
//			TR1 = 1;		//启动定时器1

//			以定时器2做为波特率发生器
			SCON = 0x50;		//8位数据,可变波特率
			AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
			AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
			AUXR |= 0x10;		//定时器2开始计时
			T2L = (65536 - (FOSC/4/BAUD1));		//设定定时初值
			T2H = (65536 - (FOSC/4/BAUD1))>>8;		//设定定时初值
			AUXR |= 0x10;		//启动定时器2
		#endif

		#if UART1_485		//初始化成接收状态
			RS485_1=0;
		#endif
		#if UART1_RECV || UART1_NONS
			ES=1;			//开启串口中断，如果只做为同步发送用，则无需开启
		#endif
		EA=1;				//开启总中断，如果只做为发送用，则无需开启
	}

	#if UART1_SEND || UART1_NONS
		//实时发送一个字符，
		void Uart1_Send(U8 send)
		{
			#if UART1_485
				RS485_1=1;		//发送前，改为发送状态
			#endif
		
			#if UART1_NONS		//如果为异步发送
				Uart1TBuf[Uart1Ti]=send;//压入发送缓存中
				Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;
				if(!Uart1Busy)				//如果不忙
				{
					Uart1Busy=1;				//改发送状态为忙
					SBUF=Uart1TBuf[Uart1Ti2];	//发送当前字符
					Uart1Ti2 = (Uart1Ti2+1) % UART1_MAXTI;	//已发送+1
				}
			#else
				SBUF=send;		//同步发送
				while(!TI);			//等待TI变成1，说明是发送完成，非中断方式
				TI=0;				//更改状态，这里并不会改变P3.3的端口的状态
				
				#if UART1_485
					RS485_1=0;		//发送完后，改为接收状态
				#endif
			#endif
		}
		//实时发送一个字符，转成16进制再发送
		void Uart1_SendHex(U8 send)
		{
			#define hex(n) {if(n>9)n+='A'-10;else n+='0';}
			U8 t;
			#if UART1_485
				RS485_1=1;		//发送前，改为发送状态
			#endif
		
			#if UART1_NONS		//如果为异步发送
				t=send>>4;//取高4位
				hex(t);
				Uart1TBuf[Uart1Ti]=t;//压入发送缓存中
				Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;
				t=send&0x0f;//取低4位
				hex(t);
				Uart1TBuf[Uart1Ti]=t;//压入发送缓存中
				Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;
				Uart1TBuf[Uart1Ti]=' ';
				Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;
				if(!Uart1Busy)				//如果不忙
				{
					Uart1Busy=1;				//改发送状态为忙
					SBUF=Uart1TBuf[Uart1Ti2];	//发送当前字符
					Uart1Ti2 = (Uart1Ti2+1) % UART1_MAXTI;	//已发送+1
				}
			#else
				t=send>>4;//取高4位
				hex(t);
				SBUF=t;		//同步发送
				while(!TI);			//等待TI变成1，说明是发送完成，非中断方式
				TI=0;
								//更改状态，这里并不会改变P3.3的端口的状态
				t=send&0x0f;//取低4位
				hex(t);
				SBUF=t;
				while(!TI);
				TI=0;
				
				#if UART1_485
					RS485_1=0;		//发送完后，改为接收状态
				#endif
			#endif
		}
		//实时发送一个字符串，
		void Uart1_SendStr(U8 *send)
		{
			if(*send)//字符串不为空
			{
				#if UART1_485
					RS485_1=1;		//发送前，改为发送状态
				#endif
			
				#if UART1_NONS		//如果为异步发送
					while(*send)
					{
						Uart1TBuf[Uart1Ti]=*send++;//压入发送缓存中
						Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;
						if(!Uart1Busy)				//如果不忙
						{
							Uart1Busy=1;				//改发送状态为忙
							SBUF=Uart1TBuf[Uart1Ti2];	//发送当前字符
							Uart1Ti2 = (Uart1Ti2+1) % UART1_MAXTI;	//已发送+1
						}
					}
				#else
					while(*send)
					{
						SBUF=*send++;		//同步发送
						while(!TI);			//等待TI变成1，说明是发送完成，非中断方式
						TI=0;				//更改状态，这里并不会改变P3.3的端口的状态
					
						#if UART1_485
							RS485_1=0;		//发送完后，改为接收状态
						#endif
					}
				#endif
			}
		}
		//实时发送一个数，显示000~255的数字，用于显示IP地址
		void Uart1_SendNum(U8 send)
		{
			#define SENDNUM1(n) {Uart1TBuf[Uart1Ti]=n+'0';Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;}//异步发送
			#define SENDNUM2(n)	{SBUF=n+'0';while(!TI);TI=0;}//同步发送
			#if UART1_485
				RS485_1=1;		//发送前，改为发送状态
			#endif
		
			#if UART1_NONS		//如果为异步发送
				if(send>99)SENDNUM1(send/100);
				if(send>9)SENDNUM1(send/10%10);
				SENDNUM1(send%10);
				if(!Uart1Busy)//如果不忙
				{
					Uart1Busy=1;				//改发送状态为忙
					SBUF=Uart1TBuf[Uart1Ti2];	//发送当前字符
					Uart1Ti2 = (Uart1Ti2+1) % UART1_MAXTI;	//已发送+1
				}
			#else
				if(send>99)SENDNUM2(send/100);
				if(send>9)SENDNUM2(send/10%10);
				SENDNUM2(send%10);
			
				#if UART1_485
					RS485_1=0;		//发送完后，改为接收状态
				#endif
			#endif
		}
	#endif

	#if UART1_RECV
		//接收一个字节，在接收之前，用if(ifRecv())先判断有无数据
		U8 Uart1_Recv()
		{
			U8 t;
			t=Uart1RBuf[Uart1Ri2];
			Uart1Ri2 = (Uart1Ri2+1)%UART1_MAXRI;
			return t;
		}
	#endif
    #if UART1_RECV || UART1_NONS		
		/*串行通讯中断，收发完成都会进入该中断*/
		void UART1_ISR(void) interrupt 4	  //0号串口中断
		{
			if(RI)									  //sbit RI    = SCON^0;
			{
				RI=0;//接收处理完成后再清零，表示将继续接收
				#if UART1_RECV
					//接收一个字节
					Uart1RBuf[Uart1Ri]=SBUF;	//把收到的字符，压入循环队列中
					Uart1Ri=(Uart1Ri+1)%UART1_MAXRI;	//接收缓存后移一位
				#endif
			}

			if(TI)
			{
				TI=0;
				#if UART1_NONS			//只有当异步发送时，才在中断中修改TI的值
					if(TING1)		//如果还有内容需要发送
					{
						SBUF=Uart1TBuf[Uart1Ti2];	//发送当前字符
						Uart1Ti2 = (Uart1Ti2+1) % UART1_MAXTI;	//已发送+1
					}
					else			//全部发完
					{
						Uart1Busy=0;	//不忙，可以继续发送
						#if UART1_485
							RS485_1=0;		//发送完后，改为接收状态
						#endif
					}
				#endif
			}
		}
	#endif
#endif

