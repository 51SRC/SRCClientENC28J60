#include "TIMER.h"

//��Ϊͬһ����ֻ��Ҫһ����ʱ���͹��ˣ���Ҫ���ڲ���ʱ��˸�����������Ѷ��ң�����Ķ�ʱ������ģ�����趨
#define TIMER 1		//��ʱ����0-4������STC15ϵ�е�Ƭ����0-4��STC89��STC12ֻ������0��1��Ĭ��Ϊ��ʱ��1

#if TIMER==0  		//������ʱ�����жϺ�
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


unsigned int xdata RL_THL;		//��װֵ�����ټ�����
unsigned char xdata Tim_Ms=0;
unsigned char xdata Tim_10Ms=0,Tim_Sec=0;

//��ʱ����ʼ��
void Timer_Init(void)
{
	RL_THL = (65536 - RELOAD);	//��װֵ
	#if TIMER==0
		#if MCU_TYPE != STC89C	//89оƬ��û��AUXR�Ĵ���	
			#if TIMER_TYPE==TIM1MS	//���Ϊ1����
				AUXR |= 0x80;		//��ʱ��ʱ��1Tģʽ
			#else
				AUXR &= 0x7F;		//��ʱ��ʱ��12Tģʽ
			#endif
		#endif
		TMOD &= 0xF0;			//���ö�ʱ��ģʽ
		#if MCU_TYPE!=STC15W	//15ϵ���Զ���װ��ģʽΪ0
			TMOD |= 0x01;		//���ö�ʱ��ģʽ1��16λ��ʱ��
		#endif
		TH0=RL_THL>>8;			//���嶨ʱ�����ж�ʱ��,11.059M����,���0x4C00,��50ms
		TL0=RL_THL;				//12M����,��Ϊ0x3CB0,��50ms,��ʵ������ȷ��
		TF0 = 0;		//���TF0��־
		TR0=1;    		//��ʱ��0����
		ET0=1;   		//1���жϿ���/0�Ŷ�ʱ���жϿ���
		EA=1; 	//���жϿ���
	#endif

	#if TIMER==1
		#if MCU_TYPE != STC89C	//89оƬ��û��AUXR�Ĵ���	
			#if TIMER_TYPE==TIM1MS	//���Ϊ1����
				AUXR |= 0x40;		//��ʱ��ʱ��1Tģʽ
			#else
				AUXR &= 0xBF;		//��ʱ��ʱ��12Tģʽ��Ĭ��Ϊ12Tģʽ
			#endif
		#endif
		TMOD &= 0x0F;			//���ö�ʱ��ģʽ
		#if MCU_TYPE!=STC15W	//15ϵ���Զ���װ��ģʽΪ0
			TMOD |= 0x10;		//��ʱ��������ʽΪ1��16λ��ʱ��
		#endif
		TH1=RL_THL>>8; 			//���嶨ʱ�����ж�ʱ��,11.059M����,���0x4C00,��50ms
		TL1=RL_THL;				//12M����,��Ϊ0x3CB0,��50ms,��ʵ������ȷ��
		TF1 = 0;			//���TF1��־
		TR1=1;				//��ʱ��1����
		ET1=1;			//3���жϿ���/1�Ŷ�ʱ���жϿ���
		EA=1; 	//���жϿ���
	#endif

	#if TIMER==2
		#if TIMER_TYPE==TIM1MS	//���Ϊ1����
			AUXR |= 0x04;		//��ʱ��ʱ��1Tģʽ
		#else
			AUXR &= 0xFB;		//��ʱ��ʱ��12Tģʽ
		#endif
		T2L = RL_THL;		//���ö�ʱ��ֵ
		T2H = RL_THL>>8;	//���ö�ʱ��ֵ
		AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
		IE2  |=  (1<<2);    //�����ж�
		EA=1; 	//���жϿ���
	#endif

	#if TIMER==3
		#if TIMER_TYPE==TIM1MS	//���Ϊ1����
			T4T3M |= 0x02;		//��ʱ��ʱ��1Tģʽ
		#else
			T4T3M &= 0xFD;		//��ʱ��ʱ��12Tģʽ
		#endif
		T3L = RL_THL;		//���ö�ʱ��ֵ
		T3H = RL_THL>>8;	//���ö�ʱ��ֵ
		T4T3M |= 0x08;		//��ʱ��3��ʼ��ʱ
		IE2 |= 0x20;		//����ʱ��3�ж�
		EA=1; 	//���жϿ���
	#endif

	#if TIMER==4
		#if TIMER_TYPE==TIM1MS	//���Ϊ1����
			T4T3M |= 0x20;		//��ʱ��ʱ��1Tģʽ
		#else
			T4T3M &= 0xDF;		//��ʱ��ʱ��12Tģʽ
		#endif
		T4L = RL_THL;		//���ö�ʱ��ֵ
		T4H = RL_THL>>8;	//���ö�ʱ��ֵ
		T4T3M |= 0x80;		//��ʱ��4��ʼ��ʱ
		IE2 |= 0x40;		//����ʱ��4�ж�
		EA=1; 	//���жϿ���
	#endif

}

void tim() interrupt INTX
{
	#if MCU_TYPE!=STC15W	//15ϵ���Զ���װ�������ֶ���װ
		#if TIMER == 1
			TH1 = RL_THL>>8;
			TL1 = RL_THL;
		#else
			TH0 = RL_THL>>8;
			TL0 = RL_THL;
		#endif
	#endif

	#if TIMER_TYPE==TIM10MS		//10ms
		if(!(Tim_10Ms=(Tim_10Ms+1)%100))	/*���ʱ���Ѵ�1��*/
		{
			Tim_Sec=(Tim_Sec+1)%60;
		}
	#else	//1ms
		if(!(Tim_Ms=(Tim_Ms+1)%10))	/*���ʱ���Ѵ�1��*/
		{
			if(!(Tim_10Ms=(Tim_10Ms+1)%100))
			{
				Tim_Sec=(Tim_Sec+1)%60;
			}
		}
	#endif
}
