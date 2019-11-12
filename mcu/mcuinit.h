#ifndef __STC_H__
#define __STC_H__
#include <intrins.h>
/*********************************************************************/
/* ���������ѧϰ�ο���δ�������������������κ���ҵ��;            */
/* FuIPרΪ��Ƭ�������Ƶ�TCP/IPЭ��ջ���������κ�8,16,32λ��Ƭ��     */
/* �����򣬿����ڿͻ��ˣ������    					            	 */
/* ������̳��www.fuhome.net/bbs/						             */
/* ��ֲ��ע�⣬�˳���ʹ��Ӳ��SPI��ģ��SPI�������𣬱�����Ĭ����ģ��  */
/* ���͸������ݱ�����ARP DHCP Ping TCP�������/�ͻ��ˣ� UDP        */
/* FuIP�����ڣ��ͻ�/���񣬲�������������ɽ���fuhome.net ƽ̨����Զ�̿���*/
/* �汾��v3.1 2018��7��23��						                     */
/* �����ܶ�	�˼�									       		     */
/* ���ڸ��£������ע�������ţ����������룬����	     			     */
/* Copyright fuhome.net δ��֮�� ʵ���ң��ÿƼ���������				 */
/*********************************************************************/

typedef char I8;
typedef int I16;
typedef long I32;
typedef unsigned char U8; 
typedef unsigned int U16;
typedef unsigned long int U32;
typedef volatile char V8;
typedef volatile int V16;
typedef volatile long V32;
typedef volatile unsigned char VU8;
typedef volatile unsigned int VU16;
typedef volatile unsigned long VU32;

#define STC89C  0			//89C52��Ƭ��
#define STC12C  1			//12C5A��Ƭ��
#define STC15W  2			//15W4K��Ƭ��
#define MCU_TYPE STC15W		//������ĵ�Ƭ������

#define FOSC    11059200L		//����Ƶ�ʣ�ÿ���񵴴���
//#define FOSC    22118400L		//����Ƶ�ʣ�ÿ���񵴴���
//#define FOSC	30000000L		//����Ƶ�ʣ�ÿ���񵴴���

#define FOSC_12 FOSC / 12		//��ʱ��0 ����12��Ƶ

#if MCU_TYPE==STC89C
	#include <REG52.H>
#endif

#if MCU_TYPE==STC12C
	#include <STC/STC12C5A60S2.H>
#endif

#if MCU_TYPE==STC15W
	#include <STC/stc15.h>
#endif
	 
#endif
