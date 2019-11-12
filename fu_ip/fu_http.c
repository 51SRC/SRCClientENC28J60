#include "fu_http.h"
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

char* pSend;//发送指针

/* 数据包 */
U8 code fu_get[109]={
"GET / HTTP/1.0\r\n"
"Host: baidu.com\r\n"
"Connection: close\r\n"
"Accept: text/html\r\n"
"User-Agent: Fuhome (STC12 MCU)\r\n"
"\r\n\r\n"};



U8 code fu_index[253]={//413
"HTTP/1.1 200 OK\r\n"
"Server: Fuhome server\r\n"
"love: only\r\n"
"Content-type: text/html\r\n\r\n"

"<html><body><center>"									  
"<h3>Fuhome.net FuIP2.2</h3>"
"<li><a href='open.html' target='view'>open</a></li>"
"<li><a href='close.html' target='view'>close</a></li>"
"</center></body></html>"
};
//[内20个字符地址从fu_main[353]开始，作为显示字符]


U8 code fu_open[120]={//72
"HTTP/1.1 200 OK\r\n"
"Server: Fuhome server 2.2\r\n"
"love: only\r\n"
"Content-type: text/html\r\n\r\n"

"<html><body>LED opened!</body></html>"

};

U8 code fu_close[119]={//83
"HTTP/1.1 200 OK\r\n"
"Server: Fuhome server 2.2\r\n"
"love: only\r\n"
"Content-type: text/html\r\n\r\n"

"<html><body>LED closd!</body></html>"

};

