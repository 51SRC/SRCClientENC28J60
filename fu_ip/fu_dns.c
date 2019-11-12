/********************************************************************/
/* 本程序仅供学习参考，未经作者允许，不得用于任何商业用途           */
/* 技术论坛：www.fuhome.net/bbs/                                    */
/* DNS相关                                                          */
/* 本程序用于硬件设备接入fuhome.net 平台进行远程控制（APP/微信/WEB  */ 
/* 版本：v2.0 2017年3月17日 龙剑奋斗                                */
/* 版本：v3.0 2018年7月21日 乘简                                    */
/* 定期更新，敬请关注——开放，交流，深入，进步                       */
/* Copyright fuhome.net 未来之家 实验室，让科技融入生活             */
/********************************************************************/
#include "fu_dns.h"
#include "fu_config.h"
#include "fu_send.h"
#include <string.h>

//前12字节            ID       tag       numq      numa       numa1    numa2
U8 code DNS_HDR[]={0x12,0x34,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00};

//获取 DNS： dev.fuhome.net
//U8 xdata DNS_DATA[20]={0x03,0x77,0x77,0x77,0x06,0x66,0x75,0x68,0x6f,0x6d,0x65,0x03,0x6e,0x65,0x74,0x00,}/*域名以00结尾，03 06后面字符串的长度*/

//                   type     classes //type为1表示A记录; classes:DNS_HDR+域名+DNS_QER
U8 code DNS_QER[]={0x00,0x01,0x00,0x01};

//U8 xdata DNS_Server[4]={0,0,0,0};//DNS服务器，默认为网关的IP，所以此变量暂不需要
U8 xdata DNS_MAC[]={0,0,0,0,0,0};//DNS的MAC地址，也是网关的MAC

/*******************************************/
/* 功能：复制域名到发送格式                */
/* 输入：输入如“dev.fuhome.net”            */
/* 输出：" \x03dev\x06fuhome\x3net"        */
/* 备注：                                  */
/*******************************************/
void DNS_Copy(U8 Index,U8 *p)
{
	U8 b=1;
	U8 i=0;
	U8 len=2;
	while(*p)
	{
		if(*p=='.')//如果为域名分隔符
		{
			ReHost[Index].Re_DNS[b]=i;//当前
			i=0;//重记本级域名的长度
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
	ReHost[Index].Re_DNS[b]=i;//保存顶级域名的长度
	ReHost[Index].Re_DNS[len]=0;//字符串结束标识
	ReHost[Index].Re_DNS[0]=len;//保存总长度，用于比较
}

/*******************************************/
/* 功能：组建查询包                        */
/* 输入：输入如“dev.fuhome.net”            */
/* 输出：无                                */
/* 备注：                                  */
/*******************************************/
U8 DNS_Request_Copy(U8 Index)
{
	U8 len=ReHost[Index].Re_DNS[0];//包长度

	memcpy(DATA,DNS_HDR,12);//将数据复制到缓冲区
	memcpy(&DATA[12],&ReHost[Index].Re_DNS[1],len);
	memcpy(&DATA[len+12],DNS_QER,4);
	return len+16;
}
