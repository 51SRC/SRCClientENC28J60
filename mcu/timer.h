#ifndef _TIMER_H_
#define _TIMER_H_
#include "mcuinit.h"

//����ʱ�����Զ�����ʱ�䣬10������1����
//ע�⣬��10���붨ʱʱ����Ҫ��12Tģʽ����Ϊ11M���Ͼ���11059200/100=110592�������ͷ�Χ
#define TIM10MS 0	//10����
#define TIM1MS  1	//1���룬����ĳЩ����ĵ�����
#define TIMER_TYPE TIM10MS		//������Ķ�ʱ���

#if TIMER_TYPE==TIM1MS
	#define RELOAD 	(FOSC / 1000)	//1ms��װֵ
#else
	#define RELOAD 	(FOSC_12 / 100)	//10ms��װֵ
#endif

extern unsigned int xdata RL_THL;			//��װֵ
extern unsigned char xdata Tim_Ms;			//1ms������
extern unsigned char xdata Tim_10Ms,Tim_Sec;

void Timer_Init(void);		//��ʱ����ʼ������
bit BetweenSec(unsigned char s);//ÿ��s�뷵��1����,sΪ1-59֮�����

#endif
