
// File Name: 433_Wiminet.c

#include "stdlib.h"
#include "string.h"
#include "API-Platform.h"
#include "433_Wiminet.h"
#include "bsp_SysTick.h"
#include "gprs.h"
#include "bsp_usart.h"
#include "common.h"





extern  char      Usart3_recev_buff[1000];
extern  uint16_t  Usart3_recev_count;
extern  uint8_t   DMA_UART3_RECEV_FLAG ;       //USART3 DMA接收标志变量
//extern  uint32_t  time;                     // ms 计时变量 

extern u8  		  SENSOR_ID[6];
extern u16			NODE_ADDR;


extern unsigned char DMA_UART3_RecevDetect(unsigned char RecevFlag,u8* pDeviceID, u16 sNodeAddress);     //USART3接收数据监测与数据解析
//extern void  RecvBuffInit_USART3(void);
//extern  void  ReceiveAnalysis_433(void);
//extern  char  Receive_Monitor_GPRS(void);
//extern  void Receive_Deal_GPRS(void);
//extern u8  NetStatus_Detection( void );
// *****************************************************************************
// Design Notes:  
// -----------------------------------------------------------------------------
unsigned char ReverseBitOrder08( unsigned char iSrc )
{
   unsigned char index;
   unsigned char iDst;
   
   iDst = iSrc & 0X01;
   for( index = 0X00; index < 0X07; index++ )
   {
      iDst <<= 0X01;
      iSrc >>= 0X01;
      iDst |= ( iSrc & 0X01 );
   }
   return iDst;
}

// *****************************************************************************
// Design Notes:  
// -----------------------------------------------------------------------------
unsigned short ReverseBitOrder16( unsigned short iSrc )
{
   unsigned char index;
   unsigned short iDst;
   
   iDst = iSrc & 0X01;
   for( index = 0X00; index < 0X0F; index++ )
   {
      iDst <<= 0X01;
      iSrc >>= 0X01;
      iDst |= ( iSrc & 0X01 );
   }
   return iDst;
}

// *****************************************************************************
// Design Notes: CRC-16
// f(X)=X^16 + X^15 + X^2 + X^0
// POLYNOMIALS = 0X8005
// -----------------------------------------------------------------------------
unsigned short CRC16( unsigned char * pMsg, unsigned short iSize )
{
   unsigned char  index;
   unsigned short iCRC;
   
   // The default value
   iCRC = 0XFFFF;
   while ( iSize-- )
   {
      iCRC ^= ( ( ( unsigned short ) ReverseBitOrder08( *pMsg ) ) << 0X08 );
      for ( index = 0X00; index < 0X08; index++ )
      {
         if ( iCRC & 0X8000 )
         {
            iCRC = ( iCRC << 1 ) ^ 0X8005;
         }
         else
         {
            iCRC <<= 1;
         }
      }
      pMsg++;
   }
   return ReverseBitOrder16( iCRC );
}

//// -----------------------------------------------------------------------------
//// DESCRIPTION: CRC-16校验的高位字节表
//// -----------------------------------------------------------------------------
//static const unsigned char HiCRCTable[] = { 
//0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 
//0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 
//0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 
//0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 
//0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 
//0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 
//0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 
//0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 
//0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 
//0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 
//0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 
//0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 
//0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 
//0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 
//0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40, 0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 
//0X00, 0XC1, 0X81, 0X40, 0X01, 0XC0, 0X80, 0X41, 0X01, 0XC0, 0X80, 0X41, 0X00, 0XC1, 0X81, 0X40 };

//// -----------------------------------------------------------------------------
//// DESCRIPTION: CRC-16校验的低位字节表
//// -----------------------------------------------------------------------------
//static const unsigned char LoCRCTable[] = { 
//0X00, 0XC0, 0XC1, 0X01, 0XC3, 0X03, 0X02, 0XC2, 0XC6, 0X06, 0X07, 0XC7, 0X05, 0XC5, 0XC4, 0X04, 
//0XCC, 0X0C, 0X0D, 0XCD, 0X0F, 0XCF, 0XCE, 0X0E, 0X0A, 0XCA, 0XCB, 0X0B, 0XC9, 0X09, 0X08, 0XC8, 
//0XD8, 0X18, 0X19, 0XD9, 0X1B, 0XDB, 0XDA, 0X1A, 0X1E, 0XDE, 0XDF, 0X1F, 0XDD, 0X1D, 0X1C, 0XDC, 
//0X14, 0XD4, 0XD5, 0X15, 0XD7, 0X17, 0X16, 0XD6, 0XD2, 0X12, 0X13, 0XD3, 0X11, 0XD1, 0XD0, 0X10, 
//0XF0, 0X30, 0X31, 0XF1, 0X33, 0XF3, 0XF2, 0X32, 0X36, 0XF6, 0XF7, 0X37, 0XF5, 0X35, 0X34, 0XF4, 
//0X3C, 0XFC, 0XFD, 0X3D, 0XFF, 0X3F, 0X3E, 0XFE, 0XFA, 0X3A, 0X3B, 0XFB, 0X39, 0XF9, 0XF8, 0X38, 
//0X28, 0XE8, 0XE9, 0X29, 0XEB, 0X2B, 0X2A, 0XEA, 0XEE, 0X2E, 0X2F, 0XEF, 0X2D, 0XED, 0XEC, 0X2C, 
//0XE4, 0X24, 0X25, 0XE5, 0X27, 0XE7, 0XE6, 0X26, 0X22, 0XE2, 0XE3, 0X23, 0XE1, 0X21, 0X20, 0XE0, 
//0XA0, 0X60, 0X61, 0XA1, 0X63, 0XA3, 0XA2, 0X62, 0X66, 0XA6, 0XA7, 0X67, 0XA5, 0X65, 0X64, 0XA4, 
//0X6C, 0XAC, 0XAD, 0X6D, 0XAF, 0X6F, 0X6E, 0XAE, 0XAA, 0X6A, 0X6B, 0XAB, 0X69, 0XA9, 0XA8, 0X68, 
//0X78, 0XB8, 0XB9, 0X79, 0XBB, 0X7B, 0X7A, 0XBA, 0XBE, 0X7E, 0X7F, 0XBF, 0X7D, 0XBD, 0XBC, 0X7C, 
//0XB4, 0X74, 0X75, 0XB5, 0X77, 0XB7, 0XB6, 0X76, 0X72, 0XB2, 0XB3, 0X73, 0XB1, 0X71, 0X70, 0XB0, 
//0X50, 0X90, 0X91, 0X51, 0X93, 0X53, 0X52, 0X92, 0X96, 0X56, 0X57, 0X97, 0X55, 0X95, 0X94, 0X54, 
//0X9C, 0X5C, 0X5D, 0X9D, 0X5F, 0X9F, 0X9E, 0X5E, 0X5A, 0X9A, 0X9B, 0X5B, 0X99, 0X59, 0X58, 0X98, 
//0X88, 0X48, 0X49, 0X89, 0X4B, 0X8B, 0X8A, 0X4A, 0X4E, 0X8E, 0X8F, 0X4F, 0X8D, 0X4D, 0X4C, 0X8C, 
//0X44, 0X84, 0X85, 0X45, 0X87, 0X47, 0X46, 0X86, 0X82, 0X42, 0X43, 0X83, 0X41, 0X81, 0X80, 0X40 };


//// *****************************************************************************
//// Design Notes:  
//// -----------------------------------------------------------------------------
//unsigned short QuickCRC16( unsigned char * pMsg, unsigned short iSize )
//{
//   unsigned char iHiVal;                // high byte of CRC initialized
//   unsigned char iLoVal;                // low byte of CRC initialized
//   unsigned char index;                 // will index into CRC lookup table
//   
//   // Initial value for the CRC
//   iHiVal = 0XFF;
//   iLoVal = 0XFF;
//   
//   while ( iSize-- )
//   {
//      // Calculate the CRC
//      index = iLoVal ^ ( unsigned char )( *pMsg++ );
//      
//      iLoVal = iHiVal ^ HiCRCTable[index];
//      iHiVal = LoCRCTable[index];
//   }
//   return ( iHiVal << 8 | iLoVal );
//}
// *****************************************************************************
// Design Notes:  
// -----------------------------------------------------------------------------
void UpdateNodeMsgCRC( NodeMsg * pMsg )
{
   unsigned char  iSize;
   unsigned short iCRC;
   
   // The message header
//   pMsg->m_iHeader = 0xAA;

   // The defualt CRC value
   pMsg->m_iCRCode = 0x00;

   // The message size
   iSize = 0x09;
   iSize += pMsg->m_iAmount;

   // Update the CRC value
   iCRC = CRC16( ( unsigned char * )pMsg, iSize );

// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#if ( CPU_ENDIAN_MODE == LITTLE_ENDIAN_MODE )

   // Change the byte order
   iCRC = ntohs( iCRC );

// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#endif

   // Restore the CRC of this message
   pMsg->m_iCRCode = iCRC;
}

//unsigned short ReverseByteOrder (unsigned short OriginalData)
//{
//   char* pData = NULL;
//	 pData = (char*)&OriginalData;
//	 return  (*pData<<8) + *(pData+1);
//}

// *****************************************************************************
// Design Notes: 初始化结构体
// -----------------------------------------------------------------------------
void InitCommandMessage( NodeMsg * pMsg )
{
//   // Initialize the message
//   InitWiMinetMessage( pMsg, 0X01 );   
	 // Reset the message body
   memset( pMsg, 0x00, sizeof( NodeMsg ) );
}

//// *****************************************************************************
//// Design Notes:  对查询主站ID函数封包

//void IMP_GetCoordinatorID( NodeMsg * pMsg )
//{
//   // The command for this packet
//   pMsg->m_iHeader = 0xAA;
//	 // The command for this packet
////   pMsg->m_iOpCode = CMDMSG_GET_COORDINATOR_ID;
//   pMsg->m_iOpCode = 0x76;  
//}
//// *****************************************************************************
//// Design Notes: 查询主站ID
//// -----------------------------------------------------------------------------
// unsigned short GetCoordinatorID (void)
//{
//   NodeMsg  Msg;      //正式使用时不用
//	 NodeMsg* pMsg = &Msg;
//   unsigned short iSize;
////	 char  i=0;                   //调试使用
//	 char  receive_flag =0;
//	 //char  SendBuffer[12]={0xAA,0x1D,0x33,0x86,0x03,0x00,0x03,0x00,0x00,0x01,0x02,0x03};
//   char  ID_GetFlag[2]={0xAA,0xF6};
//   char*  pRecevBuff=NULL;
//   unsigned short  ID_Temp=0x0000;

//   // Initialize the message body
//   InitCommandMessage( pMsg );

//   // Construct the message
//   IMP_GetCoordinatorID( pMsg );
//   UpdateNodeMsgCRC( pMsg );
//	 // The total message size
//   iSize = pMsg->m_iAmount + 0x09;

//	 USART_DataBlock_Send(USART1,(char* )(pMsg),iSize);    //调试使用
//   USART_DataBlock_Send(USART1,"\r\n",2);                //调试使用
//	 USART_DataBlock_Send(USART3,(char* )(pMsg),iSize);
//	 
//	 Delay_ms(1000);
//   receive_flag = Receive_Monitor_433();
//	 if(receive_flag == 1)
//	 {
//		 pRecevBuff = Find_SpecialString(Usart3_recev_buff,ID_GetFlag,300,2);  //检查有无收到主站回复
//		 if(pRecevBuff!=NULL)                                     //有回复，提取地址信息   //暂时缺少CRC校验
//		 {   
//			
////			 printf("\r\nResponse from 433 is:");                   //调试使用
////			 for(i=0;i<Usart3_recev_count;i++)                      //调试使用
////			 {
////					printf(" %x",pRecevBuff[i]);                        //调试使用
////			 }
//			 ID_Temp = (pRecevBuff[9]*256) +pRecevBuff[10];
//			 pRecevBuff =NULL;
//			 receive_flag = 0;
//			 memset(Usart3_recev_buff,'\0',300);	
//			 Usart3_recev_count =0;                                 //清空USART3接收计数器
//			 time=0;	                                               //定时器复位
//			 return  ID_Temp;
//		 }
//		 else
//		 {	 
//				memset(Usart3_recev_buff,'\0',300);	
//				Usart3_recev_count =0;                                 //清空USART3接收计数器
//				time=0;	                                               //定时器复位
//		 }
//	 }
//	 return  0x0000;
//}
// *****************************************************************************
// Design Notes:  发送数据封包
// -----------------------------------------------------------------------------
void IMP_SendMessage( NodeMsg * pMsg )
{ 
	 // The command for this packet
   pMsg->m_iHeader = 0xAA;
	 // The command for this packet
   pMsg->m_iOpCode = 0x1E;  
	 pMsg->m_iValueC = 0x03;
}
// *****************************************************************************
// Design Notes:  发送数据主函数
// -----------------------------------------------------------------------------
uint8_t  SendMessage(char* Psend, unsigned short iSize)
{

   int  i=0;   //测试使用
   if(iSize>255)
	 {
     iSize =255;  //限定长度，防止缓存溢出
   }
 
	 for(i=0;i<iSize;i++)
	 printf("--%x--",Psend[i]);                   //发送数据打印
   mput_mix(( char * )(Psend), iSize);

	 Delay_ms(3000);                                  //为了方便发送完配置成功帧以后接收服务器会话结束命令，在此处增加延时
#if DEBUG_TEST
	 printf("\r\nSend Over!!\r\n");                   //调试使用
#endif
	 return 1;
}

// *****************************************************************************
// Design Notes:  
// -----------------------------------------------------------------------------
unsigned char IsValidNodeMsg( NodeMsg * pMsg )
{
   unsigned short iCRC1;
   unsigned short iCRC2;
   unsigned short iSize;

   // Check the header
   if ( pMsg->m_iHeader != 0XAA )
   {
      return 0X00;
   }

   // The original CRC
   iCRC1 = pMsg->m_iCRCode;

   // Clear the CRC
   pMsg->m_iCRCode = 0X00;

   // The total message size
   iSize = 0X09;
   iSize += pMsg->m_iAmount;

   // Validate the CRC of this message
   iCRC2 = CRC16( ( unsigned char * )pMsg, iSize );

// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#if ( CPU_ENDIAN_MODE == LITTLE_ENDIAN_MODE )

   // Change the byte order
   iCRC2 = ntohs( iCRC2 );
//	 printf("CRC is :%4x\r\n", iCRC2);    测试使用

// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#endif

   // Restore the CRC of this message
   pMsg->m_iCRCode = iCRC1;

   // Check the CRC value
   return ( iCRC1 == iCRC2 );
}

// *****************************************************************************
// Design Notes: 接收数据解析函数
// -----------------------------------------------------------------------------
unsigned char  ReceiveMessageVerify(char*  pRecevBuff)
{
	 NodeMsg  Msg;
	 NodeMsg* pMsg = &Msg;
   uint16_t i=0;
   unsigned char MessageRecv_Flag=0;
//   char  Recev_OK[2]={0xAA,0x1E};
//   char*  pRecevBuff=NULL;


//	  #if DEBUG_TEST	  
//    printf("\r\nReceive from 433:");
//		for(i=0;i<150;i++)
//		{
//       printf(" %x",pRecevBuff[i]);
//    }
//		#endif
	 pMsg->m_iHeader = pRecevBuff[0];
	 pMsg->m_iOpCode = pRecevBuff[1];
	 pMsg->m_iValueA = pRecevBuff[2];
	 pMsg->m_iValueB = pRecevBuff[3];
	 pMsg->m_iValueC = pRecevBuff[4];
	 pMsg->m_iValueD = pRecevBuff[5];
	 pMsg->m_iAmount = pRecevBuff[6];
	 pMsg->m_iCRCode = (pRecevBuff[8]*256)+pRecevBuff[7];    //顺序有待进一步确认
	 if(pRecevBuff< (Usart3_recev_buff+sizeof(Usart3_recev_buff)-1-9-pMsg->m_iAmount)) //防止指针越界 
	 {
     for(i=0;i<(pMsg->m_iAmount);i++)
	   {
        pMsg->m_pBuffer[i] = pRecevBuff[9+i];
     } 
	   MessageRecv_Flag = IsValidNodeMsg(pMsg);
	   if(MessageRecv_Flag ==1)                           //接收数据有效        
	   {
			 #if DEBUG_TEST	
		   printf("\r\nData receive from %4x is :",(pMsg->m_iValueA*256)+(pMsg->m_iValueB));
			 for(i=0;i<(pMsg->m_iAmount);i++)
			 {    
			   printf(" %x ",pMsg->m_pBuffer[i]);
			 } 
			 #endif
			 return 1;
	   }
   }
	 else
	 {
      printf("\r\nWarnning!Memory OverFlow Occur!!\r\n");
   }
	 return 0;
}




//// *****************************************************************************
//// Design Notes:  
//// -----------------------------------------------------------------------------
//void WriteNodeMsg( unsigned char hFile, NodeMsg * pMsg )
//{
//   unsigned char iSize;

//   // Update the message CRC value
//   UpdateNodeMsgCRC( pMsg );

//   // The packet whole size
//   iSize = 0X09 + pMsg->m_iAmount;

//   // Write out message header
////   WriteFile( hFile, ( char * )pMsg, iSize );   //暂时注释掉
//}











