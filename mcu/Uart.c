#include "Uart.h"
#include <string.h>

#if UART1

	#if UART1_RECV
		U8 xdata Uart1RBuf[UART1_MAXRI];		//���ջ�����
		#if UART1_MAXRI>255
			U16 xdata Uart1Ri=0,Uart1Ri2=0;		//Uart1RiΪʣ����ո������ѽ��ո���
		#else
			U8 xdata Uart1Ri=0,Uart1Ri2=0;		//Uart1RiΪʣ����ո������ѽ��ո���
		#endif
	#endif
	#if UART1_NONS
		U8 xdata Uart1TBuf[UART1_MAXTI];		//���ͻ�����
		#if UART1_MAXTI>255
			U16 xdata Uart1Ti=0,Uart1Ti2=0;
		#else
			U8 xdata Uart1Ti=0,Uart1Ti2=0;
		#endif
		U8 xdata Uart1Busy=0;				//���Ͳ�æ
	#endif
	
	void Uart1_Init(void)	//����1��ʼ������
	{
		//89C��Ƭ��ֻ���Զ�ʱ��1��Ϊ�����ʷ����������Զ�ʱ��1������ʹ����
		#if MCU_TYPE==STC89C
			SCON = 0x50;		//8λ����,�ɱ䲨���ʣ�ԭ����0x50��������Ҫ��0x02����ΪҪ��TI��ʼ����1

			TMOD &= 0x0F;
			TMOD |= 0x20;		//Set Timer1 as 8-bit auto reload mode
			TH1 = TL1 = -(FOSC/32/12/BAUD1);		//8λ��ʱ���Զ���װ
			TR1 = 1;			//Timer1 start run
		#endif

		#if MCU_TYPE==STC12C
			//�Զ��������ʳ�ʼ���ĺô��ǣ���ʱ��0��1������ʹ��
			PCON &= 0x7F;		//�����ʲ�����
			SCON = 0x50;		//8λ����,�ɱ䲨���ʣ�ԭ����0x50��������Ҫ��0x02����ΪҪ��TI��ʼ����1
//			AUXR &= 0xFB;		//���������ʷ�����ʱ��ΪFosc/12,��12T
//			BRT = 256-(FOSC/32/12/BAUD1);		//�趨���������ʷ�������װֵ�����AUXR��Ϊ1Tģʽʱ/12�Ͳ�����
			AUXR |= 0x04;		//���������ʷ�����ʱ��Ϊ1TģʽΪ0x04��Fosc����/12��1Tģʽ��֧�ָ��ߵĲ�����
			//15ϵ�е�Ƭ����֧�ֶ���������
			BRT = 256-(FOSC/32/BAUD1);		//�趨���������ʷ�������װֵ�����AUXR��Ϊ1Tģʽʱ/12�Ͳ�����
			AUXR |= 0x01;		//����1ѡ����������ʷ�����Ϊ�����ʷ�����
			AUXR |= 0x10;		//�������������ʷ�����
		#endif

		#if MCU_TYPE==STC15W
		    P_SW1 &= 0x3F;  //(P3.0/RxD, P3.1/TxD)	//�˿��л�
		    
		//  P_SW1 &= 3F;			//~(S1_S0 | S1_S1)=0x3F
		//  P_SW1 |= S1_S0;			//(P3.6/RxD_2, P3.7/TxD_2)

		//  P_SW1 &= 3F;			//S1_S0=0 S1_S1=1
		//  P_SW1 |= S1_S1;			//(P1.6/RxD_3, P1.7/TxD_3)

//			SCON = 0x50;		//8λ����,�ɱ䲨����
//			AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
//			AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
//			TMOD &= 0x0F;		//�趨��ʱ��1Ϊ16λ�Զ���װ��ʽ
//			TL1 = (65536 - (FOSC/4/BAUD1));		//�趨��ʱ��ֵ
//			TH1 = (65536 - (FOSC/4/BAUD1))>>8;		//�趨��ʱ��ֵ
//			ET1 = 0;		//��ֹ��ʱ��1�ж�
//			TR1 = 1;		//������ʱ��1

//			�Զ�ʱ��2��Ϊ�����ʷ�����
			SCON = 0x50;		//8λ����,�ɱ䲨����
			AUXR |= 0x01;		//����1ѡ��ʱ��2Ϊ�����ʷ�����
			AUXR |= 0x04;		//��ʱ��2ʱ��ΪFosc,��1T
			AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
			T2L = (65536 - (FOSC/4/BAUD1));		//�趨��ʱ��ֵ
			T2H = (65536 - (FOSC/4/BAUD1))>>8;		//�趨��ʱ��ֵ
			AUXR |= 0x10;		//������ʱ��2
		#endif

		#if UART1_485		//��ʼ���ɽ���״̬
			RS485_1=0;
		#endif
		#if UART1_RECV || UART1_NONS
			ES=1;			//���������жϣ����ֻ��Ϊͬ�������ã������迪��
		#endif
		EA=1;				//�������жϣ����ֻ��Ϊ�����ã������迪��
	}

	#if UART1_SEND || UART1_NONS
		//ʵʱ����һ���ַ���
		void Uart1_Send(U8 send)
		{
			#if UART1_485
				RS485_1=1;		//����ǰ����Ϊ����״̬
			#endif
		
			#if UART1_NONS		//���Ϊ�첽����
				Uart1TBuf[Uart1Ti]=send;//ѹ�뷢�ͻ�����
				Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;
				if(!Uart1Busy)				//�����æ
				{
					Uart1Busy=1;				//�ķ���״̬Ϊæ
					SBUF=Uart1TBuf[Uart1Ti2];	//���͵�ǰ�ַ�
					Uart1Ti2 = (Uart1Ti2+1) % UART1_MAXTI;	//�ѷ���+1
				}
			#else
				SBUF=send;		//ͬ������
				while(!TI);			//�ȴ�TI���1��˵���Ƿ�����ɣ����жϷ�ʽ
				TI=0;				//����״̬�����ﲢ����ı�P3.3�Ķ˿ڵ�״̬
				
				#if UART1_485
					RS485_1=0;		//������󣬸�Ϊ����״̬
				#endif
			#endif
		}
		//ʵʱ����һ���ַ���ת��16�����ٷ���
		void Uart1_SendHex(U8 send)
		{
			#define hex(n) {if(n>9)n+='A'-10;else n+='0';}
			U8 t;
			#if UART1_485
				RS485_1=1;		//����ǰ����Ϊ����״̬
			#endif
		
			#if UART1_NONS		//���Ϊ�첽����
				t=send>>4;//ȡ��4λ
				hex(t);
				Uart1TBuf[Uart1Ti]=t;//ѹ�뷢�ͻ�����
				Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;
				t=send&0x0f;//ȡ��4λ
				hex(t);
				Uart1TBuf[Uart1Ti]=t;//ѹ�뷢�ͻ�����
				Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;
				Uart1TBuf[Uart1Ti]=' ';
				Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;
				if(!Uart1Busy)				//�����æ
				{
					Uart1Busy=1;				//�ķ���״̬Ϊæ
					SBUF=Uart1TBuf[Uart1Ti2];	//���͵�ǰ�ַ�
					Uart1Ti2 = (Uart1Ti2+1) % UART1_MAXTI;	//�ѷ���+1
				}
			#else
				t=send>>4;//ȡ��4λ
				hex(t);
				SBUF=t;		//ͬ������
				while(!TI);			//�ȴ�TI���1��˵���Ƿ�����ɣ����жϷ�ʽ
				TI=0;
								//����״̬�����ﲢ����ı�P3.3�Ķ˿ڵ�״̬
				t=send&0x0f;//ȡ��4λ
				hex(t);
				SBUF=t;
				while(!TI);
				TI=0;
				
				#if UART1_485
					RS485_1=0;		//������󣬸�Ϊ����״̬
				#endif
			#endif
		}
		//ʵʱ����һ���ַ�����
		void Uart1_SendStr(U8 *send)
		{
			if(*send)//�ַ�����Ϊ��
			{
				#if UART1_485
					RS485_1=1;		//����ǰ����Ϊ����״̬
				#endif
			
				#if UART1_NONS		//���Ϊ�첽����
					while(*send)
					{
						Uart1TBuf[Uart1Ti]=*send++;//ѹ�뷢�ͻ�����
						Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;
						if(!Uart1Busy)				//�����æ
						{
							Uart1Busy=1;				//�ķ���״̬Ϊæ
							SBUF=Uart1TBuf[Uart1Ti2];	//���͵�ǰ�ַ�
							Uart1Ti2 = (Uart1Ti2+1) % UART1_MAXTI;	//�ѷ���+1
						}
					}
				#else
					while(*send)
					{
						SBUF=*send++;		//ͬ������
						while(!TI);			//�ȴ�TI���1��˵���Ƿ�����ɣ����жϷ�ʽ
						TI=0;				//����״̬�����ﲢ����ı�P3.3�Ķ˿ڵ�״̬
					
						#if UART1_485
							RS485_1=0;		//������󣬸�Ϊ����״̬
						#endif
					}
				#endif
			}
		}
		//ʵʱ����һ��������ʾ000~255�����֣�������ʾIP��ַ
		void Uart1_SendNum(U8 send)
		{
			#define SENDNUM1(n) {Uart1TBuf[Uart1Ti]=n+'0';Uart1Ti = (Uart1Ti+1) % UART1_MAXTI;}//�첽����
			#define SENDNUM2(n)	{SBUF=n+'0';while(!TI);TI=0;}//ͬ������
			#if UART1_485
				RS485_1=1;		//����ǰ����Ϊ����״̬
			#endif
		
			#if UART1_NONS		//���Ϊ�첽����
				if(send>99)SENDNUM1(send/100);
				if(send>9)SENDNUM1(send/10%10);
				SENDNUM1(send%10);
				if(!Uart1Busy)//�����æ
				{
					Uart1Busy=1;				//�ķ���״̬Ϊæ
					SBUF=Uart1TBuf[Uart1Ti2];	//���͵�ǰ�ַ�
					Uart1Ti2 = (Uart1Ti2+1) % UART1_MAXTI;	//�ѷ���+1
				}
			#else
				if(send>99)SENDNUM2(send/100);
				if(send>9)SENDNUM2(send/10%10);
				SENDNUM2(send%10);
			
				#if UART1_485
					RS485_1=0;		//������󣬸�Ϊ����״̬
				#endif
			#endif
		}
	#endif

	#if UART1_RECV
		//����һ���ֽڣ��ڽ���֮ǰ����if(ifRecv())���ж���������
		U8 Uart1_Recv()
		{
			U8 t;
			t=Uart1RBuf[Uart1Ri2];
			Uart1Ri2 = (Uart1Ri2+1)%UART1_MAXRI;
			return t;
		}
	#endif
    #if UART1_RECV || UART1_NONS		
		/*����ͨѶ�жϣ��շ���ɶ��������ж�*/
		void UART1_ISR(void) interrupt 4	  //0�Ŵ����ж�
		{
			if(RI)									  //sbit RI    = SCON^0;
			{
				RI=0;//���մ�����ɺ������㣬��ʾ����������
				#if UART1_RECV
					//����һ���ֽ�
					Uart1RBuf[Uart1Ri]=SBUF;	//���յ����ַ���ѹ��ѭ��������
					Uart1Ri=(Uart1Ri+1)%UART1_MAXRI;	//���ջ������һλ
				#endif
			}

			if(TI)
			{
				TI=0;
				#if UART1_NONS			//ֻ�е��첽����ʱ�������ж����޸�TI��ֵ
					if(TING1)		//�������������Ҫ����
					{
						SBUF=Uart1TBuf[Uart1Ti2];	//���͵�ǰ�ַ�
						Uart1Ti2 = (Uart1Ti2+1) % UART1_MAXTI;	//�ѷ���+1
					}
					else			//ȫ������
					{
						Uart1Busy=0;	//��æ�����Լ�������
						#if UART1_485
							RS485_1=0;		//������󣬸�Ϊ����״̬
						#endif
					}
				#endif
			}
		}
	#endif
#endif

