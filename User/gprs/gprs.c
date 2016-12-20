
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

//char  Receive_Monitor_GPRS(void); //3G模块串口监听，当MCU检测到3G模块串口的数据时
//void  Receive_Deal_GPRS(void);    //3G模块接收数据解析，主要用于处理MCU与3G模块之间的交互数据
//void  Receive_Analysis_GPRS(void);//3G模块接收服务器数据解析，主要处理服务器下发的基于埃德尔协议的数据
//u8    NetStatus_Detection( void );//3G模块网络连接状态检测
//void  SetRequest_GPRS(void);      //通过3G模块请求服务器下发配置信息

char      Usart3_recev_buff[1000]={'\0'};     //USART3接收缓存
uint16_t  Usart3_recev_count=0;              //USART3接收计数器

extern unsigned char Usart3_send_buff[300];
uint8_t    CSQ_OK =0;                         //信号质量标志变量


extern u8         Send_Request_flag;                         //发送数据至上位机后收到反馈
extern char   		SendData_Flag;                       //发送数据成功标志位，发送成为为1，不成功为0；
extern char       TcpConnect_Flag;                     //TCP连接成功标志位，连接成功为1，不成功为0；


extern struct    SMS_Config_RegPara   ConfigData;     //定义的下发配置参数，HEX格式，方便在系统进入休眠模式之前写入BKP寄存器
static char      RespRev_OK =0;              //成功接收服务器应答
  
extern struct    Sensor_Set  DeviceConfig;   //配置信息结构体
extern char      SetRev_OK;                  //成功接收服务器配置
extern char      Alive;                      //不休眠标志变量,为1时说明不进行休眠处理，有待调整
extern char      DatRev_OK ;                 //成功正确接收液位数据
extern uint8_t   DataCollectCount;           //数据采集计数器
extern uint8_t   LiquidDataSend_Flag;

extern uint8_t   DMA_USART3_RecevBuff[RECEIVEBUFF_SIZE];
extern uint8_t   DMA_UART3_RECEV_FLAG ;      //USART3 DMA接收标志变量

extern unsigned char  DMA_UART3_RecevDetect(unsigned char RecevFlag,u8* pDeviceID, u16 sNodeAddress);     //USART3接收数据监测与数据解析
extern void           DataMessageSend(void);      // 通过短信发送液位数据

//void  SIM5216_PowerOn(void)            //打开SIM5216模块
//{
//   GPIO_SetBits(GPIOC,GPIO_Pin_3);	   //POWER_ON引脚拉低
//   Delay_ms(120);                      //100ms延时     64ms<Ton<180ms
//   GPIO_ResetBits(GPIOC,GPIO_Pin_3);   //POWER_ON引脚拉高
//   Delay_ms(5500);                     //5s延时        Tuart>4.7s           
//	
//}

//void  SIM5216_PowerOff(void)            //关闭SIM5216模块
//{
//   GPIO_SetBits(GPIOC,GPIO_Pin_3);	    //POWER_ON引脚拉低
//   Delay_ms(2000);                      //1s延时       500ms<Ton<5s
//   GPIO_ResetBits(GPIOC,GPIO_Pin_3);    //POWER_ON引脚拉高
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
void USART_DataBlock_Send(USART_TypeDef* USART_PORT,char* SendUartBuf,u16 SendLength)    //批量向串口发送数据
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
	
	
	printf("length:%d\r\n",length);             //测试使用
  //USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);   //向USART3发送数据前，先打开USART3接收空闲中断，便于监测数据接收完成
	USART_DataBlock_Send(USART3,str,length);
	//USART_DataBlock_Send(USART3,"\r\n",2);
	USART_DataBlock_Send(USART1,str,length);
	USART_DataBlock_Send(USART1,"\r\n",2);
}

void mput(char* str)
{
//	printf("length:%d\r\n",strlen(str));     //测试使用
//	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);   //向USART3发送数据前，先打开USART3接收空闲中断，便于监测数据接收完成
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
	
	 printf("\r\nFUNCTION :GPRS_Config  start! \r\n");                                 //提示进入该函数
	 memset(DMA_USART3_RecevBuff,0x00,RECEIVEBUFF_SIZE);
   USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);  //配置串口DMA1接收
   mput("AT^UARTRPT=1");  //设置通信接口为UART
	 Delay_ms(200);      
	 mput("AT+CMGF=1");     //短信格式设置为文本模式
	 Delay_ms(300); 
	 mput("AT+CPMS=\"ME\",\"ME\",\"ME\"");     //设置发送和接收短信存储器为ME消息存储器，否则使用AT+CMGL="REC UNREAD"命令查不到短信
	 Delay_ms(300); 
   mput("AT+CNMI=1,1");   //新消息指示设置为存储并送通知
	 Delay_ms(200);
	
//	 mput("AT+CMGD=1,3");   //新消息指示设置为存储并送通知
//	 Delay_ms(500);
//	 mput("AT+CPMS?");   //新消息指示设置为存储并送通知
//	 Delay_ms(200);
	
	 printf("\r\nFUNCTION :GPRS_Config  end! \r\n");                                 //提示结束该函数
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
  u8   NetSearchCount=40;           //搜寻网络最大尝试次数
	u8   CSQ_DetectCount=2;           //网络注册成功以后再次搜索网络次数，用以稳定网络连接
//	char SendArry1[80] ={'\0'};
//  u8   i=0;
	
	printf("\r\nFUNCTION :GPRS_Init  start! \r\n");                                    //提示进入该函数
  memset(DMA_USART3_RecevBuff,0x00,RECEIVEBUFF_SIZE);
  USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);  //配置串口DMA1接收
//  SIM5216_PowerOn();
	Delay_ms(1000);
	mput("AT^MODECONFIG=19");   //
	while(NetSearchCount!=0)
	{
	   NetSearchCount--;
		 printf("\r\nGPRS Net Searching...\r\n");    //测试使用
		 mput("AT+CGREG=1");   //完成信号自动收索，当3G信号很弱时自动切换到3G信号
		 Delay_ms(800);
		 mput("AT+CGREG?");   //完成信号自动收索，当3G信号很弱时自动切换到3G信号
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
//     DS2780_Test();   //测试使用		 
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
	  unsigned char  TcpStatusFlag =0;  //TCP连接状态标志变量
	  int i;
	  for(i=0;i<4;i++)
	{
	  //mput("AT+MIPOPEN?"); //查询Socket建立情况
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
    unsigned char  TcpConnectFlag =0;  //TCP连接建立标志变量
    unsigned char  NetErrorCount =10;   //TCP建立连接尝试次数，下一版短信可配置  
#if DEBUG_TEST
	  printf(" Multi GPRS Start!\r\n");
    printf(" TCP Start Connect!\r\n");              //测试显示
#endif
    GPRS_Init();

		while(NetErrorCount>0)
		{
				 
			 mput("at+mipcall=1");
			 Delay_ms(800);		
			 snprintf(SendArry,sizeof(SendArry),"at+miptrans=1,\"%s\",%s",DeviceConfig.ServerIP,DeviceConfig.ServerPort);  // mput("at+miptrans=1,\"58.210.41.202\",2015");   //后续需要从寄存器中读取，有待进一步完善
//       snprintf(SendArry,sizeof(SendArry),"AT+MIPOPEN=1,\"TCP\",\"%s\",%s,10000",DeviceConfig.ServerIP,DeviceConfig.ServerPort);  // mput("at+miptrans=1,\"58.210.41.202\",2015");   //后续需要从寄存器中读取，有待进一步完善			
       mput(SendArry);
			 Delay_ms(5000); 
			 //mput("AT+MIPOPEN?"); //查询Socket建立情况                   注释9.9
			 Delay_ms(800);	
			 TcpConnectFlag =DMA_UART3_RecevDetect( TCPCONNECT ,0 ,0 );    //对GPRS模块接收到的数据进行解析
			 NetErrorCount--;
			 if(TcpConnectFlag==1) 
			 {
				 
				 printf("\r\nThe TCP connect is OK\r\n");
         //mput("AT+MIPMODE=1,1"); //设置SOCKET模式为Hex传输模式
         Delay_ms(500);	
				 
//			mput("AT+MIPSEND=1,5"); //发送测试
//			mput("11111");
				 break;
       }
		}
		if(NetErrorCount == 0)
		{
			printf("\r\nThe Client Could Not Connect TCP!\r\n");
       TCP_Disconnect();
			 Delay_ms(2000);
			 //TCP_Connect();                              //保证TCP连接成功
			 gotoSleep(DeviceConfig.CollectPeriod );     //多次尝试未连接到网络，直接进入休眠模式，用于监测模块唤醒以后第一次入网状态
    } 
#if DEBUG_TEST
		printf(" TCP end Connect!\r\n");                    //测试显示
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
//	TCP_Connect_Flag =0;    //关闭循环联网
	Delay_ms(500);
}
/*******************************************************************************
* Function Name  : char* Find_String(char* Source, char* Object)
* Description    : 在目标字符串中发现一个指定的字符串
* Input          : 
* Output         : 
* Return         : 如果找到，则返回目标字符串在源字符串中的首地址
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
/***********函数功能：在特定序列中发现一个指定的序列****************/
/***********如果找到，则返回目标序列在源序列中的首地址**************/
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
/********************函数功能：短信发送前预处理*********************/
/*************即在需要发送的数据最后增加一个结束符0x1a**************/
void SendPretreatment(char* pSend)
{  
   uint8_t  i=0;
	 char*    pTemp =NULL;
	
	 i= strlen(pSend);
	 pTemp =pSend+i;
   *(pTemp) =0x1a;
}

/***********************函数功能：发送短信**************************/
/*******************************************************************/
//发送短信流程：
//第一步：设置接收短信提醒格式：AT+CNMI=1,2,0,0,0    
//第二步：设定短信接收方号码：AT+CMGS="15116924685"
//第三步：发送短信正文，并以16进制0x1a作为结尾
void Sms_Send(char*  pSend)
{ 
//	uint8_t  PhoneNum[13]={0x00};    //后续更改为从寄存器中读取
	char  SendBuf[200]={0x00};                //短信发送缓存,根据最大可能的发送数据长度确定其大小

//	struct Sensor_Set* Para= &DeviceConfig;
	
//	memcpy(PhoneNum, DeviceConfig.AlarmPhoneNum,strlen((char*)DeviceConfig.AlarmPhoneNum));          
//	mput("AT+CNMI=1,2,0,0,0");
	
  memset(SendBuf,0,sizeof(SendBuf));
	snprintf(SendBuf,sizeof(SendBuf),"AT+CMGS=\"%s\"",ConfigData.CurrentPhoneNum); //发送短信号码为当前通信手机号码
	mput(SendBuf);
//	mput("AT+CMGS=\"861064617006426\"");  //测试使用，有待完善

	Delay_ms(500);
  memset(SendBuf,0,sizeof(SendBuf));
	snprintf(SendBuf, (sizeof(SendBuf)-1), pSend, strlen(pSend));
  SendPretreatment(SendBuf);
	mput(SendBuf);
  Delay_ms(1000);  
//	DMA_UART3_RecevDetect();  //等待接收信息

}

void AlarmSMS_Send(char*  pSend)  //发送报警短信
{ 
	char  SendBuf[200]={0x00};                //短信发送缓存,根据最大可能的发送数据长度确定其大小

  memset(SendBuf,0,sizeof(SendBuf));
	snprintf(SendBuf,sizeof(SendBuf),"AT+CMGS=\"%s\"",DeviceConfig.AlarmPhoneNum); //发送短信号码为预设报警手机号码
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

/**************************************字符转化为浮点数******************************************/
/**************************************用于计算报警阈值******************************************/
float  char_to_float(char* pSetPara)
{
	char CharTemp[10]={0x00};
	int i,j;
	int IntegerPart=0;
	int DecimalPart=0;
	int Counter=0;
	float number=0.0;
	for(j=0;j<strlen(pSetPara);j++)                       //调用函数已经做了防溢出处理,strlen(pSetPara)<=5
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
				 
         IntegerPart =IntegerPart + (CharTemp[i-1]-'0')*(int)(pow(10,j));   //计算报警阈值整数部分
       
				 
       }
			 
			 Counter =strlen(CharTemp)+1; //跳过小数点
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
           DecimalPart =DecimalPart + (CharTemp[i-1]-'0')*(int)(pow(10,j));  //计算报警阈值小数部分
         }
       }
	
	     number=DecimalPart*0.01 + IntegerPart;
			 return(number);
}


/**************************************字符转化为整数******************************************/
uint16_t char_to_int(char* pSetPara)
{
	char CharTemp[10]={0x00};
	int i,j;
  int length;
	int IntegerPart=0;


	for(j=0;j<strlen(pSetPara);j++)                       //调用函数已经做了防溢出处理,strlen(pSetPara)<=5
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
* Description    : 查阅未读短信
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Sms_Consult(u8* pDeviceID, u16 sNodeAddress)
{
#if DEBUG_TEST
  	printf("\r\nFUNCTION :Sms_Consult  start! \r\n");                                 //提示进入该函数
#endif	
    mput("AT+CMGL=\"REC UNREAD\"");     //读取未读信息
//	  mput("AT+CMGL=\"ALL\"");     //读取未读信息
//	  mput("AT+CMGL=0");     //读取未读信息
		Delay_ms(1500); 
	  DMA_UART3_RecevDetect( DATARECEV, pDeviceID, sNodeAddress );		
//		mput("AT+CMGD=,3");      //删除已读信息
		Delay_ms(2500);            //等待模块切换回数据传输状态
#if DEBUG_TEST		
	printf("\r\nFUNCTION :Sms_Consult  end! \r\n");                                 //提示退出该函数
#endif
}

/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/***********************函数功能：短信配置解析**************************/
/*******************************************************************/
//AT+CNMI=1,2,0,0,0                                //接收短信设置
//+CMT: "+8613121342836","","15/08/18,17:05:29+32" //接收到的短信报文格式，回车换行（0D 0A）之后为消息正文
//ok                                               //消息正文
//AT+CNMI=2,1                                      //接收短信设置
//+CMTI: "ME",1                                    //接收到新短信提醒
//AT+CMGL="REC UNREAD"                             //需发送查询未读短信命令
//+CMGL: 1,"REC UNREAD","+8613121342836","","15/08/18,17:12:19+32"   //接收到的短信报文格式，回车换行（0D 0A）之后为消息正文
//ok                                               //消息正文

void Sms_Analysis(char* pBuff)
{ 

//	char      MessageRead[11] ="REC UNREAD";                            //查询到未读短信存在
	char      MessageRecevIndicate[5] ="+32\"";                         //接收到短信指示性字符串
	////////////////////////////////短信配置///////////////////////////////////////////////////////////////////////
	char      AlarmPhoneSet[25] ="casic_set_alarm_phone_";              //报警号码设置命令
	char      ServerIpSet[25]   ="casic_set_server_ip_";                //服务器IP设置命令
	char      ServerPortSet[25] ="casic_set_server_port_";              //服务器端口号设置命令
	char      StartCollectTime[30] ="casic_start_collect_time_";              //首次采集时间设置
	char      CollectPeriod[30]    ="casic_collect_period_";                 //采集周期设置
	char 			SendCount[30]        ="casic_send_count_";                     //上传周期设置
	char 			RetryNum[30]         ="casic_retry_number_";                       //重新上传次数设置
	char      LowAlarmThresholdSet[35] ="casic_set_low_alarm_threshold_";      //低浓度报警阈值设置命令
	char      HighAlarmThresholdSet[35] ="casic_set_high_alarm_threshold_";      //高浓度报警阈值设置命令
	
  char 			SensorDataInquire[20]=	"casic_current_data";           //短信获取传感器数据
  char      SensorSetInquire[20] =  "casic_set_parameter";          //短信获取传感器配置参数			
//////////////////////////////////配置数组定义完毕////////////////////////////////////////////////////////////////////
	
	
	char      CurrentPhoneNum[16]    ={0x00};                              //存储当前通信手机号码
  
	
	char      ReceiveTemp[16] ={0x00};                     //暂时接收数据组
  char      MessageSend[200] ={0x00};                     //短信发送数据组
	
	
	
	char*     pSmsRecevBuff =NULL;            //查找特征数据位置指针
  char*     pSmsRecevData =NULL;            //查找目标数据位置指针
  uint8_t   UploadFlag =0;                  //用于指示短信设置参数修改结果
	uint8_t   i=0;                            //循环变量
  uint16_t  temp=0;                          //暂时整数保存
	uint8_t   interget=0;
	uint8_t   decimal=0;
  u8   j=0;


	pSmsRecevBuff = Find_String(pBuff,"\"REC UNREAD\""); //检查是否收到短信
	if(pSmsRecevBuff !=NULL)
	{
		if(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-29))) //防止指针越界
		{
      
			 pSmsRecevBuff = pSmsRecevBuff+15;   //指针指向手机号码前缀86		 REC UNREAD 到  +86之间为15位

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
        memcpy(ConfigData.CurrentPhoneNum,CurrentPhoneNum,i); //提取当前通信手机号码，有前缀86
			  printf("\r\nCurrentPhoneNum:%s\r\n",CurrentPhoneNum); //测试使用
    }
	   pSmsRecevBuff =NULL;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		
		  pSmsRecevBuff = Find_String(pBuff,"+32\"");         //检查是否收到短信
//	   pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, MessageRecevIndicate,sizeof(Usart3_recev_buff),strlen(MessageRecevIndicate));    //检查是否收到短信，过滤短信查询命令
		 if((pSmsRecevBuff !=NULL) &&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(MessageRecevIndicate)))))   //防止指针越界
		 {
       pSmsRecevData =pSmsRecevBuff +strlen(MessageRecevIndicate);                          //测试使用
			 pSmsRecevBuff =NULL;   
			 printf("\r\nReceive Short Message is: %s\r\n",pSmsRecevData);    //测试使用
/*****************************************参数设置功能*************************************************************/		 			 
//(2)设置采集周期			 
         //pSmsRecevBuff = Find_String(pBuff,"casic_start_collect_time_");    //设置采集周期
   			 pSmsRecevBuff = Find_SpecialString(pBuff, CollectPeriod,strlen(pBuff),strlen(CollectPeriod));   //设置采集周期
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
             printf("\r\nMessage set input ERROR!!\r\n");     //测试使用
          }
				  else 
					{
						 if(i>4)
						 {
               i=4;  //i表示有效，限制长度，防止溢出
             }
				
             memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);		                //转为10进制						 DeviceConfig.CollectPeriod  = char_to_int(AlarmThresholdTemp);
						 DeviceConfig.CollectPeriod = char_to_int(ReceiveTemp);
						 ConfigData.CollectPeriod_Byte  [0] = DeviceConfig.CollectPeriod >>8; 
						 ConfigData.CollectPeriod_Byte  [1] = DeviceConfig.CollectPeriod &0xff;
						 	printf("\r\n-----data before write in flash!----%s----",(char*)ReceiveTemp);                   //在写入FLASH之前，输出测试
						  printf("\r\n-----data before write in flash!----%d----",DeviceConfig.CollectPeriod);                   //在写入FLASH之前，输出测试
				       for(j=0;j<2;j++)
						 printf("\r\n-----data before write in flash!----%x----",ConfigData.CollectPeriod_Byte [j]);  
						 
						 UploadFlag = UploadFlash((char*)ConfigData.CollectPeriod_Byte  , 2);
}		
				  if(UploadFlag ==1)
					{
              printf("\r\nStart CollectPeriod upload success!!\r\n");    //测试使用
						  Delay_ms(2000);  //短信发送缓冲
						  Sms_Send("\rStart CollectPeriod upload success!!\r");
						  
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //将发送数组清零
						  snprintf( MessageSend , sizeof(MessageSend),"\rcollect period is %d \r",    DeviceConfig.CollectPeriod  );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //将需要发送的短信利用串口显示
						  Delay_ms(2000);  //短信发送缓冲
						  AlarmSMS_Send(MessageSend);                     //短信发送配置信息
          }
				  else           //对非法输入值给出提示（负值不做非法处理，自动取绝对值）
				  {
             printf("\r\nInput CollectPeriod not correct!\r\nPlease check and retry.\r\n");    //测试使用
						 Delay_ms(2000);  //短信发送缓冲
						 Sms_Send("\rInput CollectPeriod not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //复位变量
				  pSmsRecevBuff = NULL;    //复位变量
       }	
			 
			 //	char      StartCollectTime[30]="casic_start_collect_time_";              //首次采集时间设置
//	char      CollectPeriod[30]   ="casic_collect_period_";                 //采集周期设置
//	char 			SendCount[30]       ="casic_send_count_";                     //上传周期设置

//	char 			RetryNum[30]        ="casic_retry_number_";                       //重新上传次数设置

//(3)设置上传周期			 
         //pSmsRecevBuff = Find_String(pBuff,"casic_start_collect_time_");    //设置上传周期
   			 pSmsRecevBuff = Find_SpecialString(pBuff, SendCount,strlen(pBuff),strlen(SendCount));   //设置上传周期
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
             printf("\r\nMessage set input ERROR!!\r\n");     //测试使用
          }
				  else 
					{
						 if(i>4)
						 {
               i=4;  //i表示有效，限制长度，防止溢出
             }
					
          
             memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);		
						 
						 DeviceConfig.SendCount   = char_to_int(ReceiveTemp);
						 ConfigData.SendCount_Byte   [0] = DeviceConfig.SendCount  >> 8; 
						 ConfigData.SendCount_Byte   [1] = DeviceConfig.SendCount  &  0xff;

						 	printf("\r\n-----data before write in flash!----%s----",(char*)ReceiveTemp);                   //在写入FLASH之前，输出测试
						  printf("\r\n-----data before write in flash!----%d----",DeviceConfig.SendCount);                   //在写入FLASH之前，输出测试
						  for(j=0;j<2;j++)
						 printf("\r\n-----data before write in flash!----%x----",ConfigData.SendCount_Byte [j]);  
				     UploadFlag = UploadFlash((char*)ConfigData.SendCount_Byte    , 3);
}		
				  if(UploadFlag ==1)
					{
              printf("\r\nStart SendCount upload success!!\r\n");    //测试使用
						  Delay_ms(2000);  //短信发送缓冲
						  Sms_Send("\rSendCount upload success!!\r");
						  Delay_ms(2000);  //短信发送缓冲
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //将发送数组清零
						  snprintf( MessageSend , sizeof(MessageSend),"\rsend count time is %d \r",    DeviceConfig.SendCount   );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //将需要发送的短信利用串口显示
						  Delay_ms(2000);  //短信发送缓冲
						  AlarmSMS_Send(MessageSend);                     //短信发送配置信息           //
          }
				  else           //对非法输入值给出提示（负值不做非法处理，自动取绝对值）
				  {
             printf("\r\nInput SendCount not correct!\r\nPlease check and retry.\r\n");    //测试使用
						 Delay_ms(2000);  //短信发送缓冲
						 Sms_Send("\rInput SendCount not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //复位变量
				  pSmsRecevBuff = NULL;    //复位变量
       }
			 
//(4)设置报警电话号码
			 // pSmsRecevBuff = Find_String(pBuff,"casic_set_alarm_phone_");  //设置报警号码
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
				 if((i>15)||(i<11))    //输入数据不合法，直接丢弃，不更新配置信息，并给出错误提示
					{
						 printf("\r\nMessage set input ERROR!!\r\n");     //测试使用
//						 i=16;  //i表示有效报警阈值长度，限制长度，防止溢出
          }		
					else
					{
             memset(DeviceConfig.AlarmPhoneNum,0x00,sizeof(DeviceConfig.AlarmPhoneNum));
				     memcpy(DeviceConfig.AlarmPhoneNum, pSmsRecevData, i);
				     UploadFlag = UploadFlash(DeviceConfig.AlarmPhoneNum , 4);
          }
				  if(UploadFlag ==1)
					{
              printf("\r\nAlarm phone number upload success!!\r\n");    //测试使用
							Delay_ms(2000);  //短信发送缓冲
						  Sms_Send("\rAlarm phone number upload success!!\r");
						  Delay_ms(2000);  //短信发送缓冲
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //将发送数组清零
						  snprintf( MessageSend , sizeof(MessageSend),"\rAlarm phone number is %s \r",    DeviceConfig.AlarmPhoneNum );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //将需要发送的短信利用串口显示
						  Delay_ms(2000);  //短信发送缓冲
						  AlarmSMS_Send(MessageSend);                     //短信发送配置信息
          }
					else
					{
              printf("\r\nAlarm phone number not correct!\r\nPlease check and retry.\r\n");    //测试使用
							Delay_ms(2000);  //短信发送缓冲
						  Sms_Send("\rAlarm phone number not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //复位变量
				  pSmsRecevBuff = NULL;    //复位变量
       }
//(5)设置网络服务IP号码			 
			 pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, ServerIpSet,sizeof(Usart3_recev_buff),strlen(ServerIpSet));  //设置服务器IP
			// pSmsRecevBuff = Find_String(pBuff,"casic_set_server_ip_");    //设置服务器IP
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
				  if((i>15)||(i<7))  //输入数据不合法，直接丢弃，不更新配置信息，并给出错误提示
					{
						 printf("\r\nMessage set input ERROR!!\r\n");     //测试使用 	
//					 i=15;  //i表示有效IP地址长度，限制长度，防止溢出
          }	
          else
					{
             memset(DeviceConfig.ServerIP,0x00,sizeof(DeviceConfig.ServerIP));
						 memcpy(DeviceConfig.ServerIP, pSmsRecevData, i);		
						 UploadFlag = UploadFlash(DeviceConfig.ServerIP, 5);
          }						
				  if(UploadFlag ==1)
					{
              printf("\r\nServer IP upload success!!\r\n");    //测试使用
					  	Delay_ms(2000);  //短信发送缓冲
						  Sms_Send("\rServer IP upload success!!\r");
						  Delay_ms(2000);  //短信发送缓冲
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //将发送数组清零
						  snprintf( MessageSend , sizeof(MessageSend),"\rserver IP is %s \r",    DeviceConfig.ServerIP  );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //将需要发送的短信利用串口显示
						   Delay_ms(2000);  //短信发送缓冲
						  AlarmSMS_Send(MessageSend);                     //短信发送配置信息
          }
				  else           //对非法输入值给出提示（负值不做非法处理，自动取绝对值）
				  {
             printf("\r\nInput server IP not correct!\r\nPlease check and retry.\r\n");    //测试使用
						 Delay_ms(2000);  //短信发送缓冲
						 Sms_Send("\rInput server IP not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //复位变量
				  pSmsRecevBuff = NULL;    //复位变量
       }
//(6)设置网络服务端口号			 
			 pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, ServerPortSet,sizeof(Usart3_recev_buff),strlen(ServerPortSet));   //设置服务器端口号
//			 pSmsRecevBuff = Find_String(pBuff,"casic_set_server_port_");    //设置服务器端口号
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
				  if((i<2)||(i>5))    //输入数据不合法，直接丢弃，不更新配置信息，并给出错误提示，端口号范围有待确认
					{
						 printf("\r\nMessage set input ERROR!!\r\n");     //测试使用 	
//						 i=5;  //i表示有效端口号长度，限制长度，防止溢出
          }	
   				else
					{
             memset(DeviceConfig.ServerPort,0x00,sizeof(DeviceConfig.ServerPort));
             memcpy(DeviceConfig.ServerPort, pSmsRecevData, i);				 
				     UploadFlag = UploadFlash(DeviceConfig.ServerPort, 6);
          }						
				  if(UploadFlag ==1)
					{
              printf("\r\nServer port upload success!!:%s\r\n",pSmsRecevData);    //测试使用
					  	Delay_ms(2000);  //短信发送缓冲
						  Sms_Send("\rServer port upload success!!\r");
						  Delay_ms(2000);  //短信发送缓冲
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //将发送数组清零
						  snprintf( MessageSend , sizeof(MessageSend),"\rServer port is %s\r",    DeviceConfig.ServerPort  );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //将需要发送的短信利用串口显示
						  Delay_ms(2000);  //短信发送缓冲
						  AlarmSMS_Send(MessageSend);                     //短信发送配置信息
          }
				  else           //对非法输入值给出提示（负值不做非法处理，自动取绝对值）
				  {
             printf("\r\nInput server port not correct!\r\nPlease check and retry.\r\n");    //测试使用
					   Delay_ms(2000);  //短信发送缓冲
						 Sms_Send("\rInput server port not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //复位变量
				  pSmsRecevBuff = NULL;    //复位变量
       }
			 
//(7)设置低浓度阈值			 
			// pSmsRecevBuff = Find_String(pBuff,"casic_set_alarm_threshold_");    //设置低浓度报警阈值
			 pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, LowAlarmThresholdSet,sizeof(Usart3_recev_buff),strlen(LowAlarmThresholdSet));   //设置报警阈值
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(LowAlarmThresholdSet)-3))))          //低浓度阈值数据：XX.XX
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
             printf("\r\nMessage set input ERROR!!\r\n");     //测试使用
          }
				  else 
					{
						 if(i>5)
						 {
               i=5;  //i表示有效报警阈值长度，限制长度，防止溢出
             }
						 memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);	
						
             DeviceConfig.LowAlarmLevel .Data_Float = char_to_float(ReceiveTemp); 
           					
						printf("\r\n-----data before write in flash!----%f----",DeviceConfig.LowAlarmLevel  .Data_Float);                   //在写入FLASH之前，输出测试
 				    UploadFlag = UploadFlash((char*)DeviceConfig.LowAlarmLevel.Data_Hex ,7);
          }		
				  if(UploadFlag ==1)
					{
              printf("\r\nlow Alarm threshold upload success!!\r\n");    //测试使用
						  Delay_ms(2000);  //短信发送缓冲
						  Sms_Send("\rlow Alarm threshold upload success!!\r");
						  Delay_ms(2000);  //短信发送缓冲
						  memset(MessageSend,0x00,sizeof(MessageSend));                  //将发送数组清零
						  snprintf( MessageSend , sizeof(MessageSend),"\rlow alarm level is %0.2f \r",    DeviceConfig.LowAlarmLevel .Data_Float  );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //将需要发送的短信利用串口显示
						  Delay_ms(2000);  //短信发送缓冲
						  AlarmSMS_Send(MessageSend);                     //短信发送配置信息
          }
				  else           //对非法输入值给出提示（负值不做非法处理，自动取绝对值）
				  {
             printf("\r\nInput alarm threshold not correct!\r\nPlease check and retry.\r\n");    //测试使用
						 Delay_ms(2000);  //短信发送缓冲
						 Sms_Send("\rInput alarm threshold not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //复位变量
				  pSmsRecevBuff = NULL;    //复位变量
       }

///////////////////////////////////////////////增加配置参数/////////////////////////////////////////////////////////
			 
//	char      StartCollectTime[30]="casic_start_collect_time_";              //首次采集时间设置
//	char      CollectPeriod[30]   ="casic_collect_period_";                 //采集周期设置
//	char 			SendCount[30]       ="casic_send_count_";                     //上传周期设置
//	char 			RetryNum[30]        ="casic_retry_number_";                       //重新上传次数设置

//(8)设置第一次采集时间			 
         //pSmsRecevBuff = Find_String(pBuff,"casic_start_collect_time_");    //设置第一次采集时间
   			 pSmsRecevBuff = Find_SpecialString(pBuff, StartCollectTime,strlen(pBuff),strlen(StartCollectTime));   //设置第一次采集时间
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
             printf("\r\nMessage set input ERROR!!\r\n");     //测试使用
          }
				  else 
					{
						 if(i>4)
						 {
               i=4;  //i表示有效第一次采集时间，限制长度，防止溢出
             }
						 memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);
						 
						 DeviceConfig.CollectStartTime = char_to_int(ReceiveTemp);
						 
					  ConfigData.CollectStartTime_Byte [0] = DeviceConfig.CollectStartTime >> 8; 
					  ConfigData.CollectStartTime_Byte [1] = DeviceConfig.CollectStartTime & 0xff;
						 //printf("\r\n-----data before write in flash!----%s----",CollectStartTime_data);                   //在写入FLASH之前，输出测试
						 printf("\r\n-----data before write in flash!----%d----",DeviceConfig.CollectStartTime);                   //在写入FLASH之前，输出测试
				    for(j=0;j<2;j++)
						 printf("\r\n-----data before write in flash!----%x----",ConfigData.CollectStartTime_Byte [j]);  
						 
						 UploadFlag = UploadFlash((char*)ConfigData.CollectStartTime_Byte   , 8);
          }		
				  if(UploadFlag ==1)
					{
              printf("\r\nStart Collect Time upload success!!\r\n");    //测试使用
						  Delay_ms(2000);  //短信发送缓冲
						  Sms_Send("\rStart Collect Time upload success!!\r");
						  Delay_ms(2000);  //短信发送缓冲
						  memset(MessageSend,0x00,sizeof(MessageSend));              //将发送数组清空
						  snprintf( MessageSend , sizeof(MessageSend),"\rstart collect time is %d\r",    DeviceConfig.CollectStartTime  );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //将需要发送的短信利用串口显示
						  Delay_ms(2000);  //短信发送缓冲
						  AlarmSMS_Send(MessageSend);                     //短信发送配置信息
          }
				  else           //对非法输入值给出提示（负值不做非法处理，自动取绝对值）
				  {
             printf("\r\nInput Start Collect Time not correct!\r\nPlease check and retry.\r\n");    //测试使用
						 Delay_ms(2000);  //短信发送缓冲
						 Sms_Send("\rInput Start Collect Time not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //复位变量
				  pSmsRecevBuff = NULL;    //复位变量
       }			 
                    //	char      StartCollectTime[30]="casic_start_collect_time_";              //首次采集时间设置
                   //	char      CollectPeriod[30]   ="casic_collect_period_";                 //采集周期设置
                   //	char 			SendCount[30]       ="casic_send_count_";                     //上传周期设置
                  //	char 			CollectNum[30]      ="casic_collect_number_";                     //一次采集数量设置
                 //	char 			RetryNum[30]        ="casic_retry_number_";                       //重新上传次数设置
			 
//(10)设置重新上传次数        
   			 pSmsRecevBuff = Find_SpecialString(pBuff, RetryNum,strlen(pBuff),strlen(RetryNum));   //设置重传次数
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
             printf("\r\nMessage set input ERROR!!\r\n");     //测试使用
          }
				  else 
					{
						 if(i>2)
						 {
               i=2;  //i表示有效，限制长度，防止溢出
             }
					
          
             memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);		
						 
						 DeviceConfig.RetryNum    = char_to_int(ReceiveTemp);
						 
						 printf("\r\n-----data before write in flash!----%s----",(char*)ReceiveTemp);                   //在写入FLASH之前，输出测试
						 printf("\r\n-----data before write in flash!----%d----", DeviceConfig.RetryNum);                   //在写入FLASH之前，输出测试
				     
						 printf("\r\n-----data before write in flash!----%x----", DeviceConfig.RetryNum); 
						 UploadFlag = UploadFlash((char*)&DeviceConfig.RetryNum    , 10);
}		
				  if(UploadFlag ==1)
					{
              printf("\r\nStart RetryNum upload success!!\r\n");    //测试使用
						  Delay_ms(2000);  //短信发送缓冲
						  Sms_Send("\rStart RetryNum upload success!!\r");
						  Delay_ms(2000);  //短信发送缓冲
						  memset(MessageSend,0x00,sizeof(MessageSend));              //将发送数组清空
						  snprintf( MessageSend , sizeof(MessageSend),"\r Retry Number is %d \r",    DeviceConfig.RetryNum   );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //将需要发送的短信利用串口显示
						  Delay_ms(2000);  //短信发送缓冲
  						AlarmSMS_Send(MessageSend);                     //短信发送配置信息
          }
				  else           //对非法输入值给出提示（负值不做非法处理，自动取绝对值）
				  {
             printf("\r\nInput RetryNum not correct!\r\nPlease check and retry.\r\n");    //测试使用
						 Delay_ms(2000);  //短信发送缓冲
						 Sms_Send("\rInput RetryNum not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //复位变量
				  pSmsRecevBuff = NULL;    //复位变量
       } 			 
 
				 //	char      StartCollectTime[30]="casic_start_collect_time_";              //首次采集时间设置
//	char      CollectPeriod[30]   ="casic_collect_period_";                 //采集周期设置
//	char 			SendCount[30]       ="casic_send_count_";                     //上传周期设置
//	char 			CollectNum[30]      ="casic_collect_number_";                     //一次采集数量设置
//	char 			RetryNum[30]        ="casic_retry_number_";                       //重新上传次数设置

			 
//(11)设置高浓度报警阈值       
   			pSmsRecevBuff = Find_SpecialString(Usart3_recev_buff, HighAlarmThresholdSet,sizeof(Usart3_recev_buff),strlen(HighAlarmThresholdSet));   //设置报警阈值
			 if((pSmsRecevBuff != NULL)&&(pSmsRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-strlen(HighAlarmThresholdSet)-3))))          //高浓度阈值数据：XX.XX
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
             printf("\r\nMessage set input ERROR!!\r\n");     //测试使用
          }
				  else 
					{
						 if(i>5)
						 {
               i=5;  //i表示有效报警阈值长度，限制长度，防止溢出
             }
						 memset(ReceiveTemp,0x00,sizeof(ReceiveTemp));
						 memcpy(ReceiveTemp, pSmsRecevData, i);	
						
             DeviceConfig.HighAlarmLevel .Data_Float = char_to_float(ReceiveTemp); 
           					
						printf("\r\n-----data before write in flash!----%f----",DeviceConfig.HighAlarmLevel  .Data_Float);                   //在写入FLASH之前，输出测试
 				    UploadFlag = UploadFlash((char*)DeviceConfig.HighAlarmLevel.Data_Hex ,11);
          }		
				  if(UploadFlag ==1)
					{
              printf("\r\nHigh Alarm threshold upload success!!\r\n");    //测试使用
						  Delay_ms(2000);  //短信发送缓冲
						  Sms_Send("\rHigh Alarm threshold upload success!!\r");
						  Delay_ms(2000);  //短信发送缓冲
						
						  memset(MessageSend,0x00,sizeof(MessageSend));              //将发送数组清空
						  snprintf( MessageSend , sizeof(MessageSend),"\rhigh alarm level is %0.2f\r",    DeviceConfig.HighAlarmLevel .Data_Float );
						  printf("\r\nSend char to phone %s\r\n",MessageSend);           //将需要发送的短信利用串口显示
						  Delay_ms(2000);  //短信发送缓冲
						  AlarmSMS_Send(MessageSend);                     //短信发送配置信息
          }
				  else           //对非法输入值给出提示（负值不做非法处理，自动取绝对值）
				  {
             printf("\r\nInput alarm threshold not correct!\r\nPlease check and retry.\r\n");    //测试使用
						 Delay_ms(2000);  //短信发送缓冲
						 Sms_Send("\rInput alarm threshold not correct!\rPlease check and retry.\r");
          }
					UploadFlag =0;           //复位变量
				  pSmsRecevBuff = NULL;    //复位变量
       }
				 //	char      StartCollectTime[30]="casic_start_collect_time_";              //首次采集时间设置
//	char      CollectPeriod[30]   ="casic_collect_period_";                 //采集周期设置
//	char 			SendCount[30]       ="casic_send_count_";                     //上传周期设置
//	char 			CollectNum[30]      ="casic_collect_number_";                     //一次采集数量设置
//	char 			RetryNum[30]        ="casic_retry_number_";                       //重新上传次数设置

			 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*****************************************查询功能****************************************************************/		 
//			 pSmsRecevBuff = Find_String(pBuff,"casic_current_data");    //查询液位信息
			 pSmsRecevBuff = Find_SpecialString(pBuff, SensorDataInquire,sizeof(Usart3_recev_buff),strlen(SensorDataInquire));   //查询传感器采集数据
			 if(pSmsRecevBuff != NULL)           //有查询液位数据查询命令
			 {
				 //DataMessageSend();
				  if(DataCollectCount==0)          //数据采集完成则直接通过短信发送当前液位信息
					{
             DataMessageSend();
          }
					else
					{
             ConfigData.SensorDataInquireFlag  =1;  //液位数据未进行采集时，收到查询命令，将标志变量置1，当液位数据采集完成时通过短信发送当前液位信息
          }
				  pSmsRecevBuff = NULL;            //复位变量
       }	 
			 
			 
      	pSmsRecevBuff = Find_SpecialString(pBuff, SensorSetInquire,sizeof(Usart3_recev_buff),strlen(SensorSetInquire));   //查询设备配置信息
			 if(pSmsRecevBuff != NULL)           //
			 {
				  Delay_ms(2000);  //短信发送缓冲
				  SensorSetMessage();
				  Delay_ms(2000);  //短信发送缓冲
				  SensorSetMessage();
				 
        //ConfigData.DeviceSetInquireFlag =1;  //液位数据未进行采集时，收到查询命令，将标志变量置1，当液位数据采集完成时通过短信发送当前液位信息
          
				  pSmsRecevBuff = NULL;            //复位变量
       }	


}
  }
}

/*******************************************************************************
* Function Name  : 将配置信息利用短信的方式发送至手机
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
				 
				 
				 
				 Delay_ms(2000);  //短信发送缓冲
				 printf("\r\n the device set parametor is %s,\r\n",SensorSet);
	        //Sms_Send(SensorSet);
				Delay_ms(2000);  //短信发送缓冲
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
//   char  updata_set[7] = {0x50,0x00,0x14,0x01,0x00,0x00,0x34};            //服务器更新配置命令
//   char  complete_set[9] = {0x50,0x00,0x09,0x02,0x00,0x00,0x34,0x00,0x03};//服务器请求结束命令
//   char  DataRecv_State[8] = {0x50,0x00,0x09,0x02,0x00,0x00,0x34,0x22};   //服务器发送液位数据接收状态命令
//   
//   int TimeSet[7] ={0};                                                   //用于检验服务器下发的时间配置的合法性	
//   unsigned char   ReceiveState[4] ={0x00};                               //记录服务器各批次数据接收状态
//   unsigned char   ReceiveFlag =0xFF;                                     //记录服务器总数据接收状态
//   uint16_t PeriodSet =0;                                                 //用于检验服务器下发的数据采集周期配置的合法性	
//   uint16_t CountSet =0;                                                  //用于检验服务器下发的数据上传次数配置的合法性	
//   u8     Resp_Count=3;                                                   //Response帧发送尝试次数
//   int i=0;     
//   u8 OpCode=0; 

///////////////////////////////////////验证有无收到服务器更新配置应答////////////////////////////////
//   pRecevBuff = Find_SpecialString(Usart3_recev_buff, updata_set, sizeof(Usart3_recev_buff), sizeof(updata_set)); //检查有无配置更新命令请求
//	 if((pRecevBuff != NULL)&&(pRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-19))))        //防止指针越界                               
//	 {    
//		 #if DEBUG_TEST
//     printf("\r\nTime correct fream receive ok!!\r\n");     //测试使用
//		 #endif
//		 SetRev_OK =1;                                          //配置接收成功，标志变量置位
//		 PeriodSet =(pRecevBuff[8]*256) +pRecevBuff[9];
//		 CountSet =(pRecevBuff[10]*256) +pRecevBuff[11];
//		 for(i=0;i<7;i++)
//		 {
//        TimeSet[i] =pRecevBuff[i+12];
//     }
//		 OpCode =pRecevBuff[7];
////****************************参数配置合法性验证*************************//	
//		 if(OpCode == 0xC1)        //只有当命令是0xC1时才更新配置
//		 {
//			 if((PeriodSet>0)&&(CountSet>0))
//			 {
////				 if(CountSet>1440)    //当服务器下发配置大于1440时，说明要求主机不进入休眠状态，发行版需要关闭此功能，对非法数据进行屏蔽，不更新当前配置
////				 { 
//////					 Alive =1;
//////					 DeviceConfig.SendCount =0x0018;        //
//////		       DeviceConfig.CollectPeriod =0x0001;    //
////					 #if DEBUG_TEST
////					 printf("\r\nServer config ERROR!!\r\n");      //测试使用
////					 #endif
////				 }
//				 if((PeriodSet*CountSet)<=1440)
//				 {	 
//					 if(Alive ==1)
//					 {
//						 Alive =0;     //恢复正常休眠模式
//					 }
//					 LSLIQUSET_Handle(pRecevBuff, &DeviceConfig);	
//				 }
//				 else
//				 {
//					 #if DEBUG_TEST
//					 printf("\r\nConfig not correct!!\r\n");      //测试使用
//					 #endif
//				 }
//			 }
//			 else
//			 {
//				   #if DEBUG_TEST
//					 printf("\r\nConfig not correct!!\r\n");      //测试使用
//				   #endif
//			 }
//		 }
////****************************时间配置合法性验证*************************//
//		 if((OpCode == 0xC1)||(OpCode == 0xC2))        //命令为0xC1或0xC2时，均更新时间配置
//		 {
//				  if((TimeSet[0]>=0)&&(TimeSet[0]<=60)&&(TimeSet[1]>=0)&&(TimeSet[1]<=60)&&(TimeSet[2]>=0)&&(TimeSet[2]<=23)		 
//						&&(TimeSet[4]>=1)&&(TimeSet[4]<=31)&&(TimeSet[5]>=1)&&(TimeSet[5]<=12)&&(TimeSet[6]>=0)&&(TimeSet[5]<=99))   //特殊月份有待调整
//				  {
//					  LSTIMESET_Handle(pRecevBuff, &DeviceConfig);
//					  Time_Auto_Regulate(&DeviceConfig);                     //通过服务器进行RTC时钟校准，默认服务器发送的设置请求都需要进行校时操作
//						while(Resp_Count!=0)
//						{
//							if(RespRev_OK ==0) //是否收到服务器下发的配置会话结束帧标志变量
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
//					 printf("\r\nTime not correct!!\r\n");                 //测试使用
//					 #endif
//				 }	  
//				
//		 }
//	   pRecevBuff = NULL;
//	 }	
///////////////////////////////////////验证有无收到服务器会话结束命令////////////////////////////////	 
//	 pRecevBuff = Find_SpecialString(Usart3_recev_buff, complete_set, sizeof(Usart3_recev_buff), sizeof(complete_set)); 
//	 if((pRecevBuff != NULL) &&(pRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-12))))   //防止指针越界    
//	 {
//				Section_Handle();
//				RespRev_OK =1;    
//		 
//		    #if DEBUG_TEST	 
//				printf("\r\nSECTION SEND TEST:");                      //调试使用
//				for(i=0;i<12;i++)
//				{
//					printf("-%x-",pRecevBuff[i]);                        //调试使用
//				}                                                      //调试使用
//				printf("\r\n");                                        //调试使用
//				#endif
//				pRecevBuff =NULL;
//	 }	
///////////////////////////////////////验证有无收到服务器下发的液位数据接收状态//////////////////////	 
//	 pRecevBuff = Find_SpecialString(Usart3_recev_buff, DataRecv_State, sizeof(Usart3_recev_buff), sizeof(DataRecv_State)); 
//	 if((pRecevBuff != NULL) &&(pRecevBuff <(Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1-12))))   //防止指针越界   
//	 {
//				for(i=0;i<4;i++)
//		    {
//					ReceiveState[i] =pRecevBuff[8+i];
//					ReceiveFlag =ReceiveFlag & ReceiveState[i];
//        }
//		    if(ReceiveFlag == 0xFF)
//				{
//					#if DEBUG_TEST	 
//				  printf("\r\n液位数据接收成功!!\r\n");                 //调试使用
//					#endif
//					DatRev_OK =1;                                         //数据完全正确接收标志变量       
//					for(i=0;i<4;i++)
//		      {
//						ReceiveState[i] =0x00;                              //接收状态复位
//          }
//				}
//				else
//				{
//             ;//数据有丢失，需要重传，后续有待完善
//        }

//				pRecevBuff =NULL;
//	 }	
//	      
//}






/*******************************************************************************
* Function Name  : GPRS_Receive_NetLogin
* Description    : 接收GPRS上网注册的数据
* Input          : None
* Output         : None
* Return         : 上网注册状态
*******************************************************************************/
unsigned char GPRS_Receive_NetLogin(void)
{
	 char*            pRecevBuff =NULL;
	
	 #if DEBUG_TEST
	 printf("\r\nGPRS Net Login Receive Analysis ...\r\n");          //测试使用
	 #endif

	  ////////////////////////////////////////////////////////////////////////////////////////////////////////
    pRecevBuff = Find_SpecialString(Usart3_recev_buff, "+CGREG: 1,1", sizeof(Usart3_recev_buff), 11);  //检查模块入网状态
		if(pRecevBuff!=NULL)                                                
		{   
#if DEBUG_TEST
			 printf("\r\nNet Register OK!!\r\n"); 
#endif			
			 CSQ_OK =1;              //握手有响应时，标志变量置1			 
		   pRecevBuff =NULL;
			 return  1;
		}	 
		//////////////////////////////////////////////////////////////////////////////////////////////////////
    pRecevBuff = Find_SpecialString(Usart3_recev_buff, "+CGREG: 1,5", sizeof(Usart3_recev_buff), 11);  //检查模块入网状态
		if(pRecevBuff!=NULL)                                                
		{   
#if DEBUG_TEST
			 printf("\r\nNet Register OK!!Roaming on!!\r\n");   
#endif
			 CSQ_OK =1;              //握手有响应时，标志变量置1			 
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
   printf("\r\nTCP Connect is in process...\r\n");          //测试使用
	 #endif

		pRecevBuff = Find_SpecialString(Usart3_recev_buff, "CONNECT", sizeof(Usart3_recev_buff), 7);  //检查模块入网状态
		//pRecevBuff = Find_SpecialString(Usart3_recev_buff, "CONNECT", sizeof(Usart3_recev_buff), 10);  //检查模块入网状态
	  if(pRecevBuff != NULL)                                       //网络连接出现故障时，对3G模块复位
		{
			 return 1;
		}
		return 0;
}

/*******************************************************************************
* Function Name  : GPRS_Receive_DataAnalysis
* Description    : 3G模块进行数据的第一层解析
* Input          : None
* Output         : None      
* Return         : None
*******************************************************************************/
unsigned char GPRS_Receive_DataAnalysis(u8* pDeviceID, u16 sNodeAddress)	
{
	 u16     Recev_Flag2 =0;                   //接收数据正确性标志变量
	 char*   pRecevBuff =NULL; 
   char    NewMessageIndicate[7] ="+CMGL:";  //收到未读短信指示

   char   RecevFromCollector[2]={0xA3,0x20};       //接收服务器发送的数据标志序列
   u8     PayloadLen =0;
   u16    CrcVerify =0x0000;
   u16    CrcRecev  =0x0000;
   int k;

	 #if DEBUG_TEST
   printf("\r\n接收数据解析!!\r\n");          //测试使用
	 #endif

//////////////////////////////////////////////////////////////////////////////////
	 pRecevBuff = Find_SpecialString(Usart3_recev_buff, NewMessageIndicate,sizeof(Usart3_recev_buff),strlen(NewMessageIndicate));    //检查是否收到短信
   {
		 
		
		 
			if(pRecevBuff !=NULL)
			{ 
#if DEBUG_TEST
				 printf("\r\n接收短信数据解析!!\r\n");          //测试使用 
#endif				
         Sms_Analysis(pRecevBuff);          //接收短信解析
				 pRecevBuff =NULL;                  //复位查询指针
				 Delay_ms(3000);                    //等待时间不能太短，否则无法成功清空短信记录   
				 mput("AT+CMGD=1,3");               //删除全部已发、未发和已读短信
				 Delay_ms(500);           

				 return 1;
		  }  
   }

	 	 
///////////////////////////////////////////////////////////////////////////////////////////////////
   pRecevBuff = Find_SpecialString(Usart3_recev_buff,RecevFromCollector,sizeof(Usart3_recev_buff),sizeof(RecevFromCollector));  //检查有无收到主站回复
	 if((pRecevBuff != NULL)&&(pRecevBuff< Usart3_recev_buff+(sizeof(Usart3_recev_buff)-1)))  //防止指针越界          
	 {	
#if DEBUG_TEST
      printf("\r\n接收上位机数据解析!!\r\n");          //测试使用 		
#endif		 
	                                                          			
				 if(((u8)pRecevBuff[0]==0xA3)&&(pRecevBuff[1]==0x20))//第二层校验
					{
					Treaty_Data_Analysis(((u8*)pRecevBuff), &Recev_Flag2, pDeviceID, sNodeAddress);  //解析接收数据    					
           }
							else
							{
									#if DEBUG_TEST	 
									printf("\r\nReceive  data not correct!!\r\n");      //测试使用
									#endif
								 Delay_ms(500);	
	                pRecevBuff =NULL;
	                 return 0;
              }
          }	
				
	return Recev_Flag2;
   
	 #if DEBUG_TEST	 
	 printf("\r\n接收数据解析完成!!\r\n");                 //调试使用
   #endif
}




/*******************************************************************************
* Function Name  : XX
* Description    : 初始化一些配置参数
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
//	uint8_t  Flag =0;   //合法性判断
//	uint8_t  FlashStateIndicate[9] = {0x00};            //"FLASH_OK";
//  uint8_t  FlashStateFlag =0;     //用于指示Flash状态是否正常，为0时表示正常，为1时表示发生故障
	int i;
		 #if DEBUG_TEST	
  printf("\r\nConfigData_Init start...\r\n");  //测试使用
     #endif
  BKP_TamperPinCmd(DISABLE);                   //

/******************************************************************************************/
//(2)采集周期
	DataRead_From_Flash(0,2,0, ConfigData.CollectPeriod_Byte,2); //从Flash中读取液位计采集间隔
  Temp =ConfigData.CollectPeriod_Byte[0]*256 + ConfigData.CollectPeriod_Byte[1];
  //printf("------------%d------------",Temp);
  if((Temp>0)&&(Temp<=60))                    //可配置1~60分钟进行数据采集间隔
	{
		Para->CollectPeriod =  Temp;
	}
	else
	{
    Para->CollectPeriod =60;           //每隔15分钟采集一次数据。
  }
		printf("\r\n**************采集周期-->> %d ***********\r\n",Para->CollectPeriod);       //输出测试
	
//(3)上报周期	
  DataRead_From_Flash(0,3,0, ConfigData.SendCount_Byte ,2);             //从Flash中读取上传周期
  Temp =ConfigData.SendCount_Byte[0]*256 + ConfigData.SendCount_Byte[1];
	
	//printf("------------%d------------",Temp);
  if((Temp>0)&&(Temp<=1440))  
	{
		Para->SendCount =  Temp;
	}
	else
	{    
    Para->SendCount =60;              //1小时发送一次数据
  }	
	
  printf("\r\n**************上报周期-->> %d *********** \r\n",Para->SendCount );   //输出测试
//(4)报警手机号
   DataRead_From_Flash(0,4,0, (u8*)ConfigData.AlarmPhoneNum,15);     //从Flash中读取预设报警手机卡号

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
	if(((length==11)||(length==13)||(length==15)) && (ConfigData.AlarmPhoneNum[0] == '8') && (ConfigData.AlarmPhoneNum[1] == '6'))          //物联网卡必须加86前缀才能成功发短信，所以最大长度为15
	{
     memcpy(Para->AlarmPhoneNum, ConfigData.AlarmPhoneNum, length);        
  }
	else                                                     
	{
		 memcpy(Para->AlarmPhoneNum,"861064617178004",15);   //读取数据无效时，初始化报警电话，后续需要根据卡号做相应调整 测试手机号码
  }
		printf("\r\n**************报警电话-->> %s *********** \r\n", Para->AlarmPhoneNum );   //输出测试

	//(5)网络服务IP号	
	DataRead_From_Flash(0,5,0, (u8*)ConfigData.ServerIP,15);   //从Flash中读取预设服务器IP
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

	if(9<length<15)                           //对IP地址合法性做初步筛选，后续有待完善 
	{
     memcpy(Para->ServerIP, ConfigData.ServerIP, length);        
  }
	else                                                     
	{
		 memcpy(Para->ServerIP,"124.42.118.86",14);                  //读取数据无效时，初始化服务器IP   
  }
	
		printf("\r\n**************网络IP-->> %s *********** \r\n", Para->ServerIP );   //输出测试
//(6) 网络服务端口号	
	DataRead_From_Flash(0,6,0, (u8*)ConfigData.ServerPort,5);   //从Flash中读取预设服务器端口号
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
	if(length>0)                                              //对服务器端口号合法性做初步筛选，后续有待完善 
	{
     memcpy(Para->ServerPort, ConfigData.ServerPort, length);        
  }
	else                                                     
	{ 
		 memcpy(Para->ServerPort,"2017",4);                          //读取数据无效时，初始化服务器端口号 
  }
	
		printf("\r\n**************网络端口号-->> %s *********** \r\n", Para->ServerPort );   //输出测试


//(7)低浓度报警阈值
	DataRead_From_Flash(0,7,0, ConfigData.LowAlarmLevel .Data_Hex ,4);   //从Flash中读取预设报警阈值

	fTemp = ConfigData.LowAlarmLevel .Data_Float ;
	
  //printf("\r\n---read low level ---%f----\r\n",fTemp );                       //打印FLASH读取的原始数据
   	
                               

		 if( (0.0<fTemp ) && ( fTemp <25.0) )
     Para->LowAlarmLevel  .Data_Float  =ConfigData.LowAlarmLevel .Data_Float  ;      

	else                                                     
	{ 
		 Para->LowAlarmLevel .Data_Float  =25.0;                        //读取数据无效时，将报警阈值设为25.0，当报警阈值为0时，不会触发报警事件
  }
	
	printf("\r\n**************低报警浓度-->> %0.2f ************\r\n", Para->LowAlarmLevel  .Data_Float );   //输出测试
	
// （8）开始采集时间
	DataRead_From_Flash(0,8,0,ConfigData.CollectStartTime_Byte ,2); //从Flash中读取第一次采集时间

	//Temp =  char_to_int(CharArry);
	
  Temp =ConfigData.CollectStartTime_Byte [0]*256 + ConfigData.CollectStartTime_Byte [1];
	
	//printf("\r\n---read CollectStartTime ---%d----\r\n",Temp );                       //打印FLASH读取的原始数据
  if((Temp>=0)&&(Temp<=1440))  
	{
		Para->CollectStartTime  =  Temp;
	}
	else
	{
    Para->CollectStartTime  = 0;           //第一次采集时间为0点钟
  }

	printf("\r\n**************开始采集时间-->> %d ***********\r\n", Para->CollectStartTime );   //输出测试
	
	
//(9)已采集数量
	DataRead_From_Flash(0,9,0,    &(ConfigData.CollectNum)   ,1); //从Flash中读取当前采集数量
	
//	printf("\r\n---read retry number ---%s----\r\n",(u8*)ConfigData.CollectNum_Byte );                       //打印FLASH读取的原始数据
  //Temp =ConfigData.CollectNum_Byte [0]*256 + ConfigData.CollectNum_Byte [1];
	Temp = ConfigData.CollectNum;
	//printf("\r\n---read CollectNum ---%d----\r\n",Temp );                       //打印FLASH读取的原始数据
//  if((0 <=ConfigData.CollectNum )&&(ConfigData.CollectNum <= MAX_COLLECTNUM))  
//	{
//		if(ConfigData.CollectNum <=( Para->SendCount /Para->CollectPeriod ))
//		{ Para->CollectNum  =  ConfigData.CollectNum; }
//		else
//		{ Para->CollectNum =( Para->SendCount /Para->CollectPeriod );}
//	}
//	else
//	{
//    Para->CollectNum  = ( Para->SendCount /Para->CollectPeriod ) ;           //当前采集的传感器数据量 
//  }
	
		printf("\r\n**************已采集数量-->> %d ***********\r\n", Temp );   //输出测试
//(10)重传次数
	DataRead_From_Flash(0,10,0, &(ConfigData.RetryNum ) ,1); //从Flash中读取重传次数
  
	//printf("\r\n---read retry number ---%d----\r\n",ConfigData.RetryNum );                       //打印FLASH读取的原始数据
	
	Temp =ConfigData.RetryNum ;
  if((Temp>0)&&(Temp<=10))  
	{
		Para->RetryNum  =  Temp;
	}
	else
	{
    Para->RetryNum  =3;           //默认为3
  }
	printf("\r\n**************重传次数-->> %d ***********\r\n", Para->RetryNum );   //输出测试
	

//(11)高浓度报警阈值
	DataRead_From_Flash(0,11,0, ConfigData.HighAlarmLevel .Data_Hex  ,4);    //从Flash中读取高浓度报警
  
	//printf("\r\n---read high alarm level ---%f----\r\n",ConfigData.HighAlarmLevel .Data_Float );                       //打印FLASH读取的原始数据
	
	fTemp =ConfigData.HighAlarmLevel .Data_Float  ;
  if((fTemp>0.0) && (fTemp < 50.0) && (fTemp > ConfigData.LowAlarmLevel .Data_Float ))  
	{
		Para->HighAlarmLevel .Data_Float   =  fTemp;
	}
	else
	{
    	Para->HighAlarmLevel .Data_Float  =  50.0;           //报警高浓度阈值默认为50%
  }
	printf("\r\n**************高报警浓度-->> %0.2f ***********\r\n", 	Para->HighAlarmLevel .Data_Float );   //输出测试
	
//(12)设备已工作次数
	DataRead_From_Flash(0,12,0, (ConfigData.WorkNum_Byte),4);    //从Flash中读取已工作次数

	Temp=ConfigData.WorkNum_Byte [0]*256+ConfigData.WorkNum_Byte [1];
  Para->WorkNum  = Temp; 
	printf("\r\n**************工作次数-->> %d ***********\r\n", 	Para->WorkNum);   //输出测试
//(13)电池电量
	printf("\r\n**************电池电量-->> %d%% ***********\r\n", 	Para->BatteryCapacity=DS2780_Test());   //输出测试
//	Para->Time_Sec  =0x00;
//	Para->Time_Min  =0x00;
//	Para->Time_Hour =0x00;
//	Para->Time_Mday =0x00;
//	Para->Time_Mon  =0x00;
//	Para->Time_Year =0x00;
//  Para->BatteryCapacity =0x64;    //电池电量，暂定为100%
	Para->MessageSetFlag  =0;       
}





 
 
 




/**********************************************END******************************************/










