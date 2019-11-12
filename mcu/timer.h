#ifndef _TIMER_H_
#define _TIMER_H_
#include "mcuinit.h"

//本定时器可以定两种时间，10毫秒与1毫秒
//注意，在10毫秒定时时必须要用12T模式，因为11M以上晶振11059200/100=110592超出整型范围
#define TIM10MS 0	//10毫秒
#define TIM1MS  1	//1毫秒，用在某些特殊的单子上
#define TIMER_TYPE TIM10MS		//这里更改定时间隔

#if TIMER_TYPE==TIM1MS
	#define RELOAD 	(FOSC / 1000)	//1ms重装值
#else
	#define RELOAD 	(FOSC_12 / 100)	//10ms重装值
#endif

extern unsigned int xdata RL_THL;			//重装值
extern unsigned char xdata Tim_Ms;			//1ms计数器
extern unsigned char xdata Tim_10Ms,Tim_Sec;

void Timer_Init(void);		//定时器初始化函数
bit BetweenSec(unsigned char s);//每隔s秒返回1函数,s为1-59之间的数

#endif
