#include "TIMER.h"

//因为同一程序只需要一个定时器就够了，主要用于补偿时闪烁，比如红外与讯铃遥控器的定时器，在模块中设定
#define TIMER 1		//定时器几0-4的数，STC15系列单片机，0-4，STC89与STC12只有两个0与1，默认为定时器1

#if TIMER==0  		//各个定时器的中断号
	#define INTX 1
#endif
#if TIMER==1
	#define INTX 3
#endif
#if TIMER==2
	#define INTX 12
#endif
#if TIMER==3
	#define INTX 19
#endif
#if TIMER==4
	#define INTX 20
#endif


unsigned int xdata RL_THL;		//重装值，减少计算量
unsigned char xdata Tim_Ms=0;
unsigned char xdata Tim_10Ms=0,Tim_Sec=0;

//定时器初始化
void Timer_Init(void)
{
	RL_THL = (65536 - RELOAD);	//重装值
	#if TIMER==0
		#if MCU_TYPE != STC89C	//89芯片，没有AUXR寄存器	
			#if TIMER_TYPE==TIM1MS	//如果为1毫秒
				AUXR |= 0x80;		//定时器时钟1T模式
			#else
				AUXR &= 0x7F;		//定时器时钟12T模式
			#endif
		#endif
		TMOD &= 0xF0;			//设置定时器模式
		#if MCU_TYPE!=STC15W	//15系列自动重装，模式为0
			TMOD |= 0x01;		//设置定时器模式1，16位计时器
		#endif
		TH0=RL_THL>>8;			//定义定时器的中断时长,11.059M晶振,设成0x4C00,是50ms
		TL0=RL_THL;				//12M晶振,设为0x3CB0,是50ms,其实都不精确的
		TF0 = 0;		//清除TF0标志
		TR0=1;    		//定时器0开启
		ET0=1;   		//1号中断开启/0号定时器中断开启
		EA=1; 	//总中断开启
	#endif

	#if TIMER==1
		#if MCU_TYPE != STC89C	//89芯片，没有AUXR寄存器	
			#if TIMER_TYPE==TIM1MS	//如果为1毫秒
				AUXR |= 0x40;		//定时器时钟1T模式
			#else
				AUXR &= 0xBF;		//定时器时钟12T模式，默认为12T模式
			#endif
		#endif
		TMOD &= 0x0F;			//设置定时器模式
		#if MCU_TYPE!=STC15W	//15系列自动重装，模式为0
			TMOD |= 0x10;		//定时器工作方式为1，16位计时器
		#endif
		TH1=RL_THL>>8; 			//定义定时器的中断时长,11.059M晶振,设成0x4C00,是50ms
		TL1=RL_THL;				//12M晶振,设为0x3CB0,是50ms,其实都不精确的
		TF1 = 0;			//清除TF1标志
		TR1=1;				//定时器1开启
		ET1=1;			//3号中断开启/1号定时器中断开启
		EA=1; 	//总中断开启
	#endif

	#if TIMER==2
		#if TIMER_TYPE==TIM1MS	//如果为1毫秒
			AUXR |= 0x04;		//定时器时钟1T模式
		#else
			AUXR &= 0xFB;		//定时器时钟12T模式
		#endif
		T2L = RL_THL;		//设置定时初值
		T2H = RL_THL>>8;	//设置定时初值
		AUXR |= 0x10;		//定时器2开始计时
		IE2  |=  (1<<2);    //允许中断
		EA=1; 	//总中断开启
	#endif

	#if TIMER==3
		#if TIMER_TYPE==TIM1MS	//如果为1毫秒
			T4T3M |= 0x02;		//定时器时钟1T模式
		#else
			T4T3M &= 0xFD;		//定时器时钟12T模式
		#endif
		T3L = RL_THL;		//设置定时初值
		T3H = RL_THL>>8;	//设置定时初值
		T4T3M |= 0x08;		//定时器3开始计时
		IE2 |= 0x20;		//开定时器3中断
		EA=1; 	//总中断开启
	#endif

	#if TIMER==4
		#if TIMER_TYPE==TIM1MS	//如果为1毫秒
			T4T3M |= 0x20;		//定时器时钟1T模式
		#else
			T4T3M &= 0xDF;		//定时器时钟12T模式
		#endif
		T4L = RL_THL;		//设置定时初值
		T4H = RL_THL>>8;	//设置定时初值
		T4T3M |= 0x80;		//定时器4开始计时
		IE2 |= 0x40;		//开定时器4中断
		EA=1; 	//总中断开启
	#endif

}

void tim() interrupt INTX
{
	#if MCU_TYPE!=STC15W	//15系列自动重装，无需手动重装
		#if TIMER == 1
			TH1 = RL_THL>>8;
			TL1 = RL_THL;
		#else
			TH0 = RL_THL>>8;
			TL0 = RL_THL;
		#endif
	#endif

	#if TIMER_TYPE==TIM10MS		//10ms
		if(!(Tim_10Ms=(Tim_10Ms+1)%100))	/*如果时间已达1秒*/
		{
			Tim_Sec=(Tim_Sec+1)%60;
		}
	#else	//1ms
		if(!(Tim_Ms=(Tim_Ms+1)%10))	/*如果时间已达1秒*/
		{
			if(!(Tim_10Ms=(Tim_10Ms+1)%100))
			{
				Tim_Sec=(Tim_Sec+1)%60;
			}
		}
	#endif
}
