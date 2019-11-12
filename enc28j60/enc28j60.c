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

idata U8 Enc28j60Bank;
idata U16 NextPacketPtr;
U8 Init_Num=0;			//��¼��λ����

/***************************** ENC28J60��س��� *****************************/

/*******************************************/
/* ���ܣ�ENC28J60������                    */
/* ���룺��                                */
/* �������							       */
/*******************************************/
U8 enc28j60ReadOp(U8 op, U8 address)	  //�����ƼĴ���
{
	U8 dat1;
	//Ƭѡ	
	CSN =0;
	// issue read command
	WriteByte(op | (address & ADDR_MASK));	
	dat1 = ReadByte();
	// do dummy read if needed (for mac and mii, see datasheet page 29)
	if(address & 0x80) 	dat1 = ReadByte();
	//�ͷ�Ƭѡ
	CSN=1;
	return(dat1);
}
/*******************************************/
/* ���ܣ�ENC28J60д����                    */
/* ���룺��                                */
/* �������							       */
/*******************************************/
void enc28j60WriteOp(U8 op, U8 address, U8 mydat)	 //д���ƼĴ���
{
	CSN=0;
	// issue write command
	WriteByte( op | (address & ADDR_MASK));	//д����
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
//	// Stop reading	����Ӧ������
//	//enc28j60Write(MICMD, MICMD_MIIRD);
//	enc28j60Write(MICMD, 0x0);   //��MICMD.MIIRD λ���㡣
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
/* ���ܣ�ENC28J60������������              */
/* ���룺��                                */
/* �������							       */
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
/* ���ܣ�ENC28J60д����������              */
/* ���룺��                                */
/* �������							       */
/*******************************************/

void enc28j60WriteBuffer(U16 len, U8* dat)
{
    // assert CS
	//ENC28J60_CONTROL_PORT &= ~(1<<ENC28J60_CONTROL_CS);
	CSN = 0;//ʹ��
    
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
/* ���ܣ�ENC28J60����ARP���ݰ�����         */
/* ���룺��								   */
/* �������							       */
/*******************************************/
void ARP_Packet_Send(void)
{
	// ����дָ�뿪ʼ�Ĵ��仺������
	enc28j60Write(EWRPTL, TXSTART_INIT);
    enc28j60Write(EWRPTH, TXSTART_INIT>>8);

    // ����TXNDָ���Ӧ�ڸ��������ݰ���С
	enc28j60Write(ETXNDL, (TXSTART_INIT+42));//14Ϊ��̫��ͷ+28 ARP���ݰ�����
    enc28j60Write(ETXNDH, (TXSTART_INIT+42)>>8);

    // дÿ�����Ŀ�����
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

    // TODO, fix this up

	
	    // ��̫��ͷ �����仺��
		enc28j60WriteBuffer(14, EN_Head);
		// ARP���ݰ� �����仺��
		enc28j60WriteBuffer(28, ARP_Data);
	    

	// ����̫�����ƼĴ���ECON1����λ ��1���Է��ͻ���������
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}



/*******************************************/
/* ���ܣ�ENC28J60����UDP���ݰ�����         */
/* ���룺UDP���ݳ���len	     			   */
/* �������							       */
/*******************************************/
void UDP_Packet_Send(U16 len)
{
	// ����дָ�뿪ʼ�Ĵ��仺������
	enc28j60Write(EWRPTL, TXSTART_INIT);
    enc28j60Write(EWRPTH, TXSTART_INIT>>8);

    // ����TXNDָ���Ӧ�ڸ��������ݰ���С
	enc28j60Write(ETXNDL, (TXSTART_INIT+42+len));//14Ϊ��̫��ͷ+20IPͷ+8UDPͷ+len���ݰ�����
    enc28j60Write(ETXNDH, (TXSTART_INIT+42+len)>>8);

    // дÿ�����Ŀ�����
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

    // TODO, fix this up

    
        // ��̫��ͷ �����仺��
		enc28j60WriteBuffer(14, EN_Head);
		// IPͷ �����仺��
		enc28j60WriteBuffer(20, IP_Head);
		// UDPͷ �����仺��
		enc28j60WriteBuffer(8, UDP_Head);
		// UDP���� �����仺��
		enc28j60WriteBuffer(len, DATA);
    

	// ����̫�����ƼĴ���ECON1����λ ��1���Է��ͻ���������
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

/*******************************************/
/* ���ܣ�ENC28J60����TCP���ݰ�����         */
/* ���룺TCP���ݳ���len      			   */
/* �������							       */
/*******************************************/
void TCP_Packet_Send(U16 len)
{
	// ����дָ�뿪ʼ�Ĵ��仺������
	enc28j60Write(EWRPTL, TXSTART_INIT);
    enc28j60Write(EWRPTH, TXSTART_INIT>>8);

    // ����TXNDָ���Ӧ�ڸ��������ݰ���С
	enc28j60Write(ETXNDL, (TXSTART_INIT+54+len));//14Ϊ��̫��ͷ+20IPͷ+20TCPͷ+len���ݰ�����
    enc28j60Write(ETXNDH, (TXSTART_INIT+54+len)>>8);

    // дÿ�����Ŀ�����
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

    // TODO, fix this up

    
        // ��̫��ͷ �����仺��
		enc28j60WriteBuffer(14, EN_Head);
		// IPͷ �����仺��
		enc28j60WriteBuffer(20, IP_Head);
		// UDPͷ �����仺��
		enc28j60WriteBuffer(20, TCP_Head);
		// UDP���� �����仺��

		enc28j60WriteBuffer(len,DATA);

    

	// ����̫�����ƼĴ���ECON1����λ ��1���Է��ͻ���������
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

/*******************************************/
/* ���ܣ�ENC28J60����ICMP_Ping���ݰ�����   */
/* ���룺��			   					   */
/* �������							       */
/*******************************************/
void ICMP_Ping_Packet_Send()//����ICMP_Ping��
{
	// ����дָ�뿪ʼ�Ĵ��仺������
	enc28j60Write(EWRPTL, TXSTART_INIT);
    enc28j60Write(EWRPTH, TXSTART_INIT>>8);

    // ����TXNDָ���Ӧ�ڸ��������ݰ���С
	enc28j60Write(ETXNDL, (TXSTART_INIT+74));//14Ϊ��̫��ͷ+20IPͷ+40ICMP_Ping���ݰ�����
    enc28j60Write(ETXNDH, (TXSTART_INIT+74)>>8);

    // дÿ�����Ŀ�����
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

    // TODO, fix this up

    
        // ��̫��ͷ �����仺��
		enc28j60WriteBuffer(14, EN_Head);
		// IPͷ �����仺��
		enc28j60WriteBuffer(20, IP_Head);
		// ICMP �����仺��
		enc28j60WriteBuffer(40, ICMP);


	// ����̫�����ƼĴ���ECON1����λ ��1���Է��ͻ���������
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}



//ETH����С����60��
#define ETHERNET_MIN_PACKET_LENGTH	0x3C
//ETHͷ����	14
#define ETHERNET_HEADER_LENGTH		0x0E
//IP+TCPͷ���� 20+ TCP=20,UDP=8
#define IP_TCP_HEADER_LENGTH 40
#define IP_UDP_HEADER_LENGTH 28
//IP+TCP+ETH ͷ����
#define TOTAL_TCP_HEADER_LENGTH (IP_TCP_HEADER_LENGTH+ETHERNET_HEADER_LENGTH)
#define TOTAL_UDP_HEADER_LENGTH (IP_UDP_HEADER_LENGTH+ETHERNET_HEADER_LENGTH)

///*******************************************/
///* ���ܣ�ENC28J60�������ݰ�����            */
///* ���룺leixing 0 TCP 1 UDP,len,���ݰ���ַ*/
///* �������							       */
///*******************************************/
//void enc28j60PacketSend(U8 leixing,U16 len,U8* packet)
//{
////    /* ��ѯ�����߼���λλ */  
////    while((enc28j60Read(ECON1) & ECON1_TXRTS)!= 0);  
//
//	// ����дָ�뿪ʼ�Ĵ��仺������
//	enc28j60Write(EWRPTL, TXSTART_INIT);
//    enc28j60Write(EWRPTH, TXSTART_INIT>>8);
//
//    // ����TXNDָ���Ӧ�ڸ��������ݰ���С
//	enc28j60Write(ETXNDL, (TXSTART_INIT+len));
//    enc28j60Write(ETXNDH, (TXSTART_INIT+len)>>8);
//
//    // дÿ�����Ŀ�����
//	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
//
//    // TODO, fix this up
//
//    if( FU_LEN <= TOTAL_UDP_HEADER_LENGTH-42 )//�������С���ⳤ��,��������
//    {
//        // ���ư������仺��
//		enc28j60WriteBuffer(len, packet);
//    }
//    else
//    {
//		// д��ͷ
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
//		// д������
//        enc28j60WriteBuffer(len, (U8 *)DATA);
//    }
//
//	// ����̫�����ƼĴ���ECON1����λ ��1���Է��ͻ���������
//	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
//}

/*******************************************/
/* ���ܣ�ENC28J60�������ݰ�����            */
/* ���룺���maxlen,�洢���ݰ���ַ         */
/* ��������ݳ���						   */
/* ����޸ģ�2016.06.05 QQ:18407088(�˼�)  */
/*******************************************/
U16 enc28j60PacketReceive(U16 maxlen, U8* packet)	 //�������ݰ���������󳤶� ����
{
    static xdata U16 NextPackLen=0,ReadedPackLen=0;		//�´��Ƿ��������
	static idata U16 CurrentPacketPtr;	  	//���浱ǰ����λ��
	U16 rxstat;
    U16 len;
	U8 i=0;

	if(NextPackLen==0)		//������յİ��ڻ��淶Χ��
	{
		// ��黺���Ƿ�һ�����Ѿ��յ�
		if( !(enc28j60Read(EIR) & EIR_PKTIF) )
	    {
	        // ͨ���鿴EPKTCNT�Ĵ����ٴμ���Ƿ��յ���
			if (enc28j60Read(EPKTCNT) == 0)
	            return 0;
	    }
	    
		//���ý��յ������ݰ���ָ�뿪ʼ
		enc28j60Write(ERDPTL, (NextPacketPtr));
	    enc28j60Write(ERDPTH, (NextPacketPtr)>>8);
		CurrentPacketPtr=NextPacketPtr;

	    // ��һ�������ָ��
		NextPacketPtr  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	    NextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	
	    // ��ȡ���ĳ���
		len  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	    len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
		len-=4;// �Ƴ�CRC�ֶεĳ���������MAC�����泤��
	
		// ��ȡ�������ݰ���״̬
		rxstat  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
		rxstat |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	    //if ((rxstat & 0x80)==0)	return 0; 	    // invalid
	
	    // ����ʵ�����ݳ���
		if(len>maxlen)//����յ��İ��ĳ��ȣ����ڽ��ջ���ĳ���
		{
			NextPackLen=len-maxlen;	 		//ʣ���ֽ�
			len=maxlen;
			// copy the packet from the receive buffer
		    enc28j60ReadBuffer(len, packet);
			packet[16]=(len-14)>>8;//�޸�IP���ѽ��հ�����,����ֻ�н���UDP��ʱ�Ż���ֳ�������
			packet[17]=len-14;//,���õ���TCP,��ΪTCP�д���С����,���ᳬ������
		}
		else
		{
			NextPackLen=0;
			// copy the packet from the receive buffer
		    enc28j60ReadBuffer(len, packet);
		}

		ReadedPackLen=len;		//�Ѷ�����

	}
	else 		//������ϴ�û������İ�
	{
		if(NextPackLen>maxlen-42)		//����UDP��ͷ42�ֽڣ�TCP�޴�����
		{
			NextPackLen=NextPackLen-(maxlen-42);
			len=maxlen-42;
		}
		else
		{
			len=NextPackLen;
			NextPackLen=0;
		}

		//���ý��յ������ݰ���ָ�뿪ʼ
		enc28j60Write(ERDPTL, (CurrentPacketPtr+6));	   	//����ǰ��6�ֽڵĳ�����Ϣ
	    enc28j60Write(ERDPTH, (CurrentPacketPtr+6)>>8);
		enc28j60ReadBuffer(42, packet);		//������ͷ��Ŀǰ֧��UDP��ͷΪ42�ֽڣ�TCP��������������
		packet[16]=(len+28)>>8; 		//�޸�IP���ѽ��հ�����,����ֻ�н���UDP��ʱ�Ż���ֳ�������
		packet[17]=len+28;				//,���õ���TCP,��ΪTCP�д���С����,���ᳬ������

		enc28j60Write(ERDPTL, (CurrentPacketPtr+6+ReadedPackLen));	   	//�����ϴζ����λ�ô���ʼ��
	    enc28j60Write(ERDPTH, (CurrentPacketPtr+6+ReadedPackLen)>>8); 	
		enc28j60ReadBuffer(len, packet+42);		//����ʣ�µ�����
		ReadedPackLen+=len;

		len+=42;
	}
	
	if(NextPackLen==0)		 		//���������ռģ��Ļ���ռ�
	{
	    // ERXRDPT��������ָ��
		// ENC28J60��һֱд����ָ��֮ǰ��һ��ԪΪֹ
	    enc28j60Write(ERXRDPTL, (NextPacketPtr));
	    enc28j60Write(ERXRDPTH, (NextPacketPtr)>>8);
	    // Errata workaround #13. Make sure ERXRDPT is odd

	    // ���ݰ������ݼ�λEPKTCNT��1
		enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);  
	}

    return len;
}

/*******************************************/
/* ���ܣ�ENC28J60оƬ��ʼ������            */
/* ���룺��                                */
/* �������							       */
/* ����޸ģ�2016-06-14 QQ:18407088(�˼�)  */
/*******************************************/
void enc28j60_init(void)   	//�˺�������Ҫ����Timer_Init()���棬Ҫ��Ȼ��ʼ�����ɹ����Ῠס
{
	unsigned char t;
	//*****Bank1����ؼĴ�������            SPI������         ���ݿ�
	//��ʼ������һ��ʼ�Ƚ��������λ��111<������>+11111<����>�� N/A
	// ENC28J60_SOFT_RESET=0xFF
	enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
//	Delay10ms();
	/*����Ҫע�⣬�����Ƭ������5V��ѹ�ܶ࣬�ͻḴλ���ɹ���Ȼ�������ѭ����
	��Ϊģ��ENC28j60��3.3V�ģ��������ģ��Ҳ����֧��5V��Ƭ��������5.17V����ʱ���λ���ɹ�
	��������5V��һ�����ⶼû�У��������������ź�Ҳû���⣬
	���������Ǵ��ڶ���V��λ���ɹ������д�����*/
	t=Tim_Sec;
	while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY) && t==Tim_Sec);	//�ȴ���λ�ɹ�

    // do bank 0 stuff
	// initialize receive buffer
	// 16-bit transfers, must write low byte first

    // ��ʼ�����ջ����������ý�����ʼ��ַ
	NextPacketPtr = RXSTART_INIT;	 // ����һ���ݰ�ָ��
    enc28j60Write(ERXSTL, RXSTART_INIT&0xFF);
    enc28j60Write(ERXSTH, RXSTART_INIT>>8);

    // ���ý��ն�ָ��ָ���ַ
	enc28j60Write(ERXRDPTL, RXSTART_INIT&0xFF);
    enc28j60Write(ERXRDPTH, RXSTART_INIT>>8);

 	// ���ý��ջ�������ĩβ��ַ
	// ERXND�Ĵ���Ĭ��ָ�����������������һ����Ԫ
	enc28j60Write(ERXNDL, RXSTOP_INIT&0xFF);
    enc28j60Write(ERXNDH, RXSTOP_INIT>>8);

	// ���÷��ͻ���������ʼ��ַ
	// ETXST�Ĵ���Ĭ�ϵ�ַ�������������ĵ�һ����Ԫ
	enc28j60Write(ETXSTL, TXSTART_INIT&0xFF);
    enc28j60Write(ETXSTH, TXSTART_INIT>>8);

	//�����ٳ�ʼ�����ͻ�������λ��,��Ϊenc28j60��Ĭ�ϵ����Ĳ���ȫ�ָ����ͻ���
//    enc28j60Write(ETXNDL, TXSTOP_INIT&0xFF);  
//    enc28j60Write(ETXNDH, TXSTOP_INIT>>8);  

	/* ʹ�ܵ������� ʹ��CRCУ�� ʹ��*/
	//UCEN:��������,ANDOR:������ѡ��,CRCEN:CRCУ��ʹ��
	//PMEN:��ʽ���������,MPEN:MAC��ַ������,
	//HTEN:��ϣ�������,MCEN:�鲥������,BCEN:�㲥������
    //enc28j60Write(ERXFCON, ERXFCON_UCEN);//��������,���հ�Ŀ���ַ�뱾����ַ��������ʧ,�����Ҫ������������ץ������,����Ҫ�޸Ĵ˴�
    enc28j60Write(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_BCEN);//��������+�㲥
	/*************************************************/  

  /* ʹ��MAC���� ����MAC������ͣ����֡ �����յ���ͣ����֡ʱֹͣ����*/  
  /* �����ֲ�34ҳ */  
 	// Bank2����ؼĴ�������
 	// MAC��ʼ������
	// MAC����ʹ�ܣ����г���α�ʾʹ��MAC���գ�ʹ��IEEE��������
	enc28j60Write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);

    // MACON2���㣬��MAC�˳���λ״̬
	enc28j60Write(MACON2, 0x00);

  /* ��0������ж�֡��60�ֽڳ� ��׷��һ��CRC ����CRCʹ�� ֡����У��ʹ�� MACȫ˫��ʹ��*/  
    /* ��ʾ ����ENC28J60��֧��802.3���Զ�Э�̻��ƣ� ���ԶԶ˵����翨��Ҫǿ������Ϊȫ˫�� */  
    //enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);  
    // ���г���α�ʾʹ���Զ������Զ�CRC���
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN); 	//��˫��
//	enc28j60Write(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);

	// ���÷Ǳ��Ա���֮��ļ��
	enc28j60Write(MAIPGL, 0x12);
    enc28j60Write(MAIPGH, 0x0C);
    // ���ñ��Ա���֮��ļ��
	enc28j60Write(MABBIPG, 0x12);

    // ����������ջ��͵����֡���ȱ��
	enc28j60Write(MAMXFLL, MAX_FRAMELEN&0xFF);	
    enc28j60Write(MAMXFLH, MAX_FRAMELEN>>8);

    // Bank3����ؼĴ�������
	// ��MAC��ַд��MAADR0-MAADR5�Ĵ�����
	// NOTE: MAC address in ENC28J60 is byte-backward
	enc28j60Write(MAADR5, FU_MAC[0]);
	enc28j60Write(MAADR4, FU_MAC[1]);
	enc28j60Write(MAADR3, FU_MAC[2]);
	enc28j60Write(MAADR2, FU_MAC[3]);
	enc28j60Write(MAADR1, FU_MAC[4]);
	enc28j60Write(MAADR0, FU_MAC[5]);

//	/* ����PHYΪȫ˫��  LEDBΪ������ */  
//    enc28j60PhyWrite(PHCON1, PHCON1_PDPXMD);  
    
	/* LED״̬ */  
	enc28j60PhyWrite(PHLCON,0x0476);    
    
    // ��ֹ���ͻ�·���Զ�����
	enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);

    // Bank0����ؼĴ������� ����BANK0 
	enc28j60SetBank(ECON1);
    
  /* ʹ���ж� ȫ���ж� �����ж� ���մ����ж� */  
    //enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE|EIE_RXERIE);  	//INT�����ݰ��ж�ʹ��
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);	  			//û���ж�ʹ��

	//WOL�ж�ʹ��
	//enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EWOLIR, 0);//WOL�������ж���0�� �������㲥 ��
	//enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIR, 0);//WOL���ж���0
    //enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EWOLIE, 0x81);//WOL�жϸ�����ʹ��
    //enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_WOLIE);//WOL���ж�ʹ��

	// ����������ʹ��
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

}

/*******************************************/
/* ���ܣ����enc28j60�Ƿ���Ҫ��λ��ѭ�����*/
/* ���룺����                              */
/* �������							       */
/* ����޸ģ�2016-09-07 QQ:18407088(�˼�)  */
/*******************************************/
void enc28j60IsRst(U8 Sec)
{
	static U8 tSec=60;
	U8 F1,F2;
//	U16 RxInit=0;
	if(tSec==Tim_Sec || tSec==60)
	{
		tSec=(Tim_Sec+Sec)%60;	//�´μ��ʱ��
		if(SCKN)SCKN=0;		//��仰����Ҫ�ӣ������;��SCKN�߲������ٲ壬��ô�����״̬����01��04�������ͼ�ⲻ����
		if(CSN==0)CSN=1;	//���԰Ѹ��ţ����ߵ���������Ķ˿ڴ�λ�����⣬��λ����ֹ��ס��

//		//�������������ʹ�ù����У�������ģ���������Դ�߲������ز壬��������������������³�ʼ��
//		//��Ȼ������������ģ���ǻ��������ϵģ������ڵ����ϵ�����������ע��֮
//		RxInit=enc28j60Read(ERXNDL);
//		RxInit=(enc28j60Read(ERXNDH)<<8)|RxInit;
//		if(RxInit==0x1FFF)	//������ջ���Ϊ8191��˵��������û�г�ʼ��������ģ����;�����ˣ���Ҳ��Ҫ���³�ʼ��
//		{
//			Debug_Flag1++;
//			enc28j60_init();		//��Ҫ���³�ʼ��
//			Debug_Flag16++;
//			return;
//		}

//		//��ʹ�ù����У���4�������߲����������²壬��������������������³�ʼ��
//		//��Ȼ������������ģ���ǻ��������ϣ������������ߵ����Ͽ����������ע��֮
//		F1=enc28j60Read(ESTAT);
//		if(F1&ESTAT_INT)
//		{
//			Debug_Flag2++;
//			enc28j60_init();		//��Ҫ���³�ʼ��
//			Debug_Flag16++;
//			return;
//		}

//		//��������������ڲ��߰��Ͻ�һ���أ����������Ӵ����������ǲ�ͣ�Ŀ��ϲ�����һ������ģ��������ˣ�
//		//������������Լ����������³�ʼ��һ�¾�������
//		F2=enc28j60Read(ECON1);
//		if((F2&0xC0)!=0)	//ECON1_TXRST or ECON1_RXRST
//		{
//			Debug_Flag3++;
//			enc28j60_init();		//��Ҫ���³�ʼ��	
//			Debug_Flag16++;
//			return;
//		}
		//�����������������Ϊ�ҵ�Ƭ����һ���˿����ͣ�Ȼ����һ���߳�������������ģ�����4���˿ڣ�Ȼ���������
		//����ͨ������Ĵ�������������룬��Щ�ط����Ŵ�ͺ��������ַ���һ������
//		if((F1&(ESTAT_TXABRT|ESTAT_CLKRDY))==(ESTAT_TXABRT|ESTAT_CLKRDY) || F2==0)
//		{
//			enc28j60_init();		//��Ҫ���³�ʼ��	
//			Debug_Flag16++;
//		}

		//��ʵ���������������enc28j60������ֻҪ��������״̬���򶼰�ģ������һ�¾ʹ󹦸����
		F1=enc28j60Read(ESTAT);
		F2=enc28j60Read(ECON1);
		if(!(F1==0x01 && F2==0x04))
		{
			enc28j60_init();		//��Ҫ���³�ʼ��
			net_init();
			#if DEBUG
				Uart1_SendStr("Reset\n");
			#endif
		}
	}
}

