#include "stm32f10x.h"
#include "gprs.h"
#include "bsp_SysTick.h"
#include "modbus.h"
#include "string.h"
#include "AiderProtocol.h"
#include "433_Wiminet.h"
#include "bsp_rtc.h"
#include "SPI_Flash.h"
#include "API-Platform.h"
#include "SensorInit.h"
//#include  "bsp_date.h"

extern  struct Sensor_Set  DeviceConfig;          //液位计配置信息结构体
extern  struct    SMS_Config_RegPara   ConfigData;

extern u8     DataCollectCount;                   //数据采集计数器
extern char   SendData_Flag; 

extern struct    rtc_time        systmtime;        //系统时间
extern uint8_t   Usart3_send_buff[300];
extern uint8_t   DMA_UART3_RECEV_FLAG ;      //USART3 DMA接收标志变量

u8 phone_number_next=0;                   //电话号码的下一个指示
u8 ip_number_next=0;                     //网络IP的下一个指示
u8 port_number_next=0;                   //网络端口的下一个指示              

/*******************************************************************************
* Function Name  : UploadFlash
* Description    : 输入需要存储的字符串，并将存储位置输入
* Input          : None
* Output         : 返回存储成功标志
* Return         : None
*******************************************************************************/
uint8_t  UploadFlash(char* pSetPara, uint8_t  InstructCode)
{
 
 
  uint8_t   j=0;
  uint8_t   Counter=0;
  
	switch(InstructCode)
	{	
		case 2:          //存储采集周期
		{   
			 for(j=0;j<strlen(pSetPara);j++)		
			 //printf("\r\n---------%x----------\r\n",pSetPara[j]);
		   DataWrite_To_Flash(0,2,0,(uint8_t*)pSetPara,2);      //将采集周期写入Flash
			 DeviceConfig.MessageSetFlag =1;     //标示当前液位仪参数通过短信修改	
       printf("\r\n存储采集周期：%s\r\n",pSetPara);			          //测试使用						
			 return 1;
		}
		case 3:          //存储上传周期
		{  
			 for(j=0;j<strlen(pSetPara);j++)
			 //printf("\r\n---------%x----------\r\n",pSetPara[j]);
			
		   DataWrite_To_Flash(0,3,0,(uint8_t*)pSetPara,2);      //将上传次数写入Flash
			 DeviceConfig.MessageSetFlag =1;     //标示当前液位仪参数通过短信修改	
       printf("\r\n存储上报周期：%s\r\n",pSetPara);			          //测试使用						
			 return 1;
		}
		
		case 4:            //存储报警号码
		{
			  //手机号码的判别条件，后续需要增加。
			 DataWrite_To_Flash(0,4,0,(uint8_t*)pSetPara,strlen(pSetPara));   //将报警号码写入Flash 
       DeviceConfig.MessageSetFlag =1;                                  //标示当前液位仪参数通过短信修改		 
       printf("\r\n存储报警电话:%s\r\n",pSetPara);			          //测试使用
			 return 1;
		}
		
		case 5:              //存储服务器IP
		{

			 for(j=0,Counter=0;j<strlen(pSetPara);j++)                       //调用函数已经做了防溢出处理,strlen(pSetPara)<=15
			 {
          if((pSetPara[j]>='0')&&(pSetPara[j]<='9'))
					{
//             CharTemp[j] =pSetPara[j];
						 ;
          }
				  else if(pSetPara[j]=='.')                                    //分隔符统计
					{
						 Counter++;
          }
					else
					{
              break;
          }
       }
			 if(Counter==3)
			 {
          DataWrite_To_Flash(0,5,0, (uint8_t*)pSetPara,strlen(pSetPara));   //将服务器IP写入Flash 
       }
			 else
			 {
          printf("\r\nInput Server IP ERROR!!\r\n");
       }
			 DeviceConfig.MessageSetFlag =1;     //标示当前液位仪参数通过短信修改		 
			  printf("\r\n存储网络IP号: %s\r\n",pSetPara);			          //测试使用
			 return 1;
			 
		}
		
		case 6:           //存储服务器端口号
		{
			 DataWrite_To_Flash(0,6,0,(uint8_t*)pSetPara,strlen(pSetPara));      //将服务器端口号写入Flash
			 DeviceConfig.MessageSetFlag =1;     //标示当前液位仪参数通过短信修改	
        printf("\r\n存储网络端口号: %s\r\n",pSetPara);			          //测试使用			
			 return 1;
			
		}

		case 10:          //存储重传次数
		{
		   DataWrite_To_Flash(0,10,0,(uint8_t*)pSetPara,1);      //将重传次数写入Flash
			 DeviceConfig.MessageSetFlag =1;     //标示当前液位仪参数通过短信修改		 
			 printf("\r\n 存储重传次数: %x\r\n",pSetPara[0]);			          //测试使用			
			 return 1;
		}
		
		default:
		{
			 printf("\r\nInstruct Code ERROR !!\r\n");
		   return 0;
		}
  }

}

/*******************************************************************************
* Function Name  : Get_Rand_Num
* Description    : 获取0~1000随机数据
* Input          : None
* Output         : 返回随机数
* Return         : None
*******************************************************************************/
unsigned int Get_Rand_Num(void)
{    
	    unsigned int rand_num;
      srand((unsigned)RTC_GetCounter());                                     //设置种子
		  rand_num = rand()%1000;                                          //获取0~1000随机数据
	    return(rand_num);
}

/**************************************************************************
* Function Name  : SendDataToServ()
* Description    : 通信协议的发送操作实现，在用该函数之前，需进行设备类型、上报数据格式、传输方式以及软件版本的定义。
                   相关的定义见SensorInit.h和 AiderProtocol.h
                   本函数将TAG打包后进行发送，如果发送不成功，进行设定值重传。
* Input          : 
                   操作类型：    参考AiderProtocol.h内操作类型；主要包括设备响应SET/GET请求GETRESPONSE，设备主动上报TRAPREQUEST，
                                 设备开机上报信息STARTUPREQUEST，设备响应服务器唤醒WAKEUPRESPONSE，设备检查链接ONLINEREQUEST      
                   发送TAG列表:  参考AiderProtocol.h内TAG结构体，将需要发送的TAG传递进来，不传输时可传递NULL
                   TAG数量:			 需要发送的TAG的数量，TAG数量不超过20，不传输时为0
                   设备ID:       6字节的设备ID号  
* Output         : None
* Return         : 发送成功标志
**************************************************************************/
uint8_t SendDataToServ(CommunicateType CommType,struct TagStruct TagList[],uint8_t TagNum,u8* pDeviceID)
{
  DeviceType DevType 				= DEVICE;        //初始化设备类别
	ReportDataType	RDataType =	REPORTDATA;    //初始化上报类型
  SendType   SType = TYPESEND; 	            //通信方式为433通信
	struct   DataFrame  SendData;             //发送数据格式
	struct   DataFrame* pDataFrame =&SendData;  //发送数据指针
	struct   TagStruct* pTag=TagList;           //接收TAG指针
	uint8_t  pSendBuff[300];                    //需要发送的数组
	char*    pChar =NULL;
  u16      RecevFlag =0;           //服务器数据接收标志变量
	uint16_t NodeAddress;            //设备地址
  uint8_t  SendCounter;            //发送次数
	uint16_t rand_num;               //随机数字
	u16      CrcData=0;
	u8       i=0,j=0;
	uint8_t  RevTagNum = 0;       //TAG的数量
	u8       ValidLength =0;     //有效数据长度，Tag空间中实际存储有效数据的长度。
	const u8 PreLength   =16;    //帧前缀长度，指从帧前导码到OID序列（或者Tag序列）的数据长度，包括帧前导码，而不包括OID序列（或者Tag序列）
	const u8 CoreLength  =12;    //关键信息长度，指数据净荷部分长度字段指示的数值，去除OID序列（或者Tag序列）后剩余数据的长度
	const u8 TagPreLength=6;     //TAG的OID长度为4+TAG数据长度2
	uint8_t  TagLength=0;        //TAG的总长度
	uint8_t  Offset=0;           //指针偏移量
	u8       TotalLength;        //除去CRC的数据总长度
	NodeAddress = pDeviceID[4]*256 +pDeviceID[5];
	pChar =(char*)pDataFrame;
	
	memset(pChar,0x00,sizeof(struct DataFrame));   //初始化结构体
	SendData.Preamble =0xA3;
	SendData.Version  = SOFTVERSION;
	for(i=0;i<6;i++)
	{
   SendData.DeviceID[i] = pDeviceID[i];
  }
	SendData.RouteFlag =SType;                              //数据发送方式
	SendData.NodeAddr  =ntohs( NodeAddress);               //调整为网络序，即高字节在前，低字节在后
	SendData.PDU_Type  =(CommType<<8)+(1<<7)+DEVICE;
	SendData.PDU_Type  =ntohs(SendData.PDU_Type);        //调整为网络序，即高字节在前，低字节在后
	SendData.Seq       =1;
	
	
	if(pTag!=NULL)
		{
				RevTagNum=TagNum;                     //如果指针不空，则证明有数据传输过来，接收TAG的数量
				for(i=0;i<RevTagNum;i++)
				SendData.TagList[i] = TagList[i];
		} 
	else
		{
	  switch(CommType)
	 {
				
		case GETRESPONSE:
		{

			break;
		}
		
		case TRAPREQUEST:
		{ 
			
				SendData.TagList[0].OID_Command = ntohl(DEVICE_QTY);   //调整为网络序，即高字节在前，低字节在后
				SendData.TagList[0].Width =1;                         //
				SendData.TagList[0].Value[0] =DeviceConfig.BatteryCapacity;  //数据上传时的电池剩余电量		  
	
				SendData.TagList[1].OID_Command= ntohl(SYSTERM_DATA);    //调整为网络序，即高字节在前，低字节在后
				SendData.TagList[1].Width =3;                            //
				SendData.TagList[1].Value[0] =systmtime.tm_year-2000;       //系统日期，年
				SendData.TagList[1].Value[1] =systmtime.tm_mon ;            //系统日期，月
				SendData.TagList[1].Value[2] =systmtime.tm_mday ;           //系统日期，日
			
			  
				RevTagNum =2;                     //TAG的总数等于电量+日期
			  
		 	  break;
				
		}			
		case ONLINEREQUEST:
		{
			
			break;
		}
			
		case WAKEUPRESPONSE:
		{
	  SendData.TagList[0].OID_Command =ntohl(DEVICE_WAKEUP); //调整为网络序，即高字节在前，低字节在后
	  SendData.TagList[0].Width =1;
	  SendData.TagList[0].Value[0]=1;
	  RevTagNum =1;
	  		
			break;
		}
			
		case STARTUPREQUEST:
		{
			
			SendData.TagList[0].OID_Command =ntohl(DEVICE_STATE); //调整为网络序，即高字节在前，低字节在后
			SendData.TagList[0].Width =1;
			SendData.TagList[0].Value[0]=1;
			RevTagNum =1;

			break;
		}
			
		default:
		{
			RevTagNum =0;
		}

	}	
	}
	 
  		for(i=0; i<RevTagNum;i++)
			{
			TagLength = SendData.TagList[i].Width +TagPreLength;
			ValidLength += TagLength;
		  
			}
			SendData.Length =CoreLength + ValidLength;  
			SendData.Length =ntohs(SendData.Length );
			memcpy(pSendBuff,pChar,PreLength );                             //进行除TAG的数据复制
			
			SendData.CrcCode =0xffff;                                        //CRC字段赋初值
			TotalLength = PreLength + ValidLength;
			
			Offset=	PreLength;		
			for(i=0; i<RevTagNum;i++)
			{
			
		  pChar = (char*)&(SendData.TagList [i].OID_Command); 
      TagLength = SendData.TagList[i].Width +TagPreLength;				//进行TAG的复制 
			SendData.TagList[i].Width=ntohs(SendData.TagList[i].Width);
     	memcpy((pSendBuff+Offset),pChar,TagLength );
			
			Offset+=TagLength;	
			}
			
			CrcData = CRC16(pSendBuff, TotalLength);   // Update the CRC value
	    SendData.CrcCode =CrcData;
	    pSendBuff[TotalLength+1] = CrcData&0xff;   //CRC低字节在前
	    pSendBuff[TotalLength+2] = CrcData>>8;     //CRC高字节在后
						
			if(DeviceConfig.RetryNum>0)
	{
     SendCounter =DeviceConfig.RetryNum;
  }
	else
	{
     SendCounter =1;
  }
	for(i=0;i<SendCounter;i++)
	{
     RecevFlag =SendMessage(pSendBuff, TotalLength+2);
		 if(RecevFlag !=0)                     //成功接收到数据
		 {
        #if DEBUG_TEST	 
				printf("\r\nReceive TrapResponse success!\r\n"); //测试使用
				#endif
			  break;     
     }
		   rand_num=Get_Rand_Num();
#if DEBUG_TEST
		 	 printf("\r\nthe rand_num is %d\r\n",rand_num);
#endif
		  Delay_ms(3000+2*rand_num);                        //随机发送
		
  }	
}
/*******************************************************************************
* Function Name  : TrapData
* Description    : 进行传感器数据的主动上报
* Input          : 设备节点
* Output         : None
* Return         : None
*******************************************************************************/
void TrapData(u8* pDevID)
{
	  int i,j;
	  uint8_t Tag_Count=0;     
    unsigned char  readdata[7];	
	  struct TagStruct Taglist[MAX];
	  uint16_t CollectTime;
	
		Taglist[0].OID_Command = ntohl(DEVICE_QTY);   //调整为网络序，即高字节在前，低字节在后
		Taglist[0].Width =1;                         //
		Taglist[0].Value[0] =DeviceConfig.BatteryCapacity;  //数据上传时的电池剩余电量		  
	
	  Taglist[1].OID_Command= ntohl(SYSTERM_DATA);    //调整为网络序，即高字节在前，低字节在后
		Taglist[1].Width =3;                            //
		Taglist[1].Value[0] =systmtime.tm_year-2000;       //系统日期，年
		Taglist[1].Value[1] =systmtime.tm_mon ;            //系统日期，月
		Taglist[1].Value[2] =systmtime.tm_mday ;           //系统日期，日
			
			  
	  Tag_Count =2;                     //TAG的总数等于电量+日期
	
		for(i=0;i< DataCollectCount;i++)    //进行上报数据TAG的填充
	{
		///////////////////////////////////////?FLASH????????????////////////////////////////////
		DataRead_From_Flash(1,i+1,0, readdata ,sizeof(readdata));
		CollectTime = readdata[0] *256 + readdata[1];
		for(j=0;j<4;j++)
		Taglist[2+i].OID_Command =ntohl((DeviceConfig.CollectPeriod<<11)+CollectTime  +((0xC0 + REPORTDATA )<<24)); 
		
		Taglist[2+i].Width =4;
		for(j=0;j<Taglist[2+i].Width;j++)
		Taglist[2+i].Value[j]= readdata[2+j];
    Tag_Count++ ;                                           
  }
	
	 SendDataToServ(TRAPREQUEST,Taglist,Tag_Count,pDevID);                //433模块主动上传数据
}


/*******************************************************************************
* Function Name  : 二院协议数据分析
* Description    : 用于接收服务器配置，没有9字头标志
* Input          : None                       
* Output         : None
* Return         : None
*******************************************************************************/
void Treaty_Data_Analysis(u8* pTreatyBuff, u16* pFlag, u8* pDeviceID, u16 NodeAddress)	
{
	u8   DataLen =0;          //Tag序列或者OID序列的长度
	u16  PduType =0;
  
	u8   i=0,j=0,k=0;
	u8*  pChar =NULL;
	u32* pOid  =NULL;
  u8   next = 0;
	struct DataFrame ParaRequest;
  struct TagStruct RecTagList[MAX];           //接收到的TAG
	uint8_t RecTagNum;                          //接收到TAG的数量
  PduType =pTreatyBuff[13]*256 +pTreatyBuff[14];   //结算接收数据PDU编码
  switch(PduType)
	{
   /******************************************服务器请求********************************************/	     
        case ((GETREQUEST<<8)+(1<<8)+DEVICE):
				{
						printf("\r\nReceive Get Request Command from Server.\r\n");    //监测到服务器发送的请求命令
						*pFlag =PduType;
						DataLen =pTreatyBuff[2]*256 +pTreatyBuff[3]-12; //接收的OID的总长度
					  ParaRequest.Tag_Count =DataLen/4;               //接收的OID的数量,即TAG数量
					  if((ParaRequest.Tag_Count>0)&&(ParaRequest.Tag_Count <= MAX))   //限定OID的数量，控制在20个
						{
							  pOid = (u32*)&pTreatyBuff[16];
							  j    =ParaRequest.Tag_Count;
							  RecTagNum = j;                            //接收TAG的总数
								for(i=0;i<ParaRequest.Tag_Count;i++)
								{ 
									  ParaRequest.TagList[i].OID_Command =(CommandType)*pOid;                    //进行OID采集   
									  ParaRequest.TagList[i].OID_Command =ntohl( ParaRequest.TagList[i].OID_Command );      //进行网络序列的变化
									  ParaRequest.TagList[i].Width=0;
									
#if DEBUG_TEST									
									  printf("\r\n--Cycle--%4x----.\r\n", ParaRequest.OID_List[i]);   //显示OID
#endif									
										j--;
									  if(j==0)   
										{
                       break;
                    }
									  pOid++;
								}
								printf("\r\n--i:%d--j:%d--%4x----.\r\n",i,j, ParaRequest.TagList[i].OID_Command);    //显示OID
								
            }
					  else
						{
#if DEBUG_TEST	 
							  printf("\r\nReceive Command OID not correct.\r\n");    //????
#endif
            }

						DMA_UART3_RECEV_FLAG =0;     //清接收标志变量
						SendDataToServ(GETRESPONSE, ParaRequest.TagList,RecTagNum,pDeviceID);    //进行响应
					  //GetResponse( ParaRequest, Usart3_send_buff, pDeviceID,  NodeAddress);
					  break;
        }	

/******************************************设置设备参数请求********************************************/	
        case ((SETREQUEST<<8)+(1<<8)+DEVICE):                  //接收服务器下发的配置
				{
            #if DEBUG 
						printf("\r\nReceive Set Request Command from Server.\r\n");    //测试使用
						#endif
						*pFlag =PduType;
					  DataLen =pTreatyBuff[2]*256 +pTreatyBuff[3]-12;    //接收到的Tag序列的总长度
					  if(DataLen >6)           //至少存在一个合法的配置参数
						{
                pOid = (u32*)(pTreatyBuff+16);    //服务器下发的第一个OID
							  i=0;
							  while( DataLen >6 )
								{
									 printf("\r\n--Cycle--%4x----.\r\n",ntohl(*pOid));         //测试使用
									 pChar = (u8*)pOid;
                   switch(ntohl(*pOid))
									 {										
											case SYSTERM_TIME:     //系统时间
											{
													DeviceConfig.Time_Year =*(pChar+6);
												  DeviceConfig.Time_Mon  =*(pChar+7);
												  DeviceConfig.Time_Mday =*(pChar+8);
												  DeviceConfig.Time_Hour =*(pChar+9);
												  DeviceConfig.Time_Min  =*(pChar+10);
												  DeviceConfig.Time_Sec  =*(pChar+11);
												  printf("\r\n-年-月-日-时-分-秒：-%d--%d--%d--%d--%d--%d--.\r\n",DeviceConfig.Time_Year,DeviceConfig.Time_Mon,DeviceConfig.Time_Mday,
												                 DeviceConfig.Time_Hour,DeviceConfig.Time_Min, DeviceConfig.Time_Sec  );         //测试使用
												  if((DeviceConfig.Time_Mon<=12)&&(DeviceConfig.Time_Mday<=31)&&(DeviceConfig.Time_Hour<=23)&&(DeviceConfig.Time_Min<=60)&&(DeviceConfig.Time_Sec<=60)) //参数合法性判定
													{
                            Time_Auto_Regulate(&DeviceConfig);             //通过服务器下发参数进行RTC时钟校准，
                          }
												  ParaRequest.TagList[i].OID_Command=SYSTERM_TIME;
                         	ParaRequest.TagList[i].Width = 6;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
													
												  pOid =(u32*)(pChar+12);                          //指针后移1个Tag
												  DataLen = DataLen-12;
												 	i++;        //合法OID计数器
													break;							
											}	
											case CLT1_ITRL1:       //一时区采集间隔
											{
													DeviceConfig.CollectPeriod =pChar[6]*256+pChar[7];
												  ConfigData.CollectPeriod_Byte [0]=*(pChar+6);
												  ConfigData.CollectPeriod_Byte [1]=*(pChar+7);
												
												  printf("\r\n---CollectPeriod:-%d---.\r\n", DeviceConfig.CollectPeriod );         //测试使用
												  if(3<DeviceConfig.CollectPeriod<=1440)              //参数合法性判定
													{
                             UploadFlash((char*)ConfigData.CollectPeriod_Byte,2);  //参数存入Flash
                          }
													
													ParaRequest.TagList[i].OID_Command=CLT1_ITRL1;
                         	ParaRequest.TagList[i].Width = 2;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
											
												  pOid =(u32*)(pChar+8);                         //指针后移1个Tag
												  DataLen = DataLen-8;
												 	i++;        //合法OID计数器
													break;
											}	
//											case CLT1_CNT1:        //一时区采集次数
//											{
//													DeviceConfig.CollectNum =pChar[6]*256+pChar[7];
//													printf("\r\n-CollectNum:-%2x---.\r\n", DeviceConfig.CollectNum );         //测试使用
//												  if(DeviceConfig.CollectNum<=1440)              //参数合法性判定
//													{
//                             UploadFlash((char*)&(DeviceConfig.CollectNum), 9);  //参数存入Flash
//                          }
//												  ParaRequest.OID_List[i] =CLT1_CNT1;            
//												  pOid =(u32*)(pChar+8);                         //指针后移1个Tag
//												  DataLen = DataLen-8;
//												 	i++;        //合法OID计数器
//													break;
//											}
											
//									
											case UPLOAD_CYCLE:     //数据上报周期
											{
													DeviceConfig.SendCount  =pChar[6]*256+pChar[7];
												  ConfigData.SendCount_Byte  [0]=*(pChar+6);
												  ConfigData.SendCount_Byte [1]=*(pChar+7);
												 
												  if(0<DeviceConfig.SendCount <=1440)              //参数合法性判定
													{
                             UploadFlash((char*) ConfigData.SendCount_Byte , 3);  //参数存入Flash
                          }
											  	printf("\r\n-UploadCycle:-%d---.\r\n", DeviceConfig.SendCount  ); //测试使用
													
													ParaRequest.TagList[i].OID_Command=UPLOAD_CYCLE;
                         	ParaRequest.TagList[i].Width = 2;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
												
												  pOid =(u32*)(pChar+8);                           //指针后移1个Tag
												  DataLen = DataLen-8;
											  	i++;        //合法OID计数器
													break;
											}	
///////////////////////////////////////////////////////////////////////////////////////////add by gao 											
										
											 case SMS_PHONE:     //报警电话号码
											{
                           for(k=0;k<16;k++)												
													{
														DeviceConfig.AlarmPhoneNum [k]  = pChar[6+k];
														if((((pChar[6+k])>'9') ||  (pChar[6+k]<'0')) )
														{
														next=6+k; 
														phone_number_next=k;
	                          //printf("*******phone number %d******",next); 															//??????
														break;
                            }
													}
													
												  if(11<strlen(DeviceConfig.AlarmPhoneNum)<16 )              //参数合法性判定
													{
                             UploadFlash((char*)DeviceConfig.AlarmPhoneNum  ,4);  //参数存入Flash
                          }
											  	printf("\r\n-Server set SMS_PHONE:-%s---.\r\n", DeviceConfig.AlarmPhoneNum   ); //测试使用
												  ParaRequest.TagList[i].OID_Command=SMS_PHONE;
                         	ParaRequest.TagList[i].Width = phone_number_next;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
												
												  pOid =(u32*)(pChar+next);                           //指针后移1个Tag
												  DataLen = DataLen-next;
											  	i++;        //合法OID计数器
													break;
											}
											
                       	 case BSS_IP:     //网络服务IP地址
											{
												  for(k=0;k<16;k++)
												{
													DeviceConfig.ServerIP  [k] =pChar[6+k];
													if(((((pChar[6+k])>'9') ||  (pChar[6+k]<'0'))  )&&  (pChar[6+k]!='.'))
													{
													next=6+k;	
													ip_number_next=k;
													//printf("*******ip %d******",next);
													break;
                          }
													
												}
												  if(8<strlen(DeviceConfig.ServerIP )<16)              //参数合法性判定
													{
                             UploadFlash((char*)DeviceConfig.ServerIP  , 5);  //参数存入Flash
                          }
											  	printf("\r\n-BSS_IP:-%s---.\r\n", DeviceConfig.ServerIP   ); //测试使用
													ParaRequest.TagList[i].OID_Command=BSS_IP;
                         	ParaRequest.TagList[i].Width = ip_number_next;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
										
												  pOid =(u32*)(pChar+next);                           //指针后移1个Tag
												  DataLen = DataLen-next;
											  	i++;        //合法OID计数器
													break;
											}
												 case BSS_PORT:     //网络服务端口号
											{
												  for(k=0;k<6;k++)
												{
													DeviceConfig.ServerPort  [k] = pChar[6+k];
													if(((pChar[6+k])>'9') ||  (pChar[6+k]<'0'))
													{
													next=6+k;	
													port_number_next=k;
													//printf("*******port %d******",next);
													break;
                           }
												}
												  if(strlen(DeviceConfig.ServerPort )!=0)              //参数合法性判定
													{
                             UploadFlash((char*)DeviceConfig.ServerPort  , 6);  //参数存入Flash
                          }
											  	printf("\r\n-BSS_PORT:-%s---.\r\n", DeviceConfig.ServerPort   ); //测试使用
													ParaRequest.TagList[i].OID_Command=BSS_PORT;
                         	ParaRequest.TagList[i].Width = port_number_next;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
												 
												  pOid =(u32*)(pChar+next);                           //指针后移1个Tag
												  DataLen = DataLen-next;
											  	i++;        //合法OID计数器
													break;
											}


											case DEF_NR:            //重传次数
											{
													DeviceConfig.RetryNum =*(pChar+6);
												  if(1<= DeviceConfig.RetryNum <10)                         //参数合法性判定
													{
                             UploadFlash((char*)&(DeviceConfig.RetryNum), 10);  //参数存入Flash
                          }
												  printf("\r\n-Retry Num-%x---.\r\n", DeviceConfig.RetryNum);   //测试使用
//												  else               //当配置参数不合法时，使用默认参数，同时不更新Flash数据
//													{
//                             DeviceConfig.RetryNum =1;             //为了反馈真实情况不发送默认参数
//                          }
													ParaRequest.TagList[i].OID_Command=DEF_NR;
                         	ParaRequest.TagList[i].Width = 1;
													for(k=0;k<ParaRequest.TagList[i].Width;k++)
												  ParaRequest.TagList[i].Value[k] = pChar[6+k];
												  pOid =(u32*)(pChar+7);                     //指针后移1个Tag
												  DataLen = DataLen-7; 
 	                        i++;               //合法OID计数器												
													break;
											}	
											

////////////////////////////////////////////////////////////////////////////////////////////add by gao 											
											default:
											{
												 #if DEBUG_TEST	
												 printf("\r\nWarning!!Tag OID not recognition!\r\n"); //测试使用
												 #endif
												 pOid =(u32*)(pChar+1);  //指针后移一个字节，查询后续有无合法OID
												 DataLen = DataLen-1;    //指针后移一个字节，查询后续有无合法OID
												 break;
											}
									 }
                }
						}
						if((i>0)&&(i<=20))
						{
               ParaRequest.Tag_Count =i;           //对OID序列进行计数	
							 RecTagNum=i;  
            }
            else
						{
               ParaRequest.Tag_Count  =7;           //当OID计数器值超过20时,将计数器值重置成默认值（默认7）
            }	
		
            DMA_UART3_RECEV_FLAG =0;     //接收标志变量复位						
					  SendDataToServ(GETRESPONSE,ParaRequest.TagList,RecTagNum,pDeviceID);    //进行响应				
					  break;
        }	
				
/******************************************设备数据上报信息响应*********************************************/	
				case ((TRAPRESPONSE<<8)+(1<<8)+DEVICE):                  //收到上报数据的响应
				{
            #if DEBUG_TEST	 
						printf("\r\nReceive Trap Response Command from Server.\r\n");    //测试使用
						#endif
					  /////////////////////////
					  //服务器接收完整性验证，对于一次上传一帧数据的情况，只需通过PDUType进行验证，接收到应答帧即代表接收完整
						*pFlag =PduType;
					  break;
        }	
				
/******************************************设备检查链接响应*********************************************/	
				case ((ONLINERESPONSE<<8)+(1<<8)+DEVICE):                      //设备检查链接响应
				{
            #if DEBUG_TEST
						printf("\r\nReceive Online Response Command from Server.\r\n");    //测试使用
            #endif
					
						*pFlag =PduType;
					  break;
        }	
/******************************************设备开机上报信息响应*********************************************/				
				case ((STARTUPRESPONSE<<8)+(1<<8)+DEVICE):                       //收到设备开机上报信息响应
				{
            #if DEBUG_TEST	 
						printf("\r\nReceive Startup Response Command from Server.\r\n");    //测试使用
						#endif
						*pFlag =PduType;
					  break;
        }	
				
/******************************************服务器唤醒设备请求*********************************************/				
				case ((WAKEUPREQUEST<<8)+(1<<8)+DEVICE):
				{
            #if DEBUG_TEST	 
						printf("\r\nReceive Wakeup Request Command from Server.\r\n");    //测试使用
						#endif
						*pFlag =PduType;
						
					  break;
        }	

        default:
				{
					 #if DEBUG_TEST	
           printf("\r\nWarning!!PDU Type not recognition!\r\n"); //测试使用
					 #endif
					 break;
        }
   }      
}

