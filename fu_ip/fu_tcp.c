#include "fu_TCP.h"
#include <string.h>
#include "fu_config.h"
/*********************************************************************/
/* 本程序仅供学习参考，未经作者允许，不得用于任何商业用途            */
/* FuIP专为单片机而定制的TCP/IP协议栈，适用于任何8,16,32位单片机     */
/* 主程序，可用于客户端，服务端    					            	 */
/* 技术论坛：www.fuhome.net/bbs/						             */
/* 移植请注意，此程序使用硬件SPI，模拟SPI会有区别，本程序默认是模拟  */
/* 发送各种数据报程序，ARP DHCP Ping TCP（服务端/客户端） UDP        */
/* FuIP可用于，客户/服务，不限连接数，亦可接入fuhome.net 平台进行远程控制*/
/* 版本：v3.1 2018年7月23日						                     */
/* 龙剑奋斗	乘简									       		     */
/* 定期更新，敬请关注——开放，交流，深入，进步	     			     */
/* Copyright fuhome.net 未来之家 实验室，让科技融入生活				 */
/*********************************************************************/


U8  xdata TCP_False[12]={0};//TCP伪首部，12字节
U8  xdata TCP_Head[20]={0};//TCP首部，长度20字节

/*******************************************/
/* 功能：TCP 伪首部填充 12字节      	   */
/* 输入：TCP数据包长度				       */
/* 输出：无							       */
/*******************************************/
void TCP_Flase_Copy(U8 Index,U16 len)//len 为偶数
{
	memcpy(TCP_False,FU_LoaclIP,4);		//源IP
	
	memcpy(&TCP_False[4],ReHost[Index].Re_IP,4);	//目的IP

	TCP_False[8]=0;
	TCP_False[9]=6;//6 TCP
	TCP_False[10]=(20+len)>>8;//TCP首部长+数据长
	TCP_False[11]=(20+len);//TCP首部长+数据长
}

/*******************************************/
/* 功能：TCP 首部填充  20字节       	   */
/* 输入：TCP数据包长度                     */
/* 输出：无							       */
/*******************************************/
void TCP_Head_Copy(U8 Index,U8 Flag)
{
	TCP_Head[0]=ReHost[Index].Lc_PORT>>8;
	TCP_Head[1]=ReHost[Index].Lc_PORT;//源端口
	TCP_Head[2]=ReHost[Index].Re_PORT>>8;
	TCP_Head[3]=ReHost[Index].Re_PORT;//目的端口
	memcpy(&TCP_Head[4],(U8 xdata *)&ReHost[Index].TCP_Mark,4);//TCP序号
	memcpy(&TCP_Head[8],(U8 xdata *)&ReHost[Index].TCP_CMark,4);//确认序号
	TCP_Head[12]=0x50;//TCP首部长
	TCP_Head[13]=Flag;//TCP6个位标志
	TCP_Head[14]=(FU_LEN-58)>>8;
	TCP_Head[15]=(FU_LEN-58);//TCP窗口大小 FU_LEN-14-20-20-4=最大能收的TCP数据
	TCP_Head[16]=0;
	TCP_Head[17]=0;//检验和
	TCP_Head[18]=0;
	TCP_Head[19]=0;//紧急指针
}

/********************************************/
/* 功能：检验和处理函数           			*/
/* 输入：无									*/
/* 输出：16位TCP检验和						*/
/********************************************/

U16 TCP_Check_Code(U16 len)
{
    U32 tem;

	/* 计算检验和 */
	tem=FU_Check_Sum((U16 xdata *)TCP_False,12);//TCP伪首部12
	tem+=FU_Check_Sum((U16 xdata *)TCP_Head,20);//TCP首部20
	tem+=FU_Check_Sum((U16 xdata *)DATA,len);//TCP首部20
    return FU_Check_Code(tem);//计算溢出位
}

/********************************************/
/* 功能：组TCP数据包函数           			*/
/* 输入：TCP 数据长度 len 		 			*/
/* 输出：无									*/
/********************************************/
void TCP_Send(U8 Index,U8 lx,U16 len)
{
	U16 tem;
	//填充TCP伪首部 12字节
	TCP_Flase_Copy(Index,len);//len 为偶数
	//TCP 首部填充  20字节
	TCP_Head_Copy(Index,lx);
	
	tem=TCP_Check_Code(len);//得到检验和
	
	TCP_Head[16]=tem>>8;
	TCP_Head[17]=tem;//检验和稍后补充
}

