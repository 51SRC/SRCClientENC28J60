#ifndef _FU_DNS_H_
#define _FU_DNS_H_
#include "mcuinit.h"
/*********************************************************************/
/* ���������ѧϰ�ο���δ���������������������κ���ҵ��;            */
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

extern U8 xdata DNS_MAC[];//����MAC��ַ�����ڲ�ѯ������Ӧ��IP

/*******************************************/
/* ���ܣ��������������͸�ʽ                */
/* ���룺�����硰dev.fuhome.net��            */
/* �����" \x03dev\x06fuhome\x3net"        */
/* ��ע��                                  */
/*******************************************/
void DNS_Copy(U8 Index,U8 *p);

/*******************************************/
/* ���ܣ��齨��ѯ��                        */
/* ���룺�����硰dev.fuhome.net��            */
/* �������                                */
/* ��ע��                                  */
/*******************************************/
U8 DNS_Request_Copy(U8 Index);

#endif