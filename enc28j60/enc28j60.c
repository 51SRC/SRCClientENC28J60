#include "enc28j60.h"
#include "fu_config.h"
#include "spi.h"
#include "fu_enthernet.h"
#include "fu_arp.h"
#include "fu_ip.h"
#include "fu_udp.h"
#include "fu_tcp.h"
#include "fu_icmp.h"
#include "timer.h"
#if DEBUG
	#include "uart.h"
#endif
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

idata U8 Enc28j60Bank;
idata U16 NextPacketPtr;
U8 Init_Num=0;			//记录复位次数

/***************************** ENC28J60相关程序 *****************************/

/*******************************************/
/* 功能：ENC28J60读操作                    */
/* 输入：无                                */
/* 输出：无							       */
/*******************************************/
U8 enc28j60ReadOp(U8 op, U8 address)	  //读控制寄存器
{
	U8 dat1;
	//片选	
	CSN =0;
	// issue read command
	WriteByte(op | (address & ADDR_MASK));	
	dat1 = ReadByte();
	// do dummy read if needed (for mac and mii, see datasheet page 29)
	if(address & 0x80) 	dat1 = ReadByte();
	//释放片选
	CSN=1;
	return(dat1);
}
/*******************************************/
/* 功能：ENC28J60写操作                    */
/* 输入：无                                */
/* 输出：无							       */
/*******************************************/
void enc28j60WriteOp(U8 op, U8 address, U8 mydat)	 //写控制寄存器
{
	CSN=0;
	// issue write command
	WriteByte( op | (address & ADDR_MASK));	//写命令
	// write data
	WriteByte(mydat);
	CSN=1;
}
//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
void enc28j60SetBank(U8 address)
{
	// set the bank (if needed)
	if((address & BANK_MASK) != Enc28j60Bank)
	{
		// set the bank
		enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		Enc28j60Bank = (address & BANK_MASK);
	}
}
//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
U8 enc28j60Read(U8 address)
{
	// select bank to read
	enc28j60SetBank(address);	
	// do the read
	return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}
//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
void enc28j60Write(U8 address,U8 mydat)
{
	// select bank to write
	enc28j60SetBank(address);

	// do the write
	enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, mydat);
}
////*******************************************************************************************
////
//// Function : icmp_send_request
//// Description : Send ARP request packet to destination.
////
////*******************************************************************************************
//U16 enc28j60Phyread(U8 address)
//{
//	U16 mydat;
//	
//	// set the PHY register address
//	enc28j60Write(MIREGADR, address);
//	enc28j60Write(MICMD, MICMD_MIIRD);
//	
//	// Loop to wait until the PHY register has been read through the MII
//	// This requires 10.24us
//	while( (enc28j60Read(MISTAT) & MISTAT_BUSY) );
//	
//	// Stop reading	这里应该清零
//	//enc28j60Write(MICMD, MICMD_MIIRD);
//	enc28j60Write(MICMD, 0x0);   //将MICMD.MIIRD 位清零。
//
//	// Obtain results and return
//	mydat = enc28j60Read ( MIRDL );
//	mydat |= (enc28j60Read ( MIRDH )<<8);
//
//	return mydat;
//}

//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
void enc28j60PhyWrite(U8 address, U16 mydat)
{
	unsigned char t;
	// set the PHY register address
	enc28j60Write(MIREGADR, address);
	// write the PHY data
	enc28j60Write(MIWRL, mydat & 0x00ff);
	enc28j60Write(MIWRH, mydat >> 8);
	// wait until the PHY write completes
	t=Tim_10Ms;
	while((enc28j60Read(MISTAT) & MISTAT_BUSY) && t==Tim_10Ms);
//	{
//		Delay10ms();
//	}
}

/*******************************************/
/* 功能：ENC28J60读缓冲区程序              */
/* 输入：无                                */
/* 输出：无							       */
/*******************************************/
void enc28j60ReadBuffer(U16 len, U8* dat)
{
// assert CS
//	ENC28J60_CONTROL_PORT &= ~(1<<ENC28J60_CONTROL_CS);
    CSN = 0;
    // issue read command
	//SPDR = ENC28J60_READ_BUF_MEM;
	WriteByte(ENC28J60_READ_BUF_MEM);
    //while(!(SPSR & (1<<SPIF)));
    while(len--)
    {
        // read data
		//SPDR = 0x00;
        //while(!(SPSR & (1<<SPIF)));
        //*dat++ = SPDR;
		*dat++ = ReadByte();
    }	
    // release CS
	//ENC28J60_CONTROL_PORT |= (1<<ENC28J60_CONTROL_CS);
	CSN = 1;
}

/*******************************************/
/* 功能：ENC28J60写缓冲区程序              */
/* 输入：无                                */
/* 输出：无							       */
/*******************************************/

void enc28j60WriteBuffer(U16 len, U8* dat)
{
    // assert CS
	//ENC28J60_CONTROL_PORT &= ~(1<<ENC28J60_CONTROL_CS);
	CSN = 0;//使能
    
    // issue write command
	//SPDR = ENC28J60_WRITE_BUF_MEM;
	WriteByte(ENC28J60_WRITE_BUF_MEM);
   // while(!(SPSR & (1<<SPIF)));
    while(len--)
    {
        // write data
		//SPDR = *dat++;
        //while(!(SPSR & (1<<SPIF)));
		WriteByte(*dat++);
    }	
    // release CS
//	ENC28J60_CONTROL_PORT |= (1<<ENC28J60_CONTROL_CS);
	CSN = 1;
}



/*******************************************/
/* 功能：ENC28J60发送ARP数据包程序         */
/* 输入：无								   */
/* 输出：无							       */
/*******************************************/
void ARP_Packet_Send(void)
{
	// 设置写指针开始的传输缓冲区域
	enc28j60Write(EWRPTL, TXSTART_INIT);
    enc28j60Write(EWRPTH, TXSTART_INIT>>8);

    // 设置TXND指向对应于给定的数据包大小
	enc28j60Write(ETXNDL, (TXSTART_INIT+42));//14为以太网头+28 ARP数据包长度
    enc28j60Write(ETXNDH, (TXSTART_INIT+42)>>8);

    // 写每个包的控制字
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

    // TODO, fix this up

	
	    // 以太网头 到传输缓冲
		enc28j60WriteBuffer(14, EN_Head);
		// ARP数据包 到传输缓冲
		enc28j60WriteBuffer(28, ARP_Data);
	    

	// 将以太网控制寄存器ECON1所有位 置1，以发送缓冲区数据
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}



/*******************************************/
/* 功能：ENC28J60发送UDP数据包程序         */
/* 输入：UDP数据长度len	     			   */
/* 输出：无							       */
/*******************************************/
void UDP_Packet_Send(U16 len)
{
	// 设置写指针开始的传输缓冲区域
	enc28j60Write(EWRPTL, TXSTART_INIT);
    enc28j60Write(EWRPTH, TXSTART_INIT>>8);

    // 设置TXND指向对应于给定的数据包大小
	enc28j60Write(ETXNDL, (TXSTART_INIT+42+len));//14为以太网头+20IP头+8UDP头+len数据包长度
    enc28j60Write(ETXNDH, (TXSTART_INIT+42+len)>>8);

    // 写每个包的控制字
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

    // TODO, fix this up

    
        // 以太网头 到传输缓冲
		enc28j60WriteBuffer(14, EN_Head);
		// IP头 到传输缓冲
		enc28j60WriteBuffer(20, IP_Head);
		// UDP头 到传输缓冲
		enc28j60WriteBuffer(8, UDP_Head);
		// UDP数据 到传输缓冲
		enc28j60WriteBuffer(len, DATA);
    

	// 将以太网控制寄存器ECON1所有位 置1，以发送缓冲区数据
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

/*******************************************/
/* 功能：ENC28J60发送TCP数据包程序         */
/* 输入：TCP数据长度len      			   */
/* 输出：无							       */
/*******************************************/
void TCP_Packet_Send(U16 len)
{
	// 设置写指针开始的传输缓冲区域
	enc28j60Write(EWRPTL, TXSTART_INIT);
    enc28j60Write(EWRPTH, TXSTART_INIT>>8);

    // 设置TXND指向对应于给定的数据包大小
	enc28j60Write(ETXNDL, (TXSTART_INIT+54+len));//14为以太网头+20IP头+20TCP头+len数据包长度
    enc28j60Write(ETXNDH, (TXSTART_INIT+54+len)>>8);

    // 写每个包的控制字
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

    // TODO, fix this up

    
        // 以太网头 到传输缓冲
		enc28j60WriteBuffer(14, EN_Head);
		// IP头 到传输缓冲
		enc28j60WriteBuffer(20, IP_Head);
		// UDP头 到传输缓冲
		enc28j60WriteBuffer(20, TCP_Head);
		// UDP数据 到传输缓冲

		enc28j60WriteBuffer(len,DATA);

    

	// 将以太网控制寄存器ECON1所有位 置1，以发送缓冲区数据
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

/*******************************************/
/* 功能：ENC28J60发送ICMP_Ping数据包程序   */
/* 输入：无			   					   */
/* 输出：无							       */
/*******************************************/
void ICMP_Ping_Packet_Send()//发送ICMP_Ping包
{
	// 设置写指针开始的传输缓冲区域
	enc28j60Write(EWRPTL, TXSTART_INIT);
    enc28j60Write(EWRPTH, TXSTART_INIT>>8);

    // 设置TXND指向对应于给定的数据包大小
	enc28j60Write(ETXNDL, (TXSTART_INIT+74));//14为以太网头+20IP头+40ICMP_Ping数据包长度
    enc28j60Write(ETXNDH, (TXSTART_INIT+74)>>8);

    // 写每个包的控制字
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

    // TODO, fix this up

    
        // 以太网头 到传输缓冲
		enc28j60WriteBuffer(14, EN_Head);
		// IP头 到传输缓冲
		enc28j60WriteBuffer(20, IP_Head);
		// ICMP 到传输缓冲
		enc28j60WriteBuffer(40, ICMP);


	// 将以太网控制寄存器ECON1所有位 置1，以发送缓冲区数据
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}



//ETH包最小长度60个
#define ETHERNET_MIN_PACKET_LENGTH	0x3C
//ETH头长度	14
#define ETHERNET_HEADER_LENGTH		0x0E
//IP+TCP头长度 20+ TCP=20,UDP=8
#define IP_TCP_HEADER_LENGTH 40
#define IP_UDP_HEADER_LENGTH 28
//IP+TCP+ETH 头长度
#define TOTAL_TCP_HEADER_LENGTH (IP_TCP_HEADER_LENGTH+ETHERNET_HEADER_LENGTH)
#define TOTAL_UDP_HEADER_LENGTH (IP_UDP_HEADER_LENGTH+ETHERNET_HEADER_LENGTH)

///*******************************************/
///* 功能：ENC28J60发送数据包程序            */
///* 输入：leixing 0 TCP 1 UDP,len,数据包地址*/
///* 输出：无							       */
///*******************************************/
//void enc28j60PacketSend(U8 leixing,U16 len,U8* packet)
//{
////    /* 查询发送逻辑复位位 */  
////    while((enc28j60Read(ECON1) & ECON1_TXRTS)!= 0);  
//
//	// 设置写指针开始的传输缓冲区域
//	enc28j60Write(EWRPTL, TXSTART_INIT);
//    enc28j60Write(EWRPTH, TXSTART_INIT>>8);
//
//    // 设置TXND指向对应于给定的数据包大小
//	enc28j60Write(ETXNDL, (TXSTART_INIT+len));
//    enc28j60Write(ETXNDH, (TXSTART_INIT+len)>>8);
//
//    // 写每个包的控制字
//	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
//
//    // TODO, fix this up
//
//    if( FU_LEN <= TOTAL_UDP_HEADER_LENGTH-42 )//如果长度小于这长度,不明觉厉
//    {
//        // 复制包到传输缓冲
//		enc28j60WriteBuffer(len, packet);
//    }
//    else
//    {
//		// 写入头
//		if(leixing==0)
//        {
//			enc28j60WriteBuffer(TOTAL_TCP_HEADER_LENGTH, packet);
//			len -= TOTAL_TCP_HEADER_LENGTH;
//		}
//		if(leixing==1)
//		{
//			enc28j60WriteBuffer(TOTAL_UDP_HEADER_LENGTH, packet);
//			len -= TOTAL_UDP_HEADER_LENGTH;
//		}
//		// 写入数据
//        enc28j60WriteBuffer(len, (U8 *)DATA);
//    }
//
//	// 将以太网控制寄存器ECON1所有位 置1，以发送缓冲区数据
//	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
//}

/*******************************************/
/* 功能：ENC28J60接收数据包程序            */
/* 输入：最大maxlen,存储数据包地址         */
/* 输出：数据长度						   */
/* 最后修改：2016.06.05 QQ:18407088(乘简)  */
/*******************************************/
U16 enc28j60PacketReceive(U16 maxlen, U8* packet)	 //接收数据包函数，最大长度 数据
{
    static xdata U16 NextPackLen=0,ReadedPackLen=0;		//下次是否继续接收
	static idata U16 CurrentPacketPtr;	  	//保存当前包的位置
	U16 rxstat;
    U16 len;
	U8 i=0;

	if(NextPackLen==0)		//如果接收的包在缓存范围内
	{
		// 检查缓冲是否一个包已经收到
		if( !(enc28j60Read(EIR) & EIR_PKTIF) )
	    {
	        // 通过查看EPKTCNT寄存器再次检查是否收到包
			if (enc28j60Read(EPKTCNT) == 0)
	            return 0;
	    }
	    
		//设置接收到的数据包读指针开始
		enc28j60Write(ERDPTL, (NextPacketPtr));
	    enc28j60Write(ERDPTH, (NextPacketPtr)>>8);
		CurrentPacketPtr=NextPacketPtr;

	    // 下一个封包读指针
		NextPacketPtr  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	    NextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	
	    // 读取包的长度
		len  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	    len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
		len-=4;// 移除CRC字段的长度来减少MAC所报告长度
	
		// 读取接收数据包的状态
		rxstat  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
		rxstat |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	    //if ((rxstat & 0x80)==0)	return 0; 	    // invalid
	
	    // 计算实际数据长度
		if(len>maxlen)//如果收到的包的长度，大于接收缓存的长度
		{
			NextPackLen=len-maxlen;	 		//剩余字节
			len=maxlen;
			// copy the packet from the receive buffer
		    enc28j60ReadBuffer(len, packet);
			packet[16]=(len-14)>>8;//修改IP包已接收包长度,这里只有接收UDP包时才会出现超出长度
			packet[17]=len-14;//,不用担心TCP,因为TCP有窗大小限制,不会超出长度
		}
		else
		{
			NextPackLen=0;
			// copy the packet from the receive buffer
		    enc28j60ReadBuffer(len, packet);
		}

		ReadedPackLen=len;		//已读长度

	}
	else 		//如果是上次没有收完的包
	{
		if(NextPackLen>maxlen-42)		//减掉UDP包头42字节，TCP无此问题
		{
			NextPackLen=NextPackLen-(maxlen-42);
			len=maxlen-42;
		}
		else
		{
			len=NextPackLen;
			NextPackLen=0;
		}

		//设置接收到的数据包读指针开始
		enc28j60Write(ERDPTL, (CurrentPacketPtr+6));	   	//跳过前面6字节的长度信息
	    enc28j60Write(ERDPTH, (CurrentPacketPtr+6)>>8);
		enc28j60ReadBuffer(42, packet);		//读出包头，目前支持UDP包头为42字节，TCP不会出现这种情况
		packet[16]=(len+28)>>8; 		//修改IP包已接收包长度,这里只有接收UDP包时才会出现超出长度
		packet[17]=len+28;				//,不用担心TCP,因为TCP有窗大小限制,不会超出长度

		enc28j60Write(ERDPTL, (CurrentPacketPtr+6+ReadedPackLen));	   	//跳到上次读完的位置处开始读
	    enc28j60Write(ERDPTH, (CurrentPacketPtr+6+ReadedPackLen)>>8); 	
		enc28j60ReadBuffer(len, packet+42);		//读出剩下的内容
		ReadedPackLen+=len;

		len+=42;
	}
	
	if(NextPackLen==0)		 		//清除本包所占模块的缓存空间
	{
	    // ERXRDPT读缓冲器指针
		// ENC28J60将一直写到该指针之前的一单元为止
	    enc28j60Write(ERXRDPTL, (NextPacketPtr));
	    enc28j60Write(ERXRDPTH, (NextPacketPtr)>>8);
	    // Errata workaround #13. Make sure ERXRDPT is odd

	    // 数据包个数递减位EPKTCNT减1
		enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);  
	}

    return len;
}

/*******************************************/
/* 功能：ENC28J60芯片初始化程序            */
/* 输入：无                                */
/* 输出：无							       */
/* 最后修改：2016-06-14 QQ:18407088(乘简)  */
/*******************************************/
void enc28j60_init(void)   	//此函数必须要放在Timer_Init()后面，要不然初始化不成功还会卡住
{
	unsigned char t;
	//*****Bank1区相关寄存器配置            SPI操作块         数据块
	//初始化程序一开始先进行软件复位，111<操作码>+11111<参数>， N/A
	// ENC28J60_SOFT_RESET=0xFF
	enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
//	Delay10ms();
	/*这里要注意，如果单片机大于5V电压很多，就会复位不成功，然后造成死循环，
	因为模块ENC28j60是3.3V的，本人买的模块也可以支持5V单片机，但在5.17V以上时死活复位不成功
	后来降到5V则一点问题都没有，后来加了整流桥后也没问题，
	不过到底是大于多少V后复位不成功，还有待测试*/
	t=Tim_Sec;
	while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY) && t==Tim_Sec);	//等待复位成功

    // do bank 0 stuff
	// initialize receive buffer
	// 16-bit transfers, must write low byte first

    // 初始化接收缓冲区，设置接收起始地址
	NextPacketPtr = RXSTART_INIT;	 // 读下一数据包指针
    enc28j60Write(ERXSTL, RXSTART_INIT&0xFF);
    enc28j60Write(ERXSTH, RXSTART_INIT>>8);

    // 设置接收读指针指向地址
	enc28j60Write(ERXRDPTL, RXSTART_INIT&0xFF);
    enc28j60Write(ERXRDPTH, RXSTART_INIT>>8);

 	// 设置接收缓冲区的末尾地址
	// ERXND寄存器默认指向整个缓冲区的最后一个单元
	enc28j60Write(ERXNDL, RXSTOP_INIT&0xFF);
    enc28j60Write(ERXNDH, RXSTOP_INIT>>8);

	// 设置发送缓冲区的起始地址
	// ETXST寄存器默认地址是整个缓冲区的第一个单元
	enc28j60Write(ETXSTL, TXSTART_INIT&0xFF);
    enc28j60Write(ETXSTH, TXSTART_INIT>>8);

	//不用再初始化发送缓存的最后位置,因为enc28j60会默认到最后的部分全分给发送缓存
//    enc28j60Write(ETXNDL, TXSTOP_INIT&0xFF);  
//    enc28j60Write(ETXNDH, TXSTOP_INIT>>8);  

	/* 使能单播过滤 使能CRC校验 使能*/
	//UCEN:单播过滤,ANDOR:过滤器选择,CRCEN:CRC校验使能
	//PMEN:格式批配过滤器,MPEN:MAC地址过滤器,
	//HTEN:哈希表过滤器,MCEN:组播过滤器,BCEN:广播过滤器
    //enc28j60Write(ERXFCON, ERXFCON_UCEN);//单播过滤,接收包目标地址与本机地址不批配则丢失,如果你要用来做局域网抓包工具,则需要修改此处
    enc28j60Write(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_BCEN);//单播过滤+广播
	/*************************************************/  

  /* 使能MAC接收 允许MAC发送暂停控制帧 当接收到暂停控制帧时停止发送*/  
  /* 数据手册34页 */  
 	// Bank2区相关寄存器配置
 	// MAC初始化配置
	// MAC接收使能，下行程序段表示使能MAC接收，使能IEEE流量控制
	enc28j60Write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);

    // MACON2清零，让MAC退出复位状态
	enc28j60Write(MACON2, 0x00);

  /* 用0填充所有短帧至60字节长 并追加一个CRC 发送CRC使能 帧长度校验使能 MAC全双工使能*/  
    /* 提示 由于ENC28J60不支持802.3的自动协商机制， 所以对端的网络卡需要强制设置为全双工 */  
    //enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);  
    // 下行程序段表示使能自动填充和自动CRC添加
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN); 	//半双工
//	enc28j60Write(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);

	// 配置非背对背包之间的间隔
	enc28j60Write(MAIPGL, 0x12);
    enc28j60Write(MAIPGH, 0x0C);
    // 配置背对背包之间的间隔
	enc28j60Write(MABBIPG, 0x12);

    // 设置允许接收或发送的最大帧长度编程
	enc28j60Write(MAMXFLL, MAX_FRAMELEN&0xFF);	
    enc28j60Write(MAMXFLH, MAX_FRAMELEN>>8);

    // Bank3区相关寄存器配置
	// 将MAC地址写入MAADR0-MAADR5寄存器中
	// NOTE: MAC address in ENC28J60 is byte-backward
	enc28j60Write(MAADR5, FU_MAC[0]);
	enc28j60Write(MAADR4, FU_MAC[1]);
	enc28j60Write(MAADR3, FU_MAC[2]);
	enc28j60Write(MAADR2, FU_MAC[3]);
	enc28j60Write(MAADR1, FU_MAC[4]);
	enc28j60Write(MAADR0, FU_MAC[5]);

//	/* 配置PHY为全双工  LEDB为拉电流 */  
//    enc28j60PhyWrite(PHCON1, PHCON1_PDPXMD);  
    
	/* LED状态 */  
	enc28j60PhyWrite(PHLCON,0x0476);    
    
    // 阻止发送回路的自动环回
	enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);

    // Bank0区相关寄存器配置 返回BANK0 
	enc28j60SetBank(ECON1);
    
  /* 使能中断 全局中断 接收中断 接收错误中断 */  
    //enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE|EIE_RXERIE);  	//INT有数据包中断使能
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);	  			//没有中断使能

	//WOL中断使能
	//enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EWOLIR, 0);//WOL各条件中断清0（ 单播，广播 ）
	//enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIR, 0);//WOL总中断清0
    //enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EWOLIE, 0x81);//WOL中断各条件使能
    //enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_WOLIE);//WOL总中断使能

	// 包接收允许使能
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

}

/*******************************************/
/* 功能：检测enc28j60是否需要复位，循环检查*/
/* 输入：秒数                              */
/* 输出：无							       */
/* 最后修改：2016-09-07 QQ:18407088(乘简)  */
/*******************************************/
void enc28j60IsRst(U8 Sec)
{
	static U8 tSec=60;
	U8 F1,F2;
//	U16 RxInit=0;
	if(tSec==Tim_Sec || tSec==60)
	{
		tSec=(Tim_Sec+Sec)%60;	//下次检测时间
		if(SCKN)SCKN=0;		//这句话必须要加，如果中途把SCKN线拨掉，再插，那么下面的状态还是01与04，根本就检测不出来
		if(CSN==0)CSN=1;	//可以把干扰，拨线等因素引起的端口错位的问题，复位，防止卡住了

//		//这种情况，是在使用过程中，把网络模块的两根电源线拨掉再重插，会出现下面的情况，需重新初始化
//		//当然，如果你的网络模块是画在主板上的，不存在单独断电的情况，可以注释之
//		RxInit=enc28j60Read(ERXNDL);
//		RxInit=(enc28j60Read(ERXNDH)<<8)|RxInit;
//		if(RxInit==0x1FFF)	//如果接收缓存为8191，说明根本就没有初始化，或者模块中途掉电了，则也需要重新初始化
//		{
//			Debug_Flag1++;
//			enc28j60_init();		//需要重新初始化
//			Debug_Flag16++;
//			return;
//		}

//		//在使用过程中，把4根数据线拨掉，再重新插，会出现下面的情况，需重新初始化
//		//当然，如果你的网络模块是画在主板上，不存在数据线单独断开情况，可以注释之
//		F1=enc28j60Read(ESTAT);
//		if(F1&ESTAT_INT)
//		{
//			Debug_Flag2++;
//			enc28j60_init();		//需要重新初始化
//			Debug_Flag16++;
//			return;
//		}

//		//这种情况，是我在插线板上接一负载，快速让他接触不良，就是不停的开断操作，一会网络模块就死掉了，
//		//用下面这个可以检测出来，重新初始化一下就正常了
//		F2=enc28j60Read(ECON1);
//		if((F2&0xC0)!=0)	//ECON1_TXRST or ECON1_RXRST
//		{
//			Debug_Flag3++;
//			enc28j60_init();		//需要重新初始化	
//			Debug_Flag16++;
//			return;
//		}
		//下面这种情况，是因为我单片机另一个端口拉低，然后引一根线出来，触碰网络模块的这4个端口，然后就死掉了
		//可以通过下面的代码检测出来，我想，有些地方干扰大就和下面这种方法一个样吧
//		if((F1&(ESTAT_TXABRT|ESTAT_CLKRDY))==(ESTAT_TXABRT|ESTAT_CLKRDY) || F2==0)
//		{
//			enc28j60_init();		//需要重新初始化	
//			Debug_Flag16++;
//		}

		//其实不管哪种情况导致enc28j60死机，只要不是正常状态，则都把模块重启一下就大功告成了
		F1=enc28j60Read(ESTAT);
		F2=enc28j60Read(ECON1);
		if(!(F1==0x01 && F2==0x04))
		{
			enc28j60_init();		//需要重新初始化
			net_init();
			#if DEBUG
				Uart1_SendStr("Reset\n");
			#endif
		}
	}
}

