
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
extern  uint8_t   DMA_UART3_RECEV_FLAG ;       //USART3 DMA���ձ�־����
//extern  uint32_t  time;                     // ms ��ʱ���� 

extern u8  		  SENSOR_ID[6];
extern u16			NODE_ADDR;


extern unsigned char DMA_UART3_RecevDetect(unsigned char RecevFlag,u8* pDeviceID, u16 sNodeAddress);     //USART3�������ݼ�������ݽ���
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



// *****************************************************************************
// Design Notes: ��ʼ���ṹ��
// -----------------------------------------------------------------------------
void InitCommandMessage( NodeMsg * pMsg )
{
   memset( pMsg, 0x00, sizeof( NodeMsg ) );
}

//// *****************************************************************************
//// Design Notes:  �Բ�ѯ��վID�������


//// *****************************************************************************
//// Design Notes: ��ѯ��վID
//// -----------------------------------------------------------------------------
// unsigned short GetCoordinatorID (void)
//{
//   NodeMsg  Msg;      //��ʽʹ��ʱ����
//	 NodeMsg* pMsg = &Msg;
//   unsigned short iSize;
////	 char  i=0;                   //����ʹ��
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

//	 USART_DataBlock_Send(USART1,(char* )(pMsg),iSize);    //����ʹ��
//   USART_DataBlock_Send(USART1,"\r\n",2);                //����ʹ��
//	 USART_DataBlock_Send(USART3,(char* )(pMsg),iSize);
//	 
//	 Delay_ms(1000);
//   receive_flag = Receive_Monitor_433();
//	 if(receive_flag == 1)
//	 {
//		 pRecevBuff = Find_SpecialString(Usart3_recev_buff,ID_GetFlag,300,2);  //��������յ���վ�ظ�
//		 if(pRecevBuff!=NULL)                                     //�лظ�����ȡ��ַ��Ϣ   //��ʱȱ��CRCУ��
//		 {   
//			
////			 printf("\r\nResponse from 433 is:");                   //����ʹ��
////			 for(i=0;i<Usart3_recev_count;i++)                      //����ʹ��
////			 {
////					printf(" %x",pRecevBuff[i]);                        //����ʹ��
////			 }
//			 ID_Temp = (pRecevBuff[9]*256) +pRecevBuff[10];
//			 pRecevBuff =NULL;
//			 receive_flag = 0;
//			 memset(Usart3_recev_buff,'\0',300);	
//			 Usart3_recev_count =0;                                 //���USART3���ռ�����
//			 time=0;	                                               //��ʱ����λ
//			 return  ID_Temp;
//		 }
//		 else
//		 {	 
//				memset(Usart3_recev_buff,'\0',300);	
//				Usart3_recev_count =0;                                 //���USART3���ռ�����
//				time=0;	                                               //��ʱ����λ
//		 }
//	 }
//	 return  0x0000;
//}

// *****************************************************************************
// Design Notes:  ��������������
// -----------------------------------------------------------------------------
uint8_t  SendMessage(char* Psend, unsigned short iSize)
{

   int  i=0;   //����ʹ��
   if(iSize>255)
	 {
     iSize =255;  //�޶����ȣ���ֹ�������
   }
 
	 for(i=0;i<iSize;i++)
	 printf("--%x--",Psend[i]);                   //�������ݴ�ӡ
   mput_mix(( char * )(Psend), iSize);

	 Delay_ms(3000);                                  //Ϊ�˷��㷢�������óɹ�֡�Ժ���շ������Ự��������ڴ˴�������ʱ
#if DEBUG_TEST
	 printf("\r\nSend Over!!\r\n");                   //����ʹ��
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
//	 printf("CRC is :%4x\r\n", iCRC2);    ����ʹ��

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
// Design Notes: �������ݽ�������
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
	 pMsg->m_iCRCode = (pRecevBuff[8]*256)+pRecevBuff[7];    //˳���д���һ��ȷ��
	 if(pRecevBuff< (Usart3_recev_buff+sizeof(Usart3_recev_buff)-1-9-pMsg->m_iAmount)) //��ָֹ��Խ�� 
	 {
     for(i=0;i<(pMsg->m_iAmount);i++)
	   {
        pMsg->m_pBuffer[i] = pRecevBuff[9+i];
     } 
	   MessageRecv_Flag = IsValidNodeMsg(pMsg);
	   if(MessageRecv_Flag ==1)                           //����������Ч        
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
////   WriteFile( hFile, ( char * )pMsg, iSize );   //��ʱע�͵�
//}











