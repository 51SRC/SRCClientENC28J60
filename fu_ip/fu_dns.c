/********************************************************************/
/* ���������ѧϰ�ο���δ�������������������κ���ҵ��;           */
/* ������̳��www.fuhome.net/bbs/                                    */
/* DNS���                                                          */
/* ����������Ӳ���豸����fuhome.net ƽ̨����Զ�̿��ƣ�APP/΢��/WEB  */ 
/* �汾��v2.0 2017��3��17�� �����ܶ�                                */
/* �汾��v3.0 2018��7��21�� �˼�                                    */
/* ���ڸ��£������ע�������ţ����������룬����                       */
/* Copyright fuhome.net δ��֮�� ʵ���ң��ÿƼ���������             */
/********************************************************************/
#include "fu_dns.h"
#include "fu_config.h"
#include "fu_send.h"
#include <string.h>

//ǰ12�ֽ�            ID       tag       numq      numa       numa1    numa2
U8 code DNS_HDR[]={0x12,0x34,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00};

//��ȡ DNS�� dev.fuhome.net
//U8 xdata DNS_DATA[20]={0x03,0x77,0x77,0x77,0x06,0x66,0x75,0x68,0x6f,0x6d,0x65,0x03,0x6e,0x65,0x74,0x00,}/*������00��β��03 06�����ַ����ĳ���*/

//                   type     classes //typeΪ1��ʾA��¼; classes:DNS_HDR+����+DNS_QER
U8 code DNS_QER[]={0x00,0x01,0x00,0x01};

//U8 xdata DNS_Server[4]={0,0,0,0};//DNS��������Ĭ��Ϊ���ص�IP�����Դ˱����ݲ���Ҫ
U8 xdata DNS_MAC[]={0,0,0,0,0,0};//DNS��MAC��ַ��Ҳ�����ص�MAC

/*******************************************/
/* ���ܣ��������������͸�ʽ                */
/* ���룺�����硰dev.fuhome.net��            */
/* �����" \x03dev\x06fuhome\x3net"        */
/* ��ע��                                  */
/*******************************************/
void DNS_Copy(U8 Index,U8 *p)
{
	U8 b=1;
	U8 i=0;
	U8 len=2;
	while(*p)
	{
		if(*p=='.')//���Ϊ�����ָ���
		{
			ReHost[Index].Re_DNS[b]=i;//��ǰ
			i=0;//�ؼǱ��������ĳ���
			b=len;
		}
		else
		{
			ReHost[Index].Re_DNS[len]=*p;
			i++;
		}
		p++;
		len++;
	}
	ReHost[Index].Re_DNS[b]=i;//���涥�������ĳ���
	ReHost[Index].Re_DNS[len]=0;//�ַ���������ʶ
	ReHost[Index].Re_DNS[0]=len;//�����ܳ��ȣ����ڱȽ�
}

/*******************************************/
/* ���ܣ��齨��ѯ��                        */
/* ���룺�����硰dev.fuhome.net��            */
/* �������                                */
/* ��ע��                                  */
/*******************************************/
U8 DNS_Request_Copy(U8 Index)
{
	U8 len=ReHost[Index].Re_DNS[0];//������

	memcpy(DATA,DNS_HDR,12);//�����ݸ��Ƶ�������
	memcpy(&DATA[12],&ReHost[Index].Re_DNS[1],len);
	memcpy(&DATA[len+12],DNS_QER,4);
	return len+16;
}
