#ifndef __UART_H__
#define __UART_H__
#include "mcuinit.h"
#include "fu_config.h"

#if DEBUG
	#define UART1 1			//Ĭ�Ͽ�������1�������иĶ�
#else
	#define UART1 0//������
#endif

#define UART1_485 0		//����1�Ƿ�485ͨ��
#if UART1_485
	sbit RS485_1=P3^6;	//����485��ʹ�ܽ�
//	sbit RS485_1=P3^4;	//�Ͽ�
//	sbit RS485_1=P3^3;	//��ʪ�ȿ�
#endif
#define BAUD1 115200		//����1�����ʣ���ÿ�뷢��BAUD��λ����������ڶ��ö��������ʷ�������������Ҫ���һ��
#define UART1_NONS 1	//�첽���ͣ���ͬ�����Ͷ���ѡ��һ
#define UART1_SEND 0	//ͬ�����ͣ�����ͬʱΪ1��Ϊ�첽����
#define UART1_RECV 0	//�������
#define UART1_MAXRI 50	//����1�����ջ�����
#define UART1_MAXTI 500	//����1�����ջ�����

#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7

#if UART1
	extern U8 xdata Uart1RBuf[UART1_MAXRI];		//���ջ�����
	#if UART1_MAXRI>255
		extern U16 xdata Uart1Ri,Uart1Ri2;		//Uart1RiΪʣ����ո������ѽ��ո���
	#else
		extern U8 xdata Uart1Ri,Uart1Ri2;		//Uart1RiΪʣ����ո������ѽ��ո���
	#endif
	extern U8 xdata Uart1TBuf[UART1_MAXTI];		//���ͻ�����
	#if UART1_MAXTI>255
		extern U16 xdata Uart1Ti,Uart1Ti2;		//���ͼ���
	#else
		extern U8 xdata Uart1Ti,Uart1Ti2;		//���ͼ���
	#endif
	void Uart1_Init(void);	//����1��ʼ��
	#define TING1 (Uart1Ti!=Uart1Ti2)		//������ַ��ɷ���
	void Uart1_SendStr(U8 *send);	//ʵʱ����һ���ַ�
	void Uart1_Send(U8 send);	//ʵʱ����һ���ַ�
	void Uart1_SendHex(U8 send);//ת��16���Ʒ���һ���ַ�
	void Uart1_SendNum(U8 send);
	#define Uart1_SendFlag(i,s) {Uart1_SendNum(i);Uart1_SendStr(s);Uart1_SendStr("\r\n");}
	#define Uart1_SendIP(i1,i2,i3,i4) {Uart1_SendStr("IP:");Uart1_SendNum(i1);Uart1_Send('.');Uart1_SendNum(i2);Uart1_Send('.');Uart1_SendNum(i3);Uart1_Send('.');Uart1_SendNum(i4);Uart1_SendStr("\r\n");}
	#define RING1 (Uart1Ri!=Uart1Ri2)		//�Ѿ����յ����ݣ���������ĺ�������
	U8 Uart1_Recv();	//����һ���ַ�
#endif

#endif
