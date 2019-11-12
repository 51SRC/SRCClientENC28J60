#include "fu_udp.h"
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


U8 xdata UDP_False[12]={0};//UDP伪首部，12字节
U8 xdata UDP_Head[8]={0};//UDP首部，长度8字节

/*******************************************/
/* 功能：UDP 伪首部填充 12字节      	   */
/* 输入：UDP数据包长度				       */
/* 输出：无							       */
/*******************************************/
void UDP_Flase_Copy(U8 Index,U8 lx,U16 len)//len 为偶数
{
	memcpy(UDP_False,FU_LoaclIP,4);		//源IP

	if(lx)//组DNS包
		memcpy(&UDP_False[4],FU_GateWay,4);	//目的IP
	else
		memcpy(&UDP_False[4],ReHost[Index].Re_IP,4);	//目的IP
	
	UDP_False[8]=0;
	UDP_False[9]=17;
	UDP_False[10]=(8+len)>>8;	//UDP首部长+数据长
	UDP_False[11]=(8+len);		//UDP首部长+数据长
}

/*******************************************/
/* 功能：UDP 首部填充  8字节       		   */
/* 输入：UDP数据包长度                     */
/* 输出：无							       */
/*******************************************/
void UDP_Head_Copy(U8 Index,U8 lx,U16 len)
{
	UDP_Head[0]=ReHost[Index].Lc_PORT>>8;//源端口
	UDP_Head[1]=ReHost[Index].Lc_PORT;
	if(lx){//组DNS包
		UDP_Head[2]=0;//目的端口固定为53
		UDP_Head[3]=53;
	}else{
		UDP_Head[2]=ReHost[Index].Re_PORT>>8;//目的端口
		UDP_Head[3]=ReHost[Index].Re_PORT;
	}
	UDP_Head[4]=(len+8)>>8;
	UDP_Head[5]=(len+8);//UDP长度=UDP首部+数据
	UDP_Head[6]=0;
	UDP_Head[7]=0;//检验和稍后补充
}

/********************************************/
/* 功能：检验和处理函数           			*/
/* 输入：无									*/
/* 输出：16位UDP检验和						*/
/********************************************/
U16 UDP_Check_Code(U16 len)
{
    U32 tem;

	/* 计算检验和 */
	tem=FU_Check_Sum((U16 xdata *)UDP_False,12);//UDP伪首部12
	tem+=FU_Check_Sum((U16 xdata *)UDP_Head,8);//UDP首部8
	tem+=FU_Check_Sum((U16 xdata *)DATA,len);//数据长度
    return FU_Check_Code(tem);//计算溢出位
}

/********************************************/
/* 功能：组UDP数据包函数           			*/
/* 输入：lx 1为DNS，0为UDP     			    */
/* 输入：UDP 数据长度 len      			    */
/* 输出：无									*/
/********************************************/
void UDP_Send(U8 Index,U8 lx,U16 len)
{
	U16 tem;
	//填充UDP伪首部 12字节
	UDP_Flase_Copy(Index,lx,len);//len 为偶数
	//UDP 首部填充  8字节
	UDP_Head_Copy(Index,lx,len);
	
	tem=UDP_Check_Code(len);//得到检验和

	UDP_Head[6]=tem>>8;
	UDP_Head[7]=tem;//检验和稍后补充
}

