
// File Name: gprs.c
#include "string.h"
#include "gprs.h"
#include "bsp_SysTick.h"
#include "bsp_usart.h"
#include "AiderProtocol.h"
#include "bsp_rtc.h"
#include "433_Wiminet.h"
#include "common.h"
#include "math.h"
#include "SPI_Flash.h"
#include "DS2780.h"
#include "API-Platform.h"
#include "SensorInit.h"
#include "test.h"

//char  Receive_Monitor_GPRS(void); //3Gģ�鴮�ڼ�������MCU��⵽3Gģ�鴮�ڵ�����ʱ
//void  Receive_Deal_GPRS(void);    //3Gģ��������ݽ�������Ҫ���ڴ���MCU��3Gģ��֮��Ľ�������
//void  Receive_Analysis_GPRS(void);//3Gģ����շ��������ݽ�������Ҫ����������·��Ļ��ڰ��¶�Э�������
//u8    NetStatus_Detection( void );//3Gģ����������״̬���
//void  SetRequest_GPRS(void);      //ͨ��3Gģ������������·�������Ϣ

char      Usart3_recev_buff[1000]={'\0'};     //USART3���ջ���
uint16_t  Usart3_recev_count=0;              //USART3���ռ�����

extern unsigned char Usart3_send_buff[300];
uint8_t    CSQ_OK =0;                         //�ź�������־����


extern u8         Send_Request_flag;                         //������������λ�����յ�����
extern char   		SendData_Flag;                       //�������ݳɹ���־λ�����ͳ�ΪΪ1�����ɹ�Ϊ0��
extern char       TcpConnect_Flag;                     //TCP���ӳɹ���־λ�����ӳɹ�Ϊ1�����ɹ�Ϊ0��


extern struct    SMS_Config_RegPara   ConfigData;     //������·����ò�����HEX��ʽ��������ϵͳ��������ģʽ֮ǰд��BKP�Ĵ���
static char      RespRev_OK =0;              //�ɹ����շ�����Ӧ��
  
extern struct    Sensor_Set  DeviceConfig;   //������Ϣ�ṹ��
extern char      SetRev_OK;                  //�ɹ����շ���������
extern char      Alive;                      //�����߱�־����,Ϊ1ʱ˵�����������ߴ����д�����
extern char      DatRev_OK ;                 //�ɹ���ȷ����Һλ����
extern uint8_t   DataCollectCount;           //���ݲɼ�������
extern uint8_t   LiquidDataSend_Flag;

extern uint8_t   DMA_USART3_RecevBuff[RECEIVEBUFF_SIZE];
extern uint8_t   DMA_UART3_RECEV_FLAG ;      //USART3 DMA���ձ�־����

extern unsigned char  DMA_UART3_RecevDetect(unsigned char RecevFlag,u8* pDeviceID, u16 sNodeAddress);     //USART3�������ݼ�������ݽ���
extern void           DataMessageSend(void);      // ͨ�����ŷ���Һλ����

//void  SIM5216_PowerOn(void)            //��SIM5216ģ��
//{
//   GPIO_SetBits(GPIOC,GPIO_Pin_3);	   //POWER_ON��������
//   Delay_ms(120);                      //100ms��ʱ     64ms<Ton<180ms
//   GPIO_ResetBits(GPIOC,GPIO_Pin_3);   //POWER_ON��������
//   Delay_ms(5500);                     //5s��ʱ        Tuart>4.7s           
//	
//}

//void  SIM5216_PowerOff(void)            //�ر�SIM5216ģ��
//{
//   GPIO_SetBits(GPIOC,GPIO_Pin_3);	    //POWER_ON��������
//   Delay_ms(2000);                      //1s��ʱ       500ms<Ton<5s
//   GPIO_ResetBits(GPIOC,GPIO_Pin_3);    //POWER_ON��������
//   Delay_ms(6000);                             
//}
/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void delay(unsigned int dl_time)
{
   unsigned int i,y;
	 for(i=0;i<5000;i++)
	 {
      for(y=0;y<dl_time;y++);
   }
}
/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART_DataBlock_Send(USART_TypeDef* USART_PORT,char* SendUartBuf,u16 SendLength)    //�����򴮿ڷ�������
{
    u16 i;
        
    for(i=0;i<SendLength;i++)
    {
        USART_SendData(USART_PORT, *(SendUartBuf+i));
        while (USART_GetFlagStatus(USART_PORT, USART_FLAG_TC) == RESET);
    } 
}
void mput_mix(char *str,int length)
{
	
	
	printf("length:%d\r\n",length);             //����ʹ��
  //USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);   //��USART3��������ǰ���ȴ�USART3���տ����жϣ����ڼ�����ݽ������
	USART_DataBlock_Send(USART3,str,length);
	//USART_DataBlock_Send(USART3,"\r\n",2);
	USART_DataBlock_Send(USART1,str,length);
	USART_DataBlock_Send(USART1,"\r\n",2);
}

void mput(char* str)
{
//	printf("length:%d\r\n",strlen(str));     //����ʹ��
//	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);   //��USART3��������ǰ���ȴ�USART3���տ����жϣ����ڼ�����ݽ������
	USART_DataBlock_Send(USART3,str,strlen(str));
	USART_DataBlock_Send(USART3,"\r\n",2);
	USART_DataBlock_Send(USART1,str,strlen(str));
	USART_DataBlock_Send(USART1,"\r\n",2);
}
/*******************************************************************************
* Function Name  : void GPRS_Config(void)
* Description    : GPRS
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPRS_Config(void)
{
	
	 printf("\r\nFUNCTION :GPRS_Config  start! \r\n");                                 //��ʾ����ú���
	 memset(DMA_USART3_RecevBuff,0x00,RECEIVEBUFF_SIZE);
   USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);  //���ô���DMA1����
   mput("AT^UARTRPT=1");  //����ͨ�Žӿ�ΪUART
	 Delay_ms(200);      
	 mput("AT+CMGF=1");     //���Ÿ�ʽ����Ϊ�ı�ģʽ
	 Delay_ms(300); 
	 mput("AT+CPMS=\"ME\",\"ME\",\"ME\"");     //���÷��ͺͽ��ն��Ŵ洢��ΪME��Ϣ�洢��������ʹ��AT+CMGL="REC UNREAD"����鲻������
	 Delay_ms(300); 
   mput("AT+CNMI=1,1");   //����Ϣָʾ����Ϊ�洢����֪ͨ
	 Delay_ms(200);
	
//	 mput("AT+CMGD=1,3");   //����Ϣָʾ����Ϊ�洢����֪ͨ
//	 Delay_ms(500);
//	 mput("AT+CPMS?");   //����Ϣָʾ����Ϊ�洢����֪ͨ
//	 Delay_ms(200);
	
	 printf("\r\nFUNCTION :GPRS_Config  end! \r\n");                                 //��ʾ�����ú���
}
/*******************************************************************************
* Function Name  : void GPRS_Init(void)
* Description    : GPRS
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPRS_Init(void)
{
  u8   NetSearchCount=40;           //��Ѱ��������Դ���
	u8   CSQ_DetectCount=2;           //����ע��ɹ��Ժ��ٴ�������������������ȶ���������
//	char SendArry1[80] ={'\0'};
//  u8   i=0;
	
	printf("\r\nFUNCTION :GPRS_Init  start! \r\n");                                    //��ʾ����ú���
  memset(DMA_USART3_RecevBuff,0x00,RECEIVEBUFF_SIZE);
  USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);  //���ô���DMA1����
//  SIM5216_PowerOn();
	Delay_ms(1000);
	mput("AT^MODECONFIG=19");   //
	while(NetSearchCount!=0)
	{
	   NetSearchCount--;
		 printf("\r\nGPRS Net Searching...\r\n");    //����ʹ��
		 mput("AT+CGREG=1");   //����ź��Զ���������3G�źź���ʱ�Զ��л���3G�ź�
		 Delay_ms(800);
		 mput("AT+CGREG?");   //����ź��Զ���������3G�źź���ʱ�Զ��л���3G�ź�
		 Delay_ms(800);

		 DMA_UART3_RecevDetect( NETLOGIN,0,0);
		 if(CSQ_OK ==1) 
		 {
        CSQ_OK =0;
				CSQ_DetectCount--;
				if(CSQ_DetectCount==0)
				{
           break;
        } 
     }
//     DS2780_Test();   //����ʹ��		 
  }
	if(NetSearchCount==0)
	{
     gotoSleep(DeviceConfig.CollectPeriod );
  }

		printf("\r\nFUNCTION :GPRS_Init  end! \r\n");
}
/*******************************************************************************
* Function Name  : void TCP_Connect(void)
* Description    : GPRS
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
unsigned char TCP_StatusQuery(void)
{
	  unsigned char  TcpStatusFlag =0;  //TCP����״̬��־����
	  int i;
	  for(i=0;i<4;i++)
	{
	  //mput("AT+MIPOPEN?"); //��ѯSocket�������
		Delay_ms(800);	
		TcpStatusFlag =DMA_UART3_RecevDetect( TCPCONNECT ,0, 0);  
		if(TcpStatusFlag==1)
		{
		break;	
		}
	}		
    return TcpStatusFlag;
}

/*******************************************************************************
* Function Name  : void TCP_Connect(void)
* Description    : GPRS
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TCP_Connect(void)
{
    char SendArry[50] ={'\0'};
    unsigned char  TcpConnectFlag =0;  //TCP���ӽ�����־����
    unsigned char  NetErrorCount =10;   //TCP�������ӳ��Դ�������һ����ſ�����  
#if DEBUG_TEST
	  printf(" Multi GPRS Start!\r\n");
    printf(" TCP Start Connect!\r\n");              //������ʾ
#endif
    GPRS_Init();

		while(NetErrorCount>0)
		{
				 
			 mput("at+mipcall=1");
			 Delay_ms(800);		
			 snprintf(SendArry,sizeof(SendArry),"at+miptrans=1,\"%s\",%s",DeviceConfig.ServerIP,DeviceConfig.ServerPort);  // mput("at+miptrans=1,\"58.210.41.202\",2015");   //������Ҫ�ӼĴ����ж�ȡ���д���һ������
//       snprintf(SendArry,sizeof(SendArry),"AT+MIPOPEN=1,\"TCP\",\"%s\",%s,10000",DeviceConfig.ServerIP,DeviceConfig.ServerPort);  // mput("at+miptrans=1,\"58.210.41.202\",2015");   //������Ҫ�ӼĴ����ж�ȡ���д���һ������			
       mput(SendArry);
			 Delay_ms(5000); 
			 //mput("AT+MIPOPEN?"); //��ѯSocket�������                   ע��9.9
			 Delay_ms(800);	
			 TcpConnectFlag =DMA_UART3_RecevDetect( TCPCONNECT ,0 ,0 );    //��GPRSģ����յ������ݽ��н���
			 NetErrorCount--;
			 if(TcpConnectFlag==1) 
			 {
				 
				 printf("\r\nThe TCP connect is OK\r\n");
         //mput("AT+MIPMODE=1,1"); //����SOCKETģʽΪHex����ģʽ
         Delay_ms(500);	
				 
//			mput("AT+MIPSEND=1,5"); //���Ͳ���
//			mput("11111");
				 break;
       }
		}
		if(NetErrorCount == 0)
		{
			printf("\r\nThe Client Could Not Connect TCP!\r\n");
       TCP_Disconnect();
			 Delay_ms(2000);
			 //TCP_Connect();                              //��֤TCP���ӳɹ�
			 gotoSleep(DeviceConfig.CollectPeriod );     //��γ���δ���ӵ����磬ֱ�ӽ�������ģʽ�����ڼ��ģ�黽���Ժ��һ������״̬
    } 
#if DEBUG_TEST
		printf(" TCP end Connect!\r\n");                    //������ʾ
#endif		
}
/*******************************************************************************
* Function Name  : void TCP_Disconnect(void)
* Description    : GPRS
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TCP_Disconnect(void)
{
#if DEBUG_TEST
	printf("\r\nTCP link is disconnect!\r\n");
#endif
  mput("+++");
	//mput("AT+MIPCLOSE=1");
//	TCP_Connect_Flag =0;    //�ر�ѭ������
	Delay_ms(500);
}
/*******************************************************************************
* Function Name  : char* Find_String(char* Source, char* Object)
* Description    : ��Ŀ���ַ����з���һ��ָ�����ַ���
* Input          : 
* Output         : 
* Return         : ����ҵ����򷵻�Ŀ���ַ�����Դ�ַ����е��׵�ַ
*******************************************************************************/
char* Find_String(char* Source, char* Object)
{
	char*   Ptemp1 = Source;
	char*   Ptemp2 = Object;
	short   Length_Source =0;
	short   Length_Object =0;
	short   i=0,j=0;
	short   count=0;
	
	
	Length_Source = strlen(Source);
	Length_Object = strlen(Object);

	if(Length_Source < Length_Object)
	{
     return NULL;
  }
  
	else if(Length_Source == Length_Object)
	{
		 if((Length_Source==0)&&(Length_Object==0))
		 {
			  return NULL;
     }  
		 else  
		 { 
			  for(i=0;i<Length_Source;i++)
		    {
				   if(Ptemp1[i] != Ptemp2[i])
					 return NULL;
        }
				return Ptemp1;
     }	  
  }
	else 
	{
		 if(Length_Object == 0)
		 {
			  return NULL;
     }  
		 else  
		 {  count = Length_Source - Length_Object + 1;
			  for(i=0;i<count;i++)
		    {  for(j=0;j<Length_Object;j++)
					 {
               if(Ptemp1[i+j] != Ptemp2[j])
						   break;
           }
					 if(j==Length_Object)
					 return  &Ptemp1[i]; 
				 
        }
				return NULL;
     }	  
  }
}
/***********�������ܣ����ض������з���һ��ָ��������****************/
/***********����ҵ����򷵻�Ŀ��������Դ�����е��׵�ַ**************/
char* Find_SpecialString(char* Source, char* Object, short Length_Source, short Length_Object)  
{
	char*   Ptemp1 = Source;
	char*   Ptemp2 = Object;
	short   i=0,j=0;
	short   count=0;
	
	if((Length_Source < 0)||(Length_Object < 0))
	{
     return NULL;
  }
  if(Length_Source < Length_Object)
	{
     return NULL;
  }
  
	else if(Length_Source == Length_Object)
	{
		 if((Length_Source==0)&&(Length_Object==0))
		 {
			  return NULL;
     }  
		 else  
		 { 
			  for(i=0;i<Length_Source;i++)
		    {
				   if(Ptemp1[i] != Ptemp2[i])
					 return NULL;
        }
				return Ptemp1;
     }	  
  }
	else 
	{
		 if(Length_Object == 0)
		 {
			  return NULL;
     }  
		 else  
		 {  
			  count = Length_Source - Length_Object + 1;
			  for(i=0;i<count;i++)
		    {  for(j=0;j<Length_Object;j++)
					 {
               if(Ptemp1[i+j] != Ptemp2[j])
						   break;
           }
					 if(j==Length_Object)
					 {
							return  &Ptemp1[i]; 
					 }
				 
        }
				return NULL;
     }	  
  }
}
/********************�������ܣ����ŷ���ǰԤ����*********************/
/*************������Ҫ���͵������������һ��������0x1a**************/
void SendPretreatment(char* pSend)
{  
   uint8_t  i=0;
	 char*    pTemp =NULL;
	
	 i= strlen(pSend);
	 pTemp =pSend+i;
   *(pTemp) =0x1a;
}

/***********************�������ܣ����Ͷ���**************************/
/*******************************************************************/
//���Ͷ������̣�
//��һ�������ý��ն������Ѹ�ʽ��AT+CNMI=1,2,0,0,0    
//�ڶ������趨���Ž��շ����룺AT+CMGS="15116924685"
//�����������Ͷ������ģ�����16����0x1a��Ϊ��β
void Sms_Send(char*  pSend)
{ 
//	uint8_t  PhoneNum[13]={0x00};    //��������Ϊ�ӼĴ����ж�ȡ
	char  SendBuf[200]={0x00};                //���ŷ��ͻ���,���������ܵķ������ݳ���ȷ�����С

//	struct Sensor_Set* Para= &DeviceConfig;
	
//	memcpy(PhoneNum, DeviceConfig.AlarmPhoneNum,strlen((char*)DeviceConfig.AlarmPhoneNum));          
//	mput("AT+CNMI=1,2,0,0,0");
	
  memset(SendBuf,0,sizeof(SendBuf));
	snprintf(SendBuf,sizeof(SendBuf),"AT+CMGS=\"%s\"",ConfigData.CurrentPhoneNum); //���Ͷ��ź���Ϊ��ǰͨ���ֻ�����
	mput(SendBuf);
//	mput("AT+CMGS=\"861064617006426\"");  //����ʹ�ã��д�����

	Delay_ms(500);
  memset(SendBuf,0,sizeof(SendBuf));
	snprintf(SendBuf, (sizeof(SendBuf)-1), pSend, strlen(pSend));
  SendPretreatment(SendBuf);
	mput(SendBuf);
  Delay_ms(1000);  
//	DMA_UART3_RecevDetect();  //�ȴ�������Ϣ

}

void AlarmSMS_Send(char*  pSend)  //���ͱ�������
{ 
	char  SendBuf[200]={0x00};                //���ŷ��ͻ���,���������ܵķ������ݳ���ȷ�����С

  memset(SendBuf,0,sizeof(SendBuf));
	snprintf(SendBuf,sizeof(SendBuf),"AT+CMGS=\"%s\"",DeviceConfig.AlarmPhoneNum); //���Ͷ��ź���ΪԤ�豨���ֻ�����
	mput(SendBuf);

	Delay_ms(500);
  memset(SendBuf,0,sizeof(SendBuf));
	snprintf(SendBuf, (sizeof(SendBuf)-1), pSend, strlen(pSend));
  SendPretreatment(SendBuf);
	mput(SendBuf);
  Delay_ms(1000);  
}


/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t Char2Hex(uint8_t* HexArry, char* CharArry, uint8_t Length_CharArry)
{
	uint8_t   i=0,j=0;         
	uint8_t   val=0;     
//	char*     pChar = CharArry;
	
  for(i=0; i<Length_CharArry; i++)
	{
		
//		val = *pChar;
		val = CharArry[i];
		if((val >= '0')&&(val <= '9'))
		{
			HexArry[j++] = val-'0';	
		}
		else if(val == '.')
		{
			HexArry[j++] = val;	
		}
//		pChar++;
	}	
  return  j;	
}

/**************************************�ַ�ת��Ϊ������******************************************/
/**************************************���ڼ��㱨����ֵ******************************************/
float  char_to_float(char* pSetPara)
{
	char CharTemp[10]={0x00};
	int i,j;
	int IntegerPart=0;
	int DecimalPart=0;
	int Counter=0;
	float number=0.0;
	for(j=0;j<strlen(pSetPara);j++)                       //���ú����Ѿ����˷��������,strlen(pSetPara)<=5
			 {
          if((pSetPara[j]>='0')&&(pSetPara[j]<='9'))
					{
             CharTemp[j] =pSetPara[j];
          }
				  else
					{
						 break;
          }
       }
			 if(strlen(CharTemp)>2)
			 {
          printf("\r\nInput ERROR!!\r\n");
          return 0;
       } 
			
			 for(i=strlen(CharTemp),j=0; i>=1; i--,j++)
			 {
				 
         IntegerPart =IntegerPart + (CharTemp[i-1]-'0')*(int)(pow(10,j));   //���㱨����ֵ��������
       
				 
       }
			 
			 Counter =strlen(CharTemp)+1; //����С����
			 memset(CharTemp,0x00,sizeof(CharTemp));
			 for(i=0,j=Counter;j<strlen(pSetPara);j++,i++)                     
			 {
          if((pSetPara[j]>='0')&&(pSetPara[j]<='9'))
					{
             CharTemp[i] =pSetPara[j];
          }
				  else
					{
						 break;
          }
       }
			 if(strlen(CharTemp)>2)
			 {
          printf("\r\nInput Alarm Threshold ERROR!!\r\n");
          return 0;
       }
       for(i=strlen(CharTemp),j=0; i>=1; i--,j++)
			 {
         if(strlen(CharTemp)==1)
				 {
           DecimalPart =(CharTemp[i-1]-'0')*10;
					 break;
         }
				 else
				 {
           DecimalPart =DecimalPart + (CharTemp[i-1]-'0')*(int)(pow(10,j));  //���㱨����ֵС������
         }
       }
	
	     number=DecimalPart*0.01 + IntegerPart;
			 return(number);
}


/**************************************�ַ�ת��Ϊ����******************************************/
uint16_t char_to_int(char* pSetPara)
{
	char CharTemp[10]={0x00};
	int i,j;
  int length;
	int IntegerPart=0;


	for(j=0;j<strlen(pSetPara);j++)                       //���ú����Ѿ����˷��������,strlen(pSetPara)<=5
			 {
          if((pSetPara[j]>='0')&&(pSetPara[j]<='9'))
					{
             CharTemp[j] =pSetPara[j];
          }
				  else
					{
						 break;
          }
       }
			 
			 if(strlen(CharTemp)>5)
			 {
          printf("\r\nInput  ERROR!!\r\n");
          return 0;
       } 
			 
			 
			 length=strlen(CharTemp);
	  
			 for(i=0;i<length;i++)
			 printf("----the data is %c -----",CharTemp[i]);
			 
       
			 
			 IntegerPart = IntegerPart + (CharTemp[length-1]-'0');
				
			 if(length>1)			 
			 for(i=1; i<strlen(CharTemp); i++)
			 {
				
				IntegerPart =IntegerPart + (CharTemp[length-1-i]-'0')*(int)(pow(10,i));  //
					
       }

	return (IntegerPart);
}




//}
/*******************************************************************************
* Function Name  : XX
* Description    : ����δ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Sms_Consult(u8* pDeviceID, u16 sNodeAddress)
{
#if DEBUG_TEST
  	printf("\r\nFUNCTION :Sms_Consult  start! \r\n");                                 //��ʾ����ú���
#endif	
    mput("AT+CMGL=\"REC UNREAD\"");     //��ȡδ����Ϣ
//	  mput("AT+CMGL=\"ALL\"");     //��ȡδ����Ϣ
//	  mput("AT+CMGL=0");     //��ȡδ����Ϣ
		Delay_ms(1500); 
	  DMA_UART3_RecevDetect( DATARECEV, pDeviceID, sNodeAddress );		
//		mput("AT+CMGD=,3");      //ɾ���Ѷ���Ϣ
		Delay_ms(2500);            //�ȴ�ģ���л������ݴ���״̬
#if DEBUG_TEST		
	printf("\r\nFUNCTION :Sms_Consult  end! \r\n");                                 //��ʾ�˳��ú���
#endif
}

/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/***********************�������ܣ��������ý���**************************/
/*******************************************************************/
//AT+CNMI=1,2,0,0,0                                //���ն�������
//+CMT: "+8613121342836","","15/08/18,17:05:29+32" //���յ��Ķ��ű��ĸ�ʽ���س����У�0D 0A��֮��Ϊ��Ϣ����
//ok                                               //��Ϣ����
//AT+CNMI=2,1                                      //���ն�������
//+CMTI: "ME",1                                    //���յ��¶�������
//AT+CMGL="REC UNREAD"                             //�跢�Ͳ�ѯδ����������
//+CMGL: 1,"REC UNREAD","+8613121342836","","15/08/18,17:12:19+32"   //���յ��Ķ��ű��ĸ�ʽ���س����У�0D 0A��֮��Ϊ��Ϣ����
//ok                                               //��Ϣ����

void Sms_Analysis(char* pBuff)
{ 

//	char      MessageRead[11] ="REC UNREAD";                            //��ѯ��δ�����Ŵ���
	char      MessageRecevIndicate[5] ="+32\"";                         //���յ�����ָʾ���ַ���
	////////////////////////////////��������///////////////////////////////////////////////////////////////////////
	char      AlarmPhoneSet[25] ="casic_set_alarm_phone_";              //����������������
	char      ServerIpSet[25]   ="casic_set_server_ip_";                //������IP��������
	char      ServerPortSet[25] ="casic_set_server_port_";              //�������˿ں���������
	char      StartCollectTime[30] ="casic_start_collect_time_";              //�״βɼ�ʱ������
	char      CollectPeriod[30]    ="casic_collect_period_";                 //�ɼ���������
	char 			SendCount[30]        ="casic_send_count_";                     //�ϴ���������
	char 			RetryNum[30]         ="casic_retry_number_";                       //�����ϴ���������
	char      LowAlarmThresholdSet[35] ="casic_set_low_alarm_threshold_";      //��Ũ�ȱ�����ֵ��������
	char      HighAlarmThresholdSet[35] ="casic_set_high_alarm_threshold_";      //��Ũ�ȱ�����ֵ��������
	
  char 			SensorDataInquire[20]=	"casic_current_data";           //���Ż�ȡ����������
  char      SensorSetInquire[20] =  "casic_set_parameter";          //���Ż�ȡ���������ò���			
//////////////////////////////////�������鶨�����////////////////////////////////////////////////////////////////////
	
	
	char      CurrentPhoneNum[16]    ={0x00};                              //�洢��ǰͨ���ֻ�����
  
	
	char      ReceiveTemp[16] ={0x00};                     //��ʱ����������
  char      MessageSend[200] ={0x00};                     //���ŷ���������
	
	
	
	char*     pSmsRecevBuff =NULL;            //������������λ��ָ��
  char*     pSmsRecevData =NULL;            //����Ŀ������λ��ָ��
  uint8_t   UploadFlag =0;                  //����ָʾ�������ò����޸Ľ��
	uint8_t   i=0;                            //ѭ������
  uint16_t  temp=0;                          //��ʱ��������
	uint8_t   interget=0;
	uint8_t   decimal=0;
  u8   j=0;


	pSmsRecevBuff = Find_String(pBuff,"\"REC UNREAD\""); //����Ƿ��յ�����
	if(pSmsRecevBuff !=NULL)
	{
		if(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-29))) //��ָֹ��Խ��
		{
      
			 pSmsRecevBuff = pSmsRecevBuff+15;   //ָ��ָ���ֻ�����ǰ׺86		 REC UNREAD ��  +86֮��Ϊ15λ

			 for(i=0;i<16;i++)
			 {
          if(*pSmsRecevBuff == '\"')
					{
              break;
          }
					CurrentPhoneNum[i] = *pSmsRecevBuff; 
          pSmsRecevBuff++;					
       }
    }
		if(i>=13)
		{
        memcpy(ConfigData.CurrentPhoneNum,CurrentPhoneNum,i); //��ȡ��ǰͨ���ֻ����룬��ǰ׺86
			  printf("\r\nCurrentPhoneNum:%s\r\n",CurrentPhoneNum); //����ʹ��
    }
	   pSmsRecevBuff =NULL;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		
		  pSmsRecevBuff = Find_String(pBuff,"+32\"");         //����Ƿ��յ�����
//	   pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, MessageRecevIndicate,sizeof(Usart3_recev_buff),strlen(MessageRecevIndicate));    //����Ƿ��յ����ţ����˶��Ų�ѯ����
		 if((pSmsRecevBuff !=NULL) &&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(MessageRecevIndicate)))))   //��ָֹ��Խ��
		 {
       pSmsRecevData =pSmsRecevBuff +strlen(MessageRecevIndicate);                          //����ʹ��
			 pSmsRecevBuff =NULL;   
			 printf("\r\nReceive Short Message is: %s\r\n",pSmsRecevData);    //����ʹ��
/*****************************************�������ù���*************************************************************/		 			 
//(2)���òɼ�����			 
         //pSmsRecevBuff = Find_String(pBuff,"casic_start_collect_time_");    //���òɼ�����
   			 pSmsRecevBuff = Find_SpecialString(pBuff, CollectPeriod,strlen(pBuff),strlen(CollectPeriod));   //���òɼ�����
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(CollectPeriod)-1))))
			 {
          pSmsRecevData =pSmsRecevBuff +strlen(CollectPeriod);   
				  for(i=0;i<4;i++)           
				  {

						 if(((pSmsRecevData[i]>='0')&&(pSmsRecevData[i]<='9')))
						 {;}
						 else
						 {
							  break;
             }
          }
					if(i==0)
					{
             printf("\r\nMessage set input ERROR!!\r\n");     //����ʹ��
          }
				  else 
					{
						 if(i>4)
						 {
               i=4;  //i��ʾ��Ч�����Ƴ��ȣ���ֹ���
             }
				
             memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);		                //תΪ10����						 DeviceConfig.CollectPeriod  = char_to_int(AlarmThresholdTemp);
						 DeviceConfig.CollectPeriod = char_to_int(ReceiveTemp);
						 ConfigData.CollectPeriod_Byte  [0] = DeviceConfig.CollectPeriod >>8; 
						 ConfigData.CollectPeriod_Byte  [1] = DeviceConfig.CollectPeriod &0xff;
						 	printf("\r\n-----data before write in flash!----%s----",(char*)ReceiveTemp);                   //��д��FLASH֮ǰ���������
						  printf("\r\n-----data before write in flash!----%d----",DeviceConfig.CollectPeriod);                   //��д��FLASH֮ǰ���������
				       for(j=0;j<2;j++)
						 printf("\r\n-----data before write in flash!----%x----",ConfigData.CollectPeriod_Byte [j]);  
						 
						 UploadFlag = UploadFlash((char*)ConfigData.CollectPeriod_Byte  , 2);
}		
				  if(UploadFlag ==1)
					{
              printf("\r\nStart CollectPeriod upload success!!\r\n");    //����ʹ��
						  Delay_ms(2000);  //���ŷ��ͻ���
						  Sms_Send("\rStart CollectPeriod upload success!!\r");
						  
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //��������������
						  snprintf( MessageSend , sizeof(MessageSend),"\rcollect period is %d \r",    DeviceConfig.CollectPeriod  );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //����Ҫ���͵Ķ������ô�����ʾ
						  Delay_ms(2000);  //���ŷ��ͻ���
						  AlarmSMS_Send(MessageSend);                     //���ŷ���������Ϣ
          }
				  else           //�ԷǷ�����ֵ������ʾ����ֵ�����Ƿ������Զ�ȡ����ֵ��
				  {
             printf("\r\nInput CollectPeriod not correct!\r\nPlease check and retry.\r\n");    //����ʹ��
						 Delay_ms(2000);  //���ŷ��ͻ���
						 Sms_Send("\rInput CollectPeriod not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //��λ����
				  pSmsRecevBuff = NULL;    //��λ����
       }	
			 
			 //	char      StartCollectTime[30]="casic_start_collect_time_";              //�״βɼ�ʱ������
//	char      CollectPeriod[30]   ="casic_collect_period_";                 //�ɼ���������
//	char 			SendCount[30]       ="casic_send_count_";                     //�ϴ���������

//	char 			RetryNum[30]        ="casic_retry_number_";                       //�����ϴ���������

//(3)�����ϴ�����			 
         //pSmsRecevBuff = Find_String(pBuff,"casic_start_collect_time_");    //�����ϴ�����
   			 pSmsRecevBuff = Find_SpecialString(pBuff, SendCount,strlen(pBuff),strlen(SendCount));   //�����ϴ�����
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(SendCount)-1))))
			 {
          pSmsRecevData =pSmsRecevBuff +strlen(SendCount);   
				  for(i=0;i<4;i++)           
				  {
//             if(pSmsRecevData[i]==0x0D)
//						 {
//							  break;
//             }
						 if(((pSmsRecevData[i]>='0')&&(pSmsRecevData[i]<='9')))
						 {;}
						 else
						 {
							  break;
             }
          }
					if(i==0)
					{
             printf("\r\nMessage set input ERROR!!\r\n");     //����ʹ��
          }
				  else 
					{
						 if(i>4)
						 {
               i=4;  //i��ʾ��Ч�����Ƴ��ȣ���ֹ���
             }
					
          
             memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);		
						 
						 DeviceConfig.SendCount   = char_to_int(ReceiveTemp);
						 ConfigData.SendCount_Byte   [0] = DeviceConfig.SendCount  >> 8; 
						 ConfigData.SendCount_Byte   [1] = DeviceConfig.SendCount  &  0xff;

						 	printf("\r\n-----data before write in flash!----%s----",(char*)ReceiveTemp);                   //��д��FLASH֮ǰ���������
						  printf("\r\n-----data before write in flash!----%d----",DeviceConfig.SendCount);                   //��д��FLASH֮ǰ���������
						  for(j=0;j<2;j++)
						 printf("\r\n-----data before write in flash!----%x----",ConfigData.SendCount_Byte [j]);  
				     UploadFlag = UploadFlash((char*)ConfigData.SendCount_Byte    , 3);
}		
				  if(UploadFlag ==1)
					{
              printf("\r\nStart SendCount upload success!!\r\n");    //����ʹ��
						  Delay_ms(2000);  //���ŷ��ͻ���
						  Sms_Send("\rSendCount upload success!!\r");
						  Delay_ms(2000);  //���ŷ��ͻ���
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //��������������
						  snprintf( MessageSend , sizeof(MessageSend),"\rsend count time is %d \r",    DeviceConfig.SendCount   );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //����Ҫ���͵Ķ������ô�����ʾ
						  Delay_ms(2000);  //���ŷ��ͻ���
						  AlarmSMS_Send(MessageSend);                     //���ŷ���������Ϣ           //
          }
				  else           //�ԷǷ�����ֵ������ʾ����ֵ�����Ƿ������Զ�ȡ����ֵ��
				  {
             printf("\r\nInput SendCount not correct!\r\nPlease check and retry.\r\n");    //����ʹ��
						 Delay_ms(2000);  //���ŷ��ͻ���
						 Sms_Send("\rInput SendCount not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //��λ����
				  pSmsRecevBuff = NULL;    //��λ����
       }
			 
//(4)���ñ����绰����
			 // pSmsRecevBuff = Find_String(pBuff,"casic_set_alarm_phone_");  //���ñ�������
   		 pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, AlarmPhoneSet,sizeof(Usart3_recev_buff),strlen(AlarmPhoneSet));  
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(AlarmPhoneSet)-15))))
			 {
          pSmsRecevData =pSmsRecevBuff +strlen(AlarmPhoneSet);
          for(i=0;i<16;i++)           
				  {

						 if((pSmsRecevData[i]>='0')&&(pSmsRecevData[i]<='9'))
						 { ; }
						 else
						 {
							  break;
             }
          }
				 if((i>15)||(i<11))    //�������ݲ��Ϸ���ֱ�Ӷ�����������������Ϣ��������������ʾ
					{
						 printf("\r\nMessage set input ERROR!!\r\n");     //����ʹ��
//						 i=16;  //i��ʾ��Ч������ֵ���ȣ����Ƴ��ȣ���ֹ���
          }		
					else
					{
             memset(DeviceConfig.AlarmPhoneNum,0x00,sizeof(DeviceConfig.AlarmPhoneNum));
				     memcpy(DeviceConfig.AlarmPhoneNum, pSmsRecevData, i);
				     UploadFlag = UploadFlash(DeviceConfig.AlarmPhoneNum , 4);
          }
				  if(UploadFlag ==1)
					{
              printf("\r\nAlarm phone number upload success!!\r\n");    //����ʹ��
							Delay_ms(2000);  //���ŷ��ͻ���
						  Sms_Send("\rAlarm phone number upload success!!\r");
						  Delay_ms(2000);  //���ŷ��ͻ���
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //��������������
						  snprintf( MessageSend , sizeof(MessageSend),"\rAlarm phone number is %s \r",    DeviceConfig.AlarmPhoneNum );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //����Ҫ���͵Ķ������ô�����ʾ
						  Delay_ms(2000);  //���ŷ��ͻ���
						  AlarmSMS_Send(MessageSend);                     //���ŷ���������Ϣ
          }
					else
					{
              printf("\r\nAlarm phone number not correct!\r\nPlease check and retry.\r\n");    //����ʹ��
							Delay_ms(2000);  //���ŷ��ͻ���
						  Sms_Send("\rAlarm phone number not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //��λ����
				  pSmsRecevBuff = NULL;    //��λ����
       }
//(5)�����������IP����			 
			 pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, ServerIpSet,sizeof(Usart3_recev_buff),strlen(ServerIpSet));  //���÷�����IP
			// pSmsRecevBuff = Find_String(pBuff,"casic_set_server_ip_");    //���÷�����IP
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(ServerIpSet)-16))))
			 {
          pSmsRecevData =pSmsRecevBuff +strlen(ServerIpSet);  
				  for(i=0;i<16;i++)           
				  {
             if(((pSmsRecevData[i]>='0')&&(pSmsRecevData[i]<='9'))||(pSmsRecevData[i]=='.'))
						 {;}
						 else
						 {
							  break;
             }
          }
				  if((i>15)||(i<7))  //�������ݲ��Ϸ���ֱ�Ӷ�����������������Ϣ��������������ʾ
					{
						 printf("\r\nMessage set input ERROR!!\r\n");     //����ʹ�� 	
//					 i=15;  //i��ʾ��ЧIP��ַ���ȣ����Ƴ��ȣ���ֹ���
          }	
          else
					{
             memset(DeviceConfig.ServerIP,0x00,sizeof(DeviceConfig.ServerIP));
						 memcpy(DeviceConfig.ServerIP, pSmsRecevData, i);		
						 UploadFlag = UploadFlash(DeviceConfig.ServerIP, 5);
          }						
				  if(UploadFlag ==1)
					{
              printf("\r\nServer IP upload success!!\r\n");    //����ʹ��
					  	Delay_ms(2000);  //���ŷ��ͻ���
						  Sms_Send("\rServer IP upload success!!\r");
						  Delay_ms(2000);  //���ŷ��ͻ���
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //��������������
						  snprintf( MessageSend , sizeof(MessageSend),"\rserver IP is %s \r",    DeviceConfig.ServerIP  );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //����Ҫ���͵Ķ������ô�����ʾ
						   Delay_ms(2000);  //���ŷ��ͻ���
						  AlarmSMS_Send(MessageSend);                     //���ŷ���������Ϣ
          }
				  else           //�ԷǷ�����ֵ������ʾ����ֵ�����Ƿ������Զ�ȡ����ֵ��
				  {
             printf("\r\nInput server IP not correct!\r\nPlease check and retry.\r\n");    //����ʹ��
						 Delay_ms(2000);  //���ŷ��ͻ���
						 Sms_Send("\rInput server IP not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //��λ����
				  pSmsRecevBuff = NULL;    //��λ����
       }
//(6)�����������˿ں�			 
			 pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, ServerPortSet,sizeof(Usart3_recev_buff),strlen(ServerPortSet));   //���÷������˿ں�
//			 pSmsRecevBuff = Find_String(pBuff,"casic_set_server_port_");    //���÷������˿ں�
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(ServerPortSet)-6))))
			 {
          pSmsRecevData =pSmsRecevBuff +strlen(ServerPortSet);  
				  for(i=0;i<6;i++)
				  {

						 if((pSmsRecevData[i]>='0')&&(pSmsRecevData[i]<='9'))
						 {;}
						 else
						 {
							  break;
             }
          }
				  if((i<2)||(i>5))    //�������ݲ��Ϸ���ֱ�Ӷ�����������������Ϣ��������������ʾ���˿ںŷ�Χ�д�ȷ��
					{
						 printf("\r\nMessage set input ERROR!!\r\n");     //����ʹ�� 	
//						 i=5;  //i��ʾ��Ч�˿ںų��ȣ����Ƴ��ȣ���ֹ���
          }	
   				else
					{
             memset(DeviceConfig.ServerPort,0x00,sizeof(DeviceConfig.ServerPort));
             memcpy(DeviceConfig.ServerPort, pSmsRecevData, i);				 
				     UploadFlag = UploadFlash(DeviceConfig.ServerPort, 6);
          }						
				  if(UploadFlag ==1)
					{
              printf("\r\nServer port upload success!!:%s\r\n",pSmsRecevData);    //����ʹ��
					  	Delay_ms(2000);  //���ŷ��ͻ���
						  Sms_Send("\rServer port upload success!!\r");
						  Delay_ms(2000);  //���ŷ��ͻ���
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //��������������
						  snprintf( MessageSend , sizeof(MessageSend),"\rServer port is %s\r",    DeviceConfig.ServerPort  );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //����Ҫ���͵Ķ������ô�����ʾ
						  Delay_ms(2000);  //���ŷ��ͻ���
						  AlarmSMS_Send(MessageSend);                     //���ŷ���������Ϣ
          }
				  else           //�ԷǷ�����ֵ������ʾ����ֵ�����Ƿ������Զ�ȡ����ֵ��
				  {
             printf("\r\nInput server port not correct!\r\nPlease check and retry.\r\n");    //����ʹ��
					   Delay_ms(2000);  //���ŷ��ͻ���
						 Sms_Send("\rInput server port not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //��λ����
				  pSmsRecevBuff = NULL;    //��λ����
       }
			 
//(7)���õ�Ũ����ֵ			 
			// pSmsRecevBuff = Find_String(pBuff,"casic_set_alarm_threshold_");    //���õ�Ũ�ȱ�����ֵ
			 pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, LowAlarmThresholdSet,sizeof(Usart3_recev_buff),strlen(LowAlarmThresholdSet));   //���ñ�����ֵ
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(LowAlarmThresholdSet)-3))))          //��Ũ����ֵ���ݣ�XX.XX
			 {
          pSmsRecevData =pSmsRecevBuff +strlen(LowAlarmThresholdSet);   
				  for(i=0;i<5;i++)           
				  {

						 if(((pSmsRecevData[i]>='0')&&(pSmsRecevData[i]<='9'))||(pSmsRecevData[i]=='.'))
						 {;}
						 else
						 {
							  break;
             }
          }
					if(i==0)
					{
             printf("\r\nMessage set input ERROR!!\r\n");     //����ʹ��
          }
				  else 
					{
						 if(i>5)
						 {
               i=5;  //i��ʾ��Ч������ֵ���ȣ����Ƴ��ȣ���ֹ���
             }
						 memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);	
						
             DeviceConfig.LowAlarmLevel .Data_Float = char_to_float(ReceiveTemp); 
           					
						printf("\r\n-----data before write in flash!----%f----",DeviceConfig.LowAlarmLevel  .Data_Float);                   //��д��FLASH֮ǰ���������
 				    UploadFlag = UploadFlash((char*)DeviceConfig.LowAlarmLevel.Data_Hex ,7);
          }		
				  if(UploadFlag ==1)
					{
              printf("\r\nlow Alarm threshold upload success!!\r\n");    //����ʹ��
						  Delay_ms(2000);  //���ŷ��ͻ���
						  Sms_Send("\rlow Alarm threshold upload success!!\r");
						  Delay_ms(2000);  //���ŷ��ͻ���
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //��������������
						  snprintf( MessageSend , sizeof(MessageSend),"\rlow alarm level is %0.2f \r",    DeviceConfig.LowAlarmLevel .Data_Float  );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //����Ҫ���͵Ķ������ô�����ʾ
						  Delay_ms(2000);  //���ŷ��ͻ���
						  AlarmSMS_Send(MessageSend);                     //���ŷ���������Ϣ
          }
				  else           //�ԷǷ�����ֵ������ʾ����ֵ�����Ƿ������Զ�ȡ����ֵ��
				  {
             printf("\r\nInput alarm threshold not correct!\r\nPlease check and retry.\r\n");    //����ʹ��
						 Delay_ms(2000);  //���ŷ��ͻ���
						 Sms_Send("\rInput alarm threshold not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //��λ����
				  pSmsRecevBuff = NULL;    //��λ����
       }

///////////////////////////////////////////////�������ò���/////////////////////////////////////////////////////////
			 
//	char      StartCollectTime[30]="casic_start_collect_time_";              //�״βɼ�ʱ������
//	char      CollectPeriod[30]   ="casic_collect_period_";                 //�ɼ���������
//	char 			SendCount[30]       ="casic_send_count_";                     //�ϴ���������
//	char 			RetryNum[30]        ="casic_retry_number_";                       //�����ϴ���������

//(8)���õ�һ�βɼ�ʱ��			 
         //pSmsRecevBuff = Find_String(pBuff,"casic_start_collect_time_");    //���õ�һ�βɼ�ʱ��
   			 pSmsRecevBuff = Find_SpecialString(pBuff, StartCollectTime,strlen(pBuff),strlen(StartCollectTime));   //���õ�һ�βɼ�ʱ��
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(StartCollectTime)-1))))
			 {
          pSmsRecevData =pSmsRecevBuff +strlen(StartCollectTime);   
				  for(i=0;i<4;i++)           
				  {

						 if(((pSmsRecevData[i]>='0')&&(pSmsRecevData[i]<='9')))
						 {;}
						 else
						 {
							  break;
             }
          }
					if(i==0)
					{
             printf("\r\nMessage set input ERROR!!\r\n");     //����ʹ��
          }
				  else 
					{
						 if(i>4)
						 {
               i=4;  //i��ʾ��Ч��һ�βɼ�ʱ�䣬���Ƴ��ȣ���ֹ���
             }
						 memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);
						 
						 DeviceConfig.CollectStartTime = char_to_int(ReceiveTemp);
						 
					  ConfigData.CollectStartTime_Byte [0] = DeviceConfig.CollectStartTime >> 8; 
					  ConfigData.CollectStartTime_Byte [1] = DeviceConfig.CollectStartTime & 0xff;
						 //printf("\r\n-----data before write in flash!----%s----",CollectStartTime_data);                   //��д��FLASH֮ǰ���������
						 printf("\r\n-----data before write in flash!----%d----",DeviceConfig.CollectStartTime);                   //��д��FLASH֮ǰ���������
				    for(j=0;j<2;j++)
						 printf("\r\n-----data before write in flash!----%x----",ConfigData.CollectStartTime_Byte [j]);  
						 
						 UploadFlag = UploadFlash((char*)ConfigData.CollectStartTime_Byte   , 8);
          }		
				  if(UploadFlag ==1)
					{
              printf("\r\nStart Collect Time upload success!!\r\n");    //����ʹ��
						  Delay_ms(2000);  //���ŷ��ͻ���
						  Sms_Send("\rStart Collect Time upload success!!\r");
						  Delay_ms(2000);  //���ŷ��ͻ���
						  memset(MessageSend,0x00,sizeof(MessageSend));              //�������������
						  snprintf( MessageSend , sizeof(MessageSend),"\rstart collect time is %d\r",    DeviceConfig.CollectStartTime  );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //����Ҫ���͵Ķ������ô�����ʾ
						  Delay_ms(2000);  //���ŷ��ͻ���
						  AlarmSMS_Send(MessageSend);                     //���ŷ���������Ϣ
          }
				  else           //�ԷǷ�����ֵ������ʾ����ֵ�����Ƿ������Զ�ȡ����ֵ��
				  {
             printf("\r\nInput Start Collect Time not correct!\r\nPlease check and retry.\r\n");    //����ʹ��
						 Delay_ms(2000);  //���ŷ��ͻ���
						 Sms_Send("\rInput Start Collect Time not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //��λ����
				  pSmsRecevBuff = NULL;    //��λ����
       }			 
                    //	char      StartCollectTime[30]="casic_start_collect_time_";              //�״βɼ�ʱ������
                   //	char      CollectPeriod[30]   ="casic_collect_period_";                 //�ɼ���������
                   //	char 			SendCount[30]       ="casic_send_count_";                     //�ϴ���������
                  //	char 			CollectNum[30]      ="casic_collect_number_";                     //һ�βɼ���������
                 //	char 			RetryNum[30]        ="casic_retry_number_";                       //�����ϴ���������
			 
//(10)���������ϴ�����        
   			 pSmsRecevBuff = Find_SpecialString(pBuff, RetryNum,strlen(pBuff),strlen(RetryNum));   //�����ش�����
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(RetryNum)-1))))
			 {
          pSmsRecevData =pSmsRecevBuff +strlen(RetryNum);   
				  for(i=0;i<2;i++)           
				  {
//             if(pSmsRecevData[i]==0x0D)
//						 {
//							  break;
//             }
						 if(((pSmsRecevData[i]>='0')&&(pSmsRecevData[i]<='9')))
						 {;}
						 else
						 {
							  break;
             }
          }
					if(i==0)
					{
             printf("\r\nMessage set input ERROR!!\r\n");     //����ʹ��
          }
				  else 
					{
						 if(i>2)
						 {
               i=2;  //i��ʾ��Ч�����Ƴ��ȣ���ֹ���
             }
					
          
             memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);		
						 
						 DeviceConfig.RetryNum    = char_to_int(ReceiveTemp);
						 
						 printf("\r\n-----data before write in flash!----%s----",(char*)ReceiveTemp);                   //��д��FLASH֮ǰ���������
						 printf("\r\n-----data before write in flash!----%d----", DeviceConfig.RetryNum);                   //��д��FLASH֮ǰ���������
				     
						 printf("\r\n-----data before write in flash!----%x----", DeviceConfig.RetryNum); 
						 UploadFlag = UploadFlash((char*)&DeviceConfig.RetryNum    , 10);
}		
				  if(UploadFlag ==1)
					{
              printf("\r\nStart RetryNum upload success!!\r\n");    //����ʹ��
						  Delay_ms(2000);  //���ŷ��ͻ���
						  Sms_Send("\rStart RetryNum upload success!!\r");
						  Delay_ms(2000);  //���ŷ��ͻ���
						  memset(MessageSend,0x00,sizeof(MessageSend));              //�������������
						  snprintf( MessageSend , sizeof(MessageSend),"\r Retry Number is %d \r",    DeviceConfig.RetryNum   );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //����Ҫ���͵Ķ������ô�����ʾ
						  Delay_ms(2000);  //���ŷ��ͻ���
  						AlarmSMS_Send(MessageSend);                     //���ŷ���������Ϣ
          }
				  else           //�ԷǷ�����ֵ������ʾ����ֵ�����Ƿ������Զ�ȡ����ֵ��
				  {
             printf("\r\nInput RetryNum not correct!\r\nPlease check and retry.\r\n");    //����ʹ��
						 Delay_ms(2000);  //���ŷ��ͻ���
						 Sms_Send("\rInput RetryNum not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //��λ����
				  pSmsRecevBuff = NULL;    //��λ����
       } 			 
 
				 //	char      StartCollectTime[30]="casic_start_collect_time_";              //�״βɼ�ʱ������
//	char      CollectPeriod[30]   ="casic_collect_period_";                 //�ɼ���������
//	char 			SendCount[30]       ="casic_send_count_";                     //�ϴ���������
//	char 			CollectNum[30]      ="casic_collect_number_";                     //һ�βɼ���������
//	char 			RetryNum[30]        ="casic_retry_number_";                       //�����ϴ���������

			 
//(11)���ø�Ũ�ȱ�����ֵ       
   			pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, HighAlarmThresholdSet,sizeof(Usart3_recev_buff),strlen(HighAlarmThresholdSet));   //���ñ�����ֵ
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(HighAlarmThresholdSet)-3))))          //��Ũ����ֵ���ݣ�XX.XX
			 {
          pSmsRecevData =pSmsRecevBuff +strlen(HighAlarmThresholdSet);   
				  for(i=0;i<5;i++)           
				  {

						 if(((pSmsRecevData[i]>='0')&&(pSmsRecevData[i]<='9'))||(pSmsRecevData[i]=='.'))
						 {;}
						 else
						 {
							  break;
             }
          }
					if(i==0)
					{
             printf("\r\nMessage set input ERROR!!\r\n");     //����ʹ��
          }
				  else 
					{
						 if(i>5)
						 {
               i=5;  //i��ʾ��Ч������ֵ���ȣ����Ƴ��ȣ���ֹ���
             }
						 memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);	
						
             DeviceConfig.HighAlarmLevel .Data_Float = char_to_float(ReceiveTemp); 
           					
						printf("\r\n-----data before write in flash!----%f----",DeviceConfig.HighAlarmLevel  .Data_Float);                   //��д��FLASH֮ǰ���������
 				    UploadFlag = UploadFlash((char*)DeviceConfig.HighAlarmLevel.Data_Hex ,11);
          }		
				  if(UploadFlag ==1)
					{
              printf("\r\nHigh Alarm threshold upload success!!\r\n");    //����ʹ��
						  Delay_ms(2000);  //���ŷ��ͻ���
						  Sms_Send("\rHigh Alarm threshold upload success!!\r");
						  Delay_ms(2000);  //���ŷ��ͻ���
						
						  memset(MessageSend,0x00,sizeof(MessageSend));              //�������������
						  snprintf( MessageSend , sizeof(MessageSend),"\rhigh alarm level is %0.2f\r",    DeviceConfig.HighAlarmLevel .Data_Float );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //����Ҫ���͵Ķ������ô�����ʾ
						  Delay_ms(2000);  //���ŷ��ͻ���
						  AlarmSMS_Send(MessageSend);                     //���ŷ���������Ϣ
          }
				  else           //�ԷǷ�����ֵ������ʾ����ֵ�����Ƿ������Զ�ȡ����ֵ��
				  {
             printf("\r\nInput alarm threshold not correct!\r\nPlease check and retry.\r\n");    //����ʹ��
						 Delay_ms(2000);  //���ŷ��ͻ���
						 Sms_Send("\rInput alarm threshold not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //��λ����
				  pSmsRecevBuff = NULL;    //��λ����
       }
				 //	char      StartCollectTime[30]="casic_start_collect_time_";              //�״βɼ�ʱ������
//	char      CollectPeriod[30]   ="casic_collect_period_";                 //�ɼ���������
//	char 			SendCount[30]       ="casic_send_count_";                     //�ϴ���������
//	char 			CollectNum[30]      ="casic_collect_number_";                     //һ�βɼ���������
//	char 			RetryNum[30]        ="casic_retry_number_";                       //�����ϴ���������

			 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*****************************************��ѯ����****************************************************************/		 
//			 pSmsRecevBuff = Find_String(pBuff,"casic_current_data");    //��ѯҺλ��Ϣ
			 pSmsRecevBuff = Find_SpecialString(pBuff, SensorDataInquire,sizeof(Usart3_recev_buff),strlen(SensorDataInquire));   //��ѯ�������ɼ�����
			 if(pSmsRecevBuff != NULL)           //�в�ѯҺλ���ݲ�ѯ����
			 {
				 //DataMessageSend();
				  if(DataCollectCount==0)          //���ݲɼ������ֱ��ͨ�����ŷ��͵�ǰҺλ��Ϣ
					{
             DataMessageSend();
          }
					else
					{
             ConfigData.SensorDataInquireFlag  =1;  //Һλ����δ���вɼ�ʱ���յ���ѯ�������־������1����Һλ���ݲɼ����ʱͨ�����ŷ��͵�ǰҺλ��Ϣ
          }
				  pSmsRecevBuff = NULL;            //��λ����
       }	 
			 
			 
      	pSmsRecevBuff = Find_SpecialString(pBuff, SensorSetInquire,sizeof(Usart3_recev_buff),strlen(SensorSetInquire));   //��ѯ�豸������Ϣ
			 if(pSmsRecevBuff != NULL)           //
			 {
				  Delay_ms(2000);  //���ŷ��ͻ���
				  SensorSetMessage();
				  Delay_ms(2000);  //���ŷ��ͻ���
				  SensorSetMessage();
				 
        //ConfigData.DeviceSetInquireFlag =1;  //Һλ����δ���вɼ�ʱ���յ���ѯ�������־������1����Һλ���ݲɼ����ʱͨ�����ŷ��͵�ǰҺλ��Ϣ
          
				  pSmsRecevBuff = NULL;            //��λ����
       }	


}
  }
}

/*******************************************************************************
* Function Name  : ��������Ϣ���ö��ŵķ�ʽ�������ֻ�
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SensorSetMessage(void)
{
	char  SensorSet[200]={0x00};
	uint8_t counter;
	uint8_t i;
	char  temp[20]={0x00};
	
	      // strcpy(SensorSet,"\r PhoneNum:"); 
        //counter=strlen(SensorSet)-1;  
	 
	       strcat(SensorSet,"\rPhone Num:");                                    //ALARM PHONE NUM 
	       snprintf( temp , sizeof(temp),"%s,",    DeviceConfig.AlarmPhoneNum      );       
				 strcat(SensorSet,temp);
		  	 
				 strcat(SensorSet,"IP:");                                                      //IP 
	       snprintf( temp , sizeof(temp),"%s,",    DeviceConfig.ServerIP      );       
				 strcat(SensorSet,temp);
				 
	       strcat(SensorSet,"PORT:");                                    //PORT
	       snprintf( temp , sizeof(temp),"%s,",    DeviceConfig.ServerPort      );       
				 strcat(SensorSet,temp);
				 
        
  
				 strcat(SensorSet,"LOWLEVEL:");                                    //LowAlarmLevel 
	       snprintf( temp , sizeof(temp),"%0.2f,",    DeviceConfig.LowAlarmLevel.Data_Float     );       
				 strcat(SensorSet,temp);
				 
				  strcat(SensorSet,"HIGHLEVEL:");                                    //HighAlarmLevel 
	       snprintf( temp , sizeof(temp),"%0.2f,",    DeviceConfig.HighAlarmLevel.Data_Float     );       
				 strcat(SensorSet,temp);
				 
				 strcat(SensorSet,"StartTime:");                                    //Start time
	       snprintf( temp , sizeof(temp),"%d,",    DeviceConfig.CollectStartTime     );       
				 strcat(SensorSet,temp);
				 
				 strcat(SensorSet,"CollectPeriod:");                                    //CollectPeriod
	       snprintf( temp , sizeof(temp),"%d,",    DeviceConfig.CollectPeriod      );       
				 strcat(SensorSet,temp);
				 
				 strcat(SensorSet,"SendCount:");                                    //SendCount
	       snprintf( temp , sizeof(temp),"%d,",    DeviceConfig.SendCount      );       
				 strcat(SensorSet,temp);
				 
				 
				  strcat(SensorSet,"RETRY NUM:");                                    //RETRY NUM
	       snprintf( temp , sizeof(temp),"%d,",    DeviceConfig.RetryNum     );       
				 strcat(SensorSet,temp);
				 
				 strcat(SensorSet,"BATTER:");                                    //BATTER Capacity
	       snprintf( temp , sizeof(temp),"%d,\r",    DeviceConfig.BatteryCapacity      );       
				 strcat(SensorSet,temp);
				 
				 
				 
				 Delay_ms(2000);  //���ŷ��ͻ���
				 printf("\r\n the device set parametor is %s,\r\n",SensorSet);
	        //Sms_Send(SensorSet);
				Delay_ms(2000);  //���ŷ��ͻ���
				AlarmSMS_Send(SensorSet);	
}






/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
//void Receive_Analysis_GPRS(void)	
//{
//   char* pRecevBuff =NULL;
//   char  updata_set[7] = {0x50,0x00,0x14,0x01,0x00,0x00,0x34};            //������������������
//   char  complete_set[9] = {0x50,0x00,0x09,0x02,0x00,0x00,0x34,0x00,0x03};//�����������������
//   char  DataRecv_State[8] = {0x50,0x00,0x09,0x02,0x00,0x00,0x34,0x22};   //����������Һλ���ݽ���״̬����
//   
//   int TimeSet[7] ={0};                                                   //���ڼ���������·���ʱ�����õĺϷ���	
//   unsigned char   ReceiveState[4] ={0x00};                               //��¼���������������ݽ���״̬
//   unsigned char   ReceiveFlag =0xFF;                                     //��¼�����������ݽ���״̬
//   uint16_t PeriodSet =0;                                                 //���ڼ���������·������ݲɼ��������õĺϷ���	
//   uint16_t CountSet =0;                                                  //���ڼ���������·��������ϴ��������õĺϷ���	
//   u8     Resp_Count=3;                                                   //Response֡���ͳ��Դ���
//   int i=0;     
//   u8 OpCode=0; 

///////////////////////////////////////��֤�����յ���������������Ӧ��////////////////////////////////
//   pRecevBuff = Find_SpecialString(Usart3_recev_buff, updata_set, sizeof(Usart3_recev_buff), sizeof(updata_set)); //����������ø�����������
//	 if((pRecevBuff != NULL)&&(pRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-19))))        //��ָֹ��Խ��                               
//	 {    
//		 #if DEBUG_TEST
//     printf("\r\nTime correct fream receive ok!!\r\n");     //����ʹ��
//		 #endif
//		 SetRev_OK =1;                                          //���ý��ճɹ�����־������λ
//		 PeriodSet =(pRecevBuff[8]*256) +pRecevBuff[9];
//		 CountSet =(pRecevBuff[10]*256) +pRecevBuff[11];
//		 for(i=0;i<7;i++)
//		 {
//        TimeSet[i] =pRecevBuff[i+12];
//     }
//		 OpCode =pRecevBuff[7];
////****************************�������úϷ�����֤*************************//	
//		 if(OpCode == 0xC1)        //ֻ�е�������0xC1ʱ�Ÿ�������
//		 {
//			 if((PeriodSet>0)&&(CountSet>0))
//			 {
////				 if(CountSet>1440)    //���������·����ô���1440ʱ��˵��Ҫ����������������״̬�����а���Ҫ�رմ˹��ܣ��ԷǷ����ݽ������Σ������µ�ǰ����
////				 { 
//////					 Alive =1;
//////					 DeviceConfig.SendCount =0x0018;        //
//////		       DeviceConfig.CollectPeriod =0x0001;    //
////					 #if DEBUG_TEST
////					 printf("\r\nServer config ERROR!!\r\n");      //����ʹ��
////					 #endif
////				 }
//				 if((PeriodSet*CountSet)<=1440)
//				 {	 
//					 if(Alive ==1)
//					 {
//						 Alive =0;     //�ָ���������ģʽ
//					 }
//					 LSLIQUSET_Handle(pRecevBuff, &DeviceConfig);	
//				 }
//				 else
//				 {
//					 #if DEBUG_TEST
//					 printf("\r\nConfig not correct!!\r\n");      //����ʹ��
//					 #endif
//				 }
//			 }
//			 else
//			 {
//				   #if DEBUG_TEST
//					 printf("\r\nConfig not correct!!\r\n");      //����ʹ��
//				   #endif
//			 }
//		 }
////****************************ʱ�����úϷ�����֤*************************//
//		 if((OpCode == 0xC1)||(OpCode == 0xC2))        //����Ϊ0xC1��0xC2ʱ��������ʱ������
//		 {
//				  if((TimeSet[0]>=0)&&(TimeSet[0]<=60)&&(TimeSet[1]>=0)&&(TimeSet[1]<=60)&&(TimeSet[2]>=0)&&(TimeSet[2]<=23)		 
//						&&(TimeSet[4]>=1)&&(TimeSet[4]<=31)&&(TimeSet[5]>=1)&&(TimeSet[5]<=12)&&(TimeSet[6]>=0)&&(TimeSet[5]<=99))   //�����·��д�����
//				  {
//					  LSTIMESET_Handle(pRecevBuff, &DeviceConfig);
//					  Time_Auto_Regulate(&DeviceConfig);                     //ͨ������������RTCʱ��У׼��Ĭ�Ϸ��������͵�����������Ҫ����Уʱ����
//						while(Resp_Count!=0)
//						{
//							if(RespRev_OK ==0) //�Ƿ��յ��������·������ûỰ����֡��־����
//							{
//								LSLIQUSET_Response(&DeviceConfig);
//								Delay_ms(4000);	
//								DMA_UART3_RecevDetect( DATARECEV ); 
//							}
//							else
//							{
//									RespRev_OK=0;
//									break;
//							}
//							Resp_Count--;
//						}
//				 }	
//				 else
//				 {
//					 #if DEBUG_TEST	 
//					 printf("\r\nTime not correct!!\r\n");                 //����ʹ��
//					 #endif
//				 }	  
//				
//		 }
//	   pRecevBuff = NULL;
//	 }	
///////////////////////////////////////��֤�����յ��������Ự��������////////////////////////////////	 
//	 pRecevBuff = Find_SpecialString(Usart3_recev_buff, complete_set, sizeof(Usart3_recev_buff), sizeof(complete_set)); 
//	 if((pRecevBuff != NULL) &&(pRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-12))))   //��ָֹ��Խ��    
//	 {
//				Section_Handle();
//				RespRev_OK =1;    
//		 
//		    #if DEBUG_TEST	 
//				printf("\r\nSECTION SEND TEST:");                      //����ʹ��
//				for(i=0;i<12;i++)
//				{
//					printf("-%x-",pRecevBuff[i]);                        //����ʹ��
//				}                                                      //����ʹ��
//				printf("\r\n");                                        //����ʹ��
//				#endif
//				pRecevBuff =NULL;
//	 }	
///////////////////////////////////////��֤�����յ��������·���Һλ���ݽ���״̬//////////////////////	 
//	 pRecevBuff = Find_SpecialString(Usart3_recev_buff, DataRecv_State, sizeof(Usart3_recev_buff), sizeof(DataRecv_State)); 
//	 if((pRecevBuff != NULL) &&(pRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-12))))   //��ָֹ��Խ��   
//	 {
//				for(i=0;i<4;i++)
//		    {
//					ReceiveState[i] =pRecevBuff[8+i];
//					ReceiveFlag =ReceiveFlag & ReceiveState[i];
//        }
//		    if(ReceiveFlag == 0xFF)
//				{
//					#if DEBUG_TEST	 
//				  printf("\r\nҺλ���ݽ��ճɹ�!!\r\n");                 //����ʹ��
//					#endif
//					DatRev_OK =1;                                         //������ȫ��ȷ���ձ�־����       
//					for(i=0;i<4;i++)
//		      {
//						ReceiveState[i] =0x00;                              //����״̬��λ
//          }
//				}
//				else
//				{
//             ;//�����ж�ʧ����Ҫ�ش��������д�����
//        }

//				pRecevBuff =NULL;
//	 }	
//	      
//}






/*******************************************************************************
* Function Name  : GPRS_Receive_NetLogin
* Description    : ����GPRS����ע�������
* Input          : None
* Output         : None
* Return         : ����ע��״̬
*******************************************************************************/
unsigned char GPRS_Receive_NetLogin(void)
{
	 char*            pRecevBuff =NULL;
	
	 #if DEBUG_TEST
	 printf("\r\nGPRS Net Login Receive Analysis ...\r\n");          //����ʹ��
	 #endif

	  ////////////////////////////////////////////////////////////////////////////////////////////////////////
    pRecevBuff = Find_SpecialString(Usart3_recev_buff, "+CGREG: 1,1", sizeof(Usart3_recev_buff), 11);  //���ģ������״̬
		if(pRecevBuff!=NULL)                                                
		{   
#if DEBUG_TEST
			 printf("\r\nNet Register OK!!\r\n"); 
#endif			
			 CSQ_OK =1;              //��������Ӧʱ����־������1			 
		   pRecevBuff =NULL;
			 return  1;
		}	 
		//////////////////////////////////////////////////////////////////////////////////////////////////////
    pRecevBuff = Find_SpecialString(Usart3_recev_buff, "+CGREG: 1,5", sizeof(Usart3_recev_buff), 11);  //���ģ������״̬
		if(pRecevBuff!=NULL)                                                
		{   
#if DEBUG_TEST
			 printf("\r\nNet Register OK!!Roaming on!!\r\n");   
#endif
			 CSQ_OK =1;              //��������Ӧʱ����־������1			 
		   pRecevBuff =NULL;
			 return  1;
		}	
    return  0;
}
/*******************************************************************************
* Function Name  : GPRS_Receive_TcpConnect
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
unsigned char GPRS_Receive_TcpConnect(void)
{
	 char*  pRecevBuff =NULL;

   #if DEBUG_TEST
   printf("\r\nTCP Connect is in process...\r\n");          //����ʹ��
	 #endif

		pRecevBuff = Find_SpecialString(Usart3_recev_buff, "CONNECT", sizeof(Usart3_recev_buff), 7);  //���ģ������״̬
		//pRecevBuff = Find_SpecialString(Usart3_recev_buff, "CONNECT", sizeof(Usart3_recev_buff), 10);  //���ģ������״̬
	  if(pRecevBuff != NULL)                                       //�������ӳ��ֹ���ʱ����3Gģ�鸴λ
		{
			 return 1;
		}
		return 0;
}

/*******************************************************************************
* Function Name  : GPRS_Receive_DataAnalysis
* Description    : 3Gģ��������ݵĵ�һ�����
* Input          : None
* Output         : None      
* Return         : None
*******************************************************************************/
unsigned char GPRS_Receive_DataAnalysis(u8* pDeviceID, u16 sNodeAddress)	
{
	 u16     Recev_Flag2 =0;                   //����������ȷ�Ա�־����
	 char*   pRecevBuff =NULL; 
   char    NewMessageIndicate[7] ="+CMGL:";  //�յ�δ������ָʾ

   char   RecevFromCollector[2]={0xA3,0x20};       //���շ��������͵����ݱ�־����
   u8     PayloadLen =0;
   u16    CrcVerify =0x0000;
   u16    CrcRecev  =0x0000;
   int k;

	 #if DEBUG_TEST
   printf("\r\n�������ݽ���!!\r\n");          //����ʹ��
	 #endif

//////////////////////////////////////////////////////////////////////////////////
	 pRecevBuff = Find_SpecialString(Usart3_recev_buff, NewMessageIndicate,sizeof(Usart3_recev_buff),strlen(NewMessageIndicate));    //����Ƿ��յ�����
   {
		 
		
		 
			if(pRecevBuff !=NULL)
			{ 
#if DEBUG_TEST
				 printf("\r\n���ն������ݽ���!!\r\n");          //����ʹ�� 
#endif				
         Sms_Analysis(pRecevBuff);          //���ն��Ž���
				 pRecevBuff =NULL;                  //��λ��ѯָ��
				 Delay_ms(3000);                    //�ȴ�ʱ�䲻��̫�̣������޷��ɹ���ն��ż�¼   
				 mput("AT+CMGD=1,3");               //ɾ��ȫ���ѷ���δ�����Ѷ�����
				 Delay_ms(500);           

				 return 1;
		  }  
   }

	 	 
///////////////////////////////////////////////////////////////////////////////////////////////////
   pRecevBuff = Find_SpecialString(Usart3_recev_buff,RecevFromCollector,sizeof(Usart3_recev_buff),sizeof(RecevFromCollector));  //��������յ���վ�ظ�
	 if((pRecevBuff != NULL)&&(pRecevBuff< Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1)))  //��ָֹ��Խ��          
	 {	
#if DEBUG_TEST
      printf("\r\n������λ�����ݽ���!!\r\n");          //����ʹ�� 		
#endif		 
	                                                          			
				 if(((u8)pRecevBuff[0]==0xA3)&&(pRecevBuff[1]==0x20))//�ڶ���У��
					{
					Treaty_Data_Analysis(((u8*)pRecevBuff), &Recev_Flag2, pDeviceID, sNodeAddress);  //������������    					
           }
							else
							{
									#if DEBUG_TEST	 
									printf("\r\nReceive  data not correct!!\r\n");      //����ʹ��
									#endif
								 Delay_ms(500);	
	                pRecevBuff =NULL;
	                 return 0;
              }
          }	
				
	return Recev_Flag2;
   
	 #if DEBUG_TEST	 
	 printf("\r\n�������ݽ������!!\r\n");                 //����ʹ��
   #endif
}




/*******************************************************************************
* Function Name  : XX
* Description    : ��ʼ��һЩ���ò���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ConfigData_Init(struct Sensor_Set* Para)
{
//	uint8_t  HexArry[7]   ={0x00}; 
    char     CharArry[16] ={0x00};
//  char*    pCharArry    =NULL;
  uint8_t  length =0;
  uint16_t Temp=0;
	float    fTemp=0;
//	uint8_t  Flag =0;   //�Ϸ����ж�
//	uint8_t  FlashStateIndicate[9] = {0x00};            //"FLASH_OK";
//  uint8_t  FlashStateFlag =0;     //����ָʾFlash״̬�Ƿ�������Ϊ0ʱ��ʾ������Ϊ1ʱ��ʾ��������
	int i;
		 #if DEBUG_TEST	
  printf("\r\nConfigData_Init start...\r\n");  //����ʹ��
     #endif
  BKP_TamperPinCmd(DISABLE);                   //

/******************************************************************************************/
//(2)�ɼ�����
	DataRead_From_Flash(0,2,0, ConfigData.CollectPeriod_Byte,2); //��Flash�ж�ȡҺλ�Ʋɼ����
  Temp =ConfigData.CollectPeriod_Byte[0]*256 + ConfigData.CollectPeriod_Byte[1];
  //printf("------------%d------------",Temp);
  if((Temp>0)&&(Temp<=60))                    //������1~60���ӽ������ݲɼ����
	{
		Para->CollectPeriod =  Temp;
	}
	else
	{
    Para->CollectPeriod =60;           //ÿ��15���Ӳɼ�һ�����ݡ�
  }
		printf("\r\n**************�ɼ�����-->> %d ***********\r\n",Para->CollectPeriod);       //�������
	
//(3)�ϱ�����	
  DataRead_From_Flash(0,3,0, ConfigData.SendCount_Byte ,2);             //��Flash�ж�ȡ�ϴ�����
  Temp =ConfigData.SendCount_Byte[0]*256 + ConfigData.SendCount_Byte[1];
	
	//printf("------------%d------------",Temp);
  if((Temp>0)&&(Temp<=1440))  
	{
		Para->SendCount =  Temp;
	}
	else
	{    
    Para->SendCount =60;              //1Сʱ����һ������
  }	
	
  printf("\r\n**************�ϱ�����-->> %d *********** \r\n",Para->SendCount );   //�������
//(4)�����ֻ���
   DataRead_From_Flash(0,4,0, (u8*)ConfigData.AlarmPhoneNum,15);     //��Flash�ж�ȡԤ�豨���ֻ�����

	for(Temp=0;Temp<15;)
  {
    if((ConfigData.AlarmPhoneNum[Temp] >='0')&&(ConfigData.AlarmPhoneNum[Temp] <='9'))
		{
       Temp++;
    }
		else
		{
			break;
    }
  }
	length =Temp;
	if(((length==11)||(length==13)||(length==15)) && (ConfigData.AlarmPhoneNum[0] == '8') && (ConfigData.AlarmPhoneNum[1] == '6'))          //�������������86ǰ׺���ܳɹ������ţ�������󳤶�Ϊ15
	{
     memcpy(Para->AlarmPhoneNum, ConfigData.AlarmPhoneNum, length);        
  }
	else                                                     
	{
		 memcpy(Para->AlarmPhoneNum,"861064617178004",15);   //��ȡ������Чʱ����ʼ�������绰��������Ҫ���ݿ�������Ӧ���� �����ֻ�����
  }
		printf("\r\n**************�����绰-->> %s *********** \r\n", Para->AlarmPhoneNum );   //�������

	//(5)�������IP��	
	DataRead_From_Flash(0,5,0, (u8*)ConfigData.ServerIP,15);   //��Flash�ж�ȡԤ�������IP
	length = strlen(ConfigData.ServerIP );
	for(Temp=0;Temp<15;)
  {
    if(((ConfigData.ServerIP[Temp] >='0')&&(ConfigData.ServerIP[Temp] <='9'))||(ConfigData.ServerIP[Temp] =='.'))
		{
       Temp++;
    }
		else
		{
			break;
    }
  }
	length =Temp;

	if(9<length<15)                           //��IP��ַ�Ϸ���������ɸѡ�������д����� 
	{
     memcpy(Para->ServerIP, ConfigData.ServerIP, length);        
  }
	else                                                     
	{
		 memcpy(Para->ServerIP,"124.42.118.86",14);                  //��ȡ������Чʱ����ʼ��������IP   
  }
	
		printf("\r\n**************����IP-->> %s *********** \r\n", Para->ServerIP );   //�������
//(6) �������˿ں�	
	DataRead_From_Flash(0,6,0, (u8*)ConfigData.ServerPort,5);   //��Flash�ж�ȡԤ��������˿ں�
//	length = strlen(ConfigData.SMS_Set_ServerPort);
	for(Temp=0;Temp<5;)
  {
    if((ConfigData.ServerPort[Temp] >='0')&&(ConfigData.ServerPort[Temp] <='9'))
		{
       Temp++;
    }
		else
		{
			break;
    }
  }
	length =Temp;
	if(length>0)                                              //�Է������˿ںźϷ���������ɸѡ�������д����� 
	{
     memcpy(Para->ServerPort, ConfigData.ServerPort, length);        
  }
	else                                                     
	{ 
		 memcpy(Para->ServerPort,"2017",4);                          //��ȡ������Чʱ����ʼ���������˿ں� 
  }
	
		printf("\r\n**************����˿ں�-->> %s *********** \r\n", Para->ServerPort );   //�������


//(7)��Ũ�ȱ�����ֵ
	DataRead_From_Flash(0,7,0, ConfigData.LowAlarmLevel .Data_Hex ,4);   //��Flash�ж�ȡԤ�豨����ֵ

	fTemp = ConfigData.LowAlarmLevel .Data_Float ;
	
  //printf("\r\n---read low level ---%f----\r\n",fTemp );                       //��ӡFLASH��ȡ��ԭʼ����
   	
                               

		 if( (0.0<fTemp ) && ( fTemp <25.0) )
     Para->LowAlarmLevel  .Data_Float  =ConfigData.LowAlarmLevel .Data_Float  ;      

	else                                                     
	{ 
		 Para->LowAlarmLevel .Data_Float  =25.0;                        //��ȡ������Чʱ����������ֵ��Ϊ25.0����������ֵΪ0ʱ�����ᴥ�������¼�
  }
	
	printf("\r\n**************�ͱ���Ũ��-->> %0.2f ************\r\n", Para->LowAlarmLevel  .Data_Float );   //�������
	
// ��8����ʼ�ɼ�ʱ��
	DataRead_From_Flash(0,8,0,ConfigData.CollectStartTime_Byte ,2); //��Flash�ж�ȡ��һ�βɼ�ʱ��

	//Temp =  char_to_int(CharArry);
	
  Temp =ConfigData.CollectStartTime_Byte [0]*256 + ConfigData.CollectStartTime_Byte [1];
	
	//printf("\r\n---read CollectStartTime ---%d----\r\n",Temp );                       //��ӡFLASH��ȡ��ԭʼ����
  if((Temp>=0)&&(Temp<=1440))  
	{
		Para->CollectStartTime  =  Temp;
	}
	else
	{
    Para->CollectStartTime  = 0;           //��һ�βɼ�ʱ��Ϊ0����
  }

	printf("\r\n**************��ʼ�ɼ�ʱ��-->> %d ***********\r\n", Para->CollectStartTime );   //�������
	
	
//(9)�Ѳɼ�����
	DataRead_From_Flash(0,9,0,    &(ConfigData.CollectNum)   ,1); //��Flash�ж�ȡ��ǰ�ɼ�����
	
//	printf("\r\n---read retry number ---%s----\r\n",(u8*)ConfigData.CollectNum_Byte );                       //��ӡFLASH��ȡ��ԭʼ����
  //Temp =ConfigData.CollectNum_Byte [0]*256 + ConfigData.CollectNum_Byte [1];
	Temp = ConfigData.CollectNum;
	//printf("\r\n---read CollectNum ---%d----\r\n",Temp );                       //��ӡFLASH��ȡ��ԭʼ����
//  if((0 <=ConfigData.CollectNum )&&(ConfigData.CollectNum <= MAX_COLLECTNUM))  
//	{
//		if(ConfigData.CollectNum <=( Para->SendCount /Para->CollectPeriod ))
//		{ Para->CollectNum  =  ConfigData.CollectNum; }
//		else
//		{ Para->CollectNum =( Para->SendCount /Para->CollectPeriod );}
//	}
//	else
//	{
//    Para->CollectNum  = ( Para->SendCount /Para->CollectPeriod ) ;           //��ǰ�ɼ��Ĵ����������� 
//  }
	
		printf("\r\n**************�Ѳɼ�����-->> %d ***********\r\n", Temp );   //�������
//(10)�ش�����
	DataRead_From_Flash(0,10,0, &(ConfigData.RetryNum ) ,1); //��Flash�ж�ȡ�ش�����
  
	//printf("\r\n---read retry number ---%d----\r\n",ConfigData.RetryNum );                       //��ӡFLASH��ȡ��ԭʼ����
	
	Temp =ConfigData.RetryNum ;
  if((Temp>0)&&(Temp<=10))  
	{
		Para->RetryNum  =  Temp;
	}
	else
	{
    Para->RetryNum  =3;           //Ĭ��Ϊ3
  }
	printf("\r\n**************�ش�����-->> %d ***********\r\n", Para->RetryNum );   //�������
	

//(11)��Ũ�ȱ�����ֵ
	DataRead_From_Flash(0,11,0, ConfigData.HighAlarmLevel .Data_Hex  ,4);    //��Flash�ж�ȡ��Ũ�ȱ���
  
	//printf("\r\n---read high alarm level ---%f----\r\n",ConfigData.HighAlarmLevel .Data_Float );                       //��ӡFLASH��ȡ��ԭʼ����
	
	fTemp =ConfigData.HighAlarmLevel .Data_Float  ;
  if((fTemp>0.0) && (fTemp < 50.0) && (fTemp > ConfigData.LowAlarmLevel .Data_Float ))  
	{
		Para->HighAlarmLevel .Data_Float   =  fTemp;
	}
	else
	{
    	Para->HighAlarmLevel .Data_Float  =  50.0;           //������Ũ����ֵĬ��Ϊ50%
  }
	printf("\r\n**************�߱���Ũ��-->> %0.2f ***********\r\n", 	Para->HighAlarmLevel .Data_Float );   //�������
	
//(12)�豸�ѹ�������
	DataRead_From_Flash(0,12,0, (ConfigData.WorkNum_Byte),4);    //��Flash�ж�ȡ�ѹ�������

	Temp=ConfigData.WorkNum_Byte [0]*256+ConfigData.WorkNum_Byte [1];
  Para->WorkNum  = Temp; 
	printf("\r\n**************��������-->> %d ***********\r\n", 	Para->WorkNum);   //�������
//(13)��ص���
	printf("\r\n**************��ص���-->> %d%% ***********\r\n", 	Para->BatteryCapacity=DS2780_Test());   //�������
//	Para->Time_Sec  =0x00;
//	Para->Time_Min  =0x00;
//	Para->Time_Hour =0x00;
//	Para->Time_Mday =0x00;
//	Para->Time_Mon  =0x00;
//	Para->Time_Year =0x00;
//  Para->BatteryCapacity =0x64;    //��ص������ݶ�Ϊ100%
	Para->MessageSetFlag  =0;       
}





 
 
 




/**********************************************END******************************************/










