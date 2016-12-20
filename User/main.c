/**
  ******************************************************************************
  * @file    main.c
  * @author  casic 203
  * @version V1.0
  * @date    2016-07-27
  * @brief   
  * @attention
  *
  ******************************************************************************
  */ 
#include "stm32f10x.h"
#include "stdlib.h"
#include "time.h"
#include "misc.h"
#include "stm32f10x_it.h"
#include "bsp_usart.h"
#include "bsp_TiMbase.h" 
#include "modbus.h"
#include "bsp_SysTick.h"
#include "string.h"
#include "gprs.h"
#include "bsp_rtc.h"
#include "bsp_date.h"
#include "WatchDog.h"
#include "AiderProtocol.h"
#include "SPI_Flash.h"
#include "common.h"
#include "DS2780.h"
#include "test.h"
#include "sensor-adc.h"
#include "SensorInit.h"


#define  COLLECT_NUM 10                                   //一次采集需要采集的数据   
#define  PREPARE_TIME 180                                 //传感器需要预热180S
#define  SENSOR_COLLECT_TIME 0.5                           //两组数据之间的采集间隔


struct    rtc_time             systmtime;               //RTC时钟设置结构体
uint32_t  Start_time;																		//传感器预热开始时间
uint32_t  End_time;																			//传感器预热结束时间

struct    Sensor_Set           DeviceConfig ={0x00};       //配置信息结构体
struct    SMS_Config_RegPara   ConfigData ={0x00};       //与FLASH交互的结构体，方便在系统进入休眠模式之前写入FLASH

uint16_t  WWDOG_Feed =0x1FFF;                  //窗口看门狗复位周期为：XX*1.8s = 7.6min

char   		SendData_Flag=0;                       //发送数据成功标志位，发送成为为1，不成功为0；
u8        ReadData_flag=0;                       //读取传感器成功标志位，成功为1，失败为0；

char      TcpConnect_Flag=0;                     //TCP连接成功标志位，连接成功为1，不成功为0；

char      PowerOffReset =0;                    //掉电重启标志位

u8        Send_Request_flag=0;                         //发送数据至上位机后收到反馈
uint8_t   LiquidDataSend_Flag =0;                 //数据发送完成标志位

uint8_t   SensorCollectNumber=0;                 //一次上报需要进行数据采集的数量         采集数量=上报周期/采集周期
uint8_t   DataCollectCount ;                 //数据采集计数器   每次初始化需要读取该值，采集完数据后写入FLASH

struct    SenserData   PerceptionData;         //传感器数据   “u16采集时间  float传感器数据    u8采集数量”

char      Usart1_recev_buff[300] ={'\0'};      //USART1接收缓存
uint16_t  Usart1_recev_count =0;               //USART1发送计数器

extern  char      Usart3_recev_buff[1000];
extern  uint16_t  Usart3_recev_count; 

extern  uint8_t   CSQ_OK ;                  //信号质量标志变量

        uint8_t   DMA_UART3_RECEV_FLAG =0;             //USART3 DMA接收标志变量
extern  uint8_t   DMA_USART3_RecevBuff[RECEIVEBUFF_SIZE];
extern  struct    DMA_USART3_RecevConfig   DMA_USART3_RecevIndicator; 

extern __IO uint16_t ADC1ConvertedValue;

/***********************************函数申明***************************************************/  
unsigned char  DMA_UART3_RecevDetect(unsigned char RecevFlag,u8* pDeviceID, u16 sNodeAddress);     //USART3接收数据监测与数据解析
void  PeripheralInit( void);          //初始化外围设备
void  DataMessageSend(void);      // 通过短信发送液位数据
int   DMA_UART3_RecevDataGet(void);     //获取串口3的DMA数据
float ReadSensorData(uint16_t data);    //将ADC采集的整数数据转换为小数

float Filter(float Original[],int num);     //对采集的数据进行滤波处理

void error_handle(u8* pDevID, u16 NodeAddr);  //异常处理函数
void Display(void);


/*******************************************************************************
* Function Name  : int  DMA_UART3_RecevDataGet(void)
* Description    : 从DMA接收存储器中提取有效数据，放入Usart3_recev_buff[],便于后续数据解析
* Input          : None
* Output         : None
* Return         : 接收数据长度
*******************************************************************************/
int  DMA_UART3_RecevDataGet(void)
{
   int i=0,j=0;
	 u16 DMA_RecevLength =0;
	
	 memset(Usart3_recev_buff, 0x00, sizeof(Usart3_recev_buff));
	 DMA_USART3_RecevIndicator.CurrentDataStartNum = DMA_USART3_RecevIndicator.NextDataStartNum ;
	  
	 i = RECEIVEBUFF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);
	 if(DMA_USART3_RecevIndicator.DMA_RecevCount <i)
	 {
     DMA_RecevLength =i -DMA_USART3_RecevIndicator.DMA_RecevCount;
   }
	 else
	 {
     DMA_RecevLength = RECEIVEBUFF_SIZE -DMA_USART3_RecevIndicator.DMA_RecevCount + i;
   }
   DMA_USART3_RecevIndicator.DMA_RecevCount = i;
	
	 if((DMA_USART3_RecevIndicator.CurrentDataStartNum + DMA_RecevLength-1) < RECEIVEBUFF_SIZE)
	 {
     DMA_USART3_RecevIndicator.CurrentDataEndNum =DMA_USART3_RecevIndicator.CurrentDataStartNum +DMA_RecevLength-1;     
   }
	 else
	 {
     DMA_USART3_RecevIndicator.CurrentDataEndNum =(DMA_USART3_RecevIndicator.CurrentDataStartNum +DMA_RecevLength-1) -RECEIVEBUFF_SIZE;  
   }
#if DEBUG_TEST 	 
	 printf("\r\nDMA UART2 Recev Data Start Num:%d----End Num: %d\r\n",DMA_USART3_RecevIndicator.CurrentDataStartNum,DMA_USART3_RecevIndicator.CurrentDataEndNum);    //数据起始位置与终止位置
#endif
	 if(DMA_USART3_RecevIndicator.CurrentDataEndNum ==(RECEIVEBUFF_SIZE-1))
	 {
	   DMA_USART3_RecevIndicator.NextDataStartNum = 0;
   }
	 else
	 {
		 DMA_USART3_RecevIndicator.NextDataStartNum = DMA_USART3_RecevIndicator.CurrentDataEndNum + 1;
   }	
   /*************************************Data Copy*********************************************************/
   if(DMA_RecevLength !=0)
	 {
     j =DMA_USART3_RecevIndicator.CurrentDataStartNum;
		 if(DMA_USART3_RecevIndicator.CurrentDataEndNum >DMA_USART3_RecevIndicator.CurrentDataStartNum)
		 {
			 for(i=0; i<DMA_RecevLength; i++,j++)
			 {
					Usart3_recev_buff[i] =DMA_USART3_RecevBuff[j];	
			 }
		 }
		 else
		 {
			 for(i=0; i<DMA_RecevLength; i++)
			 {
					if( j<(RECEIVEBUFF_SIZE-1) )
					{
						 Usart3_recev_buff[i] =DMA_USART3_RecevBuff[j];
						 j++;				
					}
					else if( j==(RECEIVEBUFF_SIZE-1) )
					{
						 Usart3_recev_buff[i] =DMA_USART3_RecevBuff[j];
						 j =0;				 
					}
			  } 
      }
    }
	  return DMA_RecevLength;
}
/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
unsigned char  DMA_UART3_RecevDetect(unsigned char RecevFlag,u8* pDeviceID, u16 sNodeAddress)
{
  
	int DataLength =0;
	int i=0;
	unsigned char  StateFlag =0;
	
  if(DMA_UART3_RECEV_FLAG==1)
  {
		 DataLength = DMA_UART3_RecevDataGet();
		 if(DataLength>0)
		 {
					  
			  for(i=0;i<DataLength;i++)
			  {
             printf(" %x ",Usart3_recev_buff[i]);               //测试使用 将串口3接收到的数据以16进制打印出来
        }
				
				if(RecevFlag ==NETLOGIN)
				{
           StateFlag =GPRS_Receive_NetLogin();      //上网注册状态检验，当返回为1时注册成功，返回为0时注册失败
        }
				else if(RecevFlag ==TCPCONNECT)
				{
           StateFlag =GPRS_Receive_TcpConnect();    //SOCKET连接: 当返回为1时SOCKET连接成功，返回为0时连接失败
        }	
        else if(RecevFlag ==DATARECEV )
				{
					 StateFlag =GPRS_Receive_DataAnalysis(pDeviceID, sNodeAddress);     //进行接收数据分析
        }	 				
		 }
		 else
		 {
        printf("\r\nNo data\r\n");
     }
		 memset(DMA_USART3_RecevBuff,0x00,RECEIVEBUFF_SIZE);   //复位DMA数据接收BUFF
     DMA_UART3_RECEV_FLAG =0;
		 
  } 
	return StateFlag;
}

/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  DataMessageSend(void)                 // 通过短信发送传感器数据
{ 
    char SensorDataInquire[50] ={0x00};
    unsigned char readdata[7];
    uint8_t i;
    uint8_t flash_num;                           //保证能够读出FLASH内的数据
    struct SenserData messagedata;
/////////////////////确定当前采集的数据所处位置，进行FLASH读取////////////////////////////////////
   if(DataCollectCount == 0)
	 {
		 flash_num = SensorCollectNumber;
    }
		else
		{
			flash_num = DataCollectCount;
     }			
    DataRead_From_Flash(1,flash_num,0, readdata ,sizeof(readdata)); 
    for(i=0;i<4;i++) 
    messagedata.Ch4Data .Data_Hex [i] = readdata[2+i];
	  snprintf( SensorDataInquire,sizeof(SensorDataInquire),"\r Current sensor data is: %0.2f !\r",    messagedata.Ch4Data .Data_Float );
		printf("\r\nSend to Phone: %s \r\n",SensorDataInquire);                        //将发送的短信内容显示出来
    //Sms_Send( SensorDataInquire );
		Delay_ms(2000);                              //延时2s	
    AlarmSMS_Send(SensorDataInquire );
    Delay_ms(1000);                              //延时2s	
}



/*******************************************************************************
* Function Name  : float ReadSensorData(uint16_t data)
* Description    : 将ADC采集的数据进行转换为FLOAT
* Input          : ADC采集的数据
* Output         : ADC采集的小数形式
* Return         : None
*******************************************************************************/
float ReadSensorData(uint16_t data)
{
	float ADC_Data;
	float Result_Data;
	#if DEBUG_TEST
	printf("\r\nThe Original ADC data is %d \r\n",data);          //读取ADC的原始值
	#endif
	ADC_Data = (float)data/4096*3.3;                          //电压的浮点形式
	Result_Data = ADC_Data * 40;                              //利用2.5V进行转换
	if(Result_Data<0)
		Result_Data=0;                                          //进行零漂处理
	else if(Result_Data>100.0)
		Result_Data=100.0;                                      //进行上限处理
	#if DEBUG_TEST	
	printf("\rThe Sensor Data is %0.2f\r\n",Result_Data);
	#endif
	return(Result_Data);
}
/*******************************************************************************
* Function Name  : 平均数法Filter
* Description    : 对传感器采集的数据进行滤波处理
* Input          : 采集的数据
* Output         : 滤波后的数据
* Return         : None
*******************************************************************************/
float Filter(float Original[],int num)
{
	float max=0;                      //最大的数
	float min=0;											//最小的数
	float sum=0;                      //总和
	float averge=0;                   //平均数输出
	int i;
	
	
	//判断输入参数
	if( num>10 )
		printf("\r\nTHE INPUT NUMBER IS MORE THAN 10,THIS FUNCTION WILL NOT WORK!\r\n");
	else if(num < 3)
		printf("\r\nTHE INPUT NUMBER IS LESS THAN 3,THIS FUNCTION WILL NOT WORK!\r\n");
	else
	{
		max=Original[0];
		min=Original[0];                       //进行初始化
		for(i=0;i<num;i++)
		{ 
			if(max<Original[i])
		  max=Original[i];
			if(min>Original[i])
			min=Original[i];
		}                                   //求出数组的最大项和最小项
		
		printf("\r\n THE MAX is %0.2f, The MIN is %0.2f",max,min);
	 if(max-min>8.0)                       //进行数据筛选
	 {
		 for(i=1;i<num-1;i++)
		 sum=sum+Original[i];
		 averge=sum /(num-2);                //去掉最大值和最小值 
		 
   }
	 else
	 {
		 for(i=0;i<num;i++)
		 sum=sum+Original[i];
		 averge=sum /num;                //所有数据的平均数
	 }
	printf("\r\nTHE averge is %0.2f\r\n",averge);	
		return averge;
		
		
	}
}


/////////////////////////传感器采集数据////////////////////////////////////////////
void SenserDataCollect(struct SenserData* pGetData, u8* pDevID, u16 NodeAddr)
{
      
			u32       TimCount_Current =0;
      u32				Prepare_time;                   //传感器预热时间
	    char      Alarm_Flag=0;
	    struct TagStruct AlarmTAG;                //报警结构体
	    uint8_t   sensordata[7]={0x00};               //将传感器数据写入FLASH
   		u8        i=0;   //测试使用
      float     result[COLLECT_NUM];
			TimCount_Current = RTC_GetCounter();
      End_time=TimCount_Current;                //开始预热
			
		  Prepare_time=End_time -Start_time;                           //计算预热时间
#if DEBUG_TEST
		  printf("\r\nTHE SENSOR HAS WORKING %d S!\r\n",Prepare_time);   //输出准备时间
#endif 
		if(Prepare_time< PREPARE_TIME)
		{
#if DEBUG_TEST			
			printf("\r\nTHE SENSOR NEED WAIT  180 S BEFORE COLLECT DATA!\r\n");   //传感器预热时间
#endif
      Delay_ms( (PREPARE_TIME - Prepare_time)*1000 );              //进行延迟，以达到准备时间
			
		}

			for(i=0;i<COLLECT_NUM;i++)
			{
				result[i]=ReadSensorData(ADC1ConvertedValue);                     //每间隔1秒进行采集
				printf("\r\nNO:%d  sensor_collect: %0.2f",i,result[i]);            //将采集的数据进行打印
				
				Delay_ms( (SENSOR_COLLECT_TIME) *1000);                                                    //间隔1S进行采数
			}
		
			PowerOFF_Sensor();                  //当数据采集完成时，关闭超声波探头电源
		
    	pGetData->Ch4Data.Data_Float  = Filter(result,COLLECT_NUM);   
	
      pGetData->CollectTime =(DeviceConfig.Time_Hour)*60 +(DeviceConfig.Time_Min);      //第一组数据采集时间
#if DEBUG_TEST    
			printf("\r\npGetData->CollectTime:%d\r\n",pGetData->CollectTime);
#endif			
			pGetData->DataCount =  DataCollectCount+1;                       //当前采集的个数
			Delay_ms(500);
      
///////////////////////////////////////////////////////////////////////////////////////////////////////////////		
      DataCollectCount = pGetData->DataCount;
			ConfigData.CollectNum = DataCollectCount;
			DataWrite_To_Flash(0,9,0,&ConfigData.CollectNum,1);
////////////////////////////////将采集的传感器数据写入FLASH///////////////////////////////////////////////////
			sensordata[0]= pGetData->CollectTime >> 8;
			sensordata[1]= pGetData->CollectTime & 0xff;
      for(i=0;i<4;i++)
			{
      sensordata[2+i] =	pGetData->Ch4Data.Data_Hex[i];
			}
      sensordata[6] = pGetData->DataCount;			
      DataWrite_To_Flash(1,pGetData->DataCount,0,(uint8_t*)sensordata,sizeof(sensordata));       //数据从Sector1开始写入
			
			ReadData_flag=1;                                                                        //采集数据完成
#if DEBUG_TEST
			printf("\r\nthe sensor data already send NO.%d to flash!\r\n",pGetData->DataCount);
#endif
//////////////如果监测的气体浓度达到了报警阈值，立即报警，无需等到上报时间的到来///////////////////////		
	
			if((pGetData->Ch4Data.Data_Float >=DeviceConfig.HighAlarmLevel .Data_Float) ||(pGetData->Ch4Data.Data_Float>=DeviceConfig.LowAlarmLevel .Data_Float)  )
			
			{
			 //DataMessageSend();		                     //短信发送	,目前注释掉
		    Alarm_Flag=1;                              //报警信号
				TCP_Connect();                                //重新建立TCP连接
				
				
				AlarmTAG.OID_Command=(DeviceConfig.CollectPeriod<<11)+pGetData->CollectTime  +((0xC0 + REPORTDATA )<<24);
				AlarmTAG.OID_Command=ntohl(AlarmTAG.OID_Command);
				AlarmTAG.Width=4;
				for(i=0;i<AlarmTAG.Width;i++)
				AlarmTAG.Value[i]=pGetData->Ch4Data.Data_Hex[i];                                 //组成报警的TAG
				SendDataToServ(TRAPREQUEST,&AlarmTAG,1,pDevID);                //433模块主动上传数据
		    //AlarmTrap(Usart3_send_buff, pDevID, NodeAddr, &PerceptionData);             //3G模块主动上传数据
		    Send_Request_flag = DMA_UART3_RecevDetect(DATARECEV,pDevID, NodeAddr);       //每隔5秒发送一次，共发送3次，收到数据时
			  if(Send_Request_flag == 1)
		     {
		      Send_Request_flag = 0;	
		   	 }
				 
				TCP_Disconnect();                      //配置下发完成，断开TCP连接
				LiquidDataSend_Flag=1;                 //液位数据成功发送到服务器标志变量
			 
     }		
///////////////////////////////////////////////////////////////////////////////////////////////////////			

			//printf("\r\nthe sensor end collect data! waiting for send!\r\n");                       //测试显示
			
			 if(DataCollectCount  < SensorCollectNumber)
			 {    
				 Sms_Consult(pDevID, NodeAddr );   
				 if(Alarm_Flag==0)
				 {  SendData_Flag=1; 	}	                         //没有报警或者达到上报时间设置发送成功
				 else
				 {
         //SendData_Flag=0; 
         }                          //有报警信号时发送不成功
				 gotoSleep(DeviceConfig.CollectPeriod  ); 
        }                                       //如果未达到上报时间，以采集时间睡眠 
			 else
				 
				if( DataCollectCount  == SensorCollectNumber)          //采集完毕，进行数据上报 ;当采集次数到达时，先判断是否阈值上报过一次
				{

				Sms_Consult(pDevID, NodeAddr );                                //查阅短信，更新配置参数，便于需要时及时上传数据
			  Delay_ms(100); 
       	DeviceConfig.BatteryCapacity = DS2780_Test(); //监测电池电量

//////////////////////////////////////////////////////////////////////				
				if(ConfigData.SensorDataInquireFlag  ==1)
				{
					ConfigData.SensorDataInquireFlag =0;
					//DataMessageSend();                   //目前不短信发送
				}
	
				TCP_Connect();                                //重新建立TCP连接
			
        TrapData(pDevID);  
									
		    Send_Request_flag = DMA_UART3_RecevDetect(DATARECEV,pDevID, NodeAddr);                //每隔5秒发送一次，共发送5次，收到数据时断开TCP连接
		    if(Send_Request_flag == 1)
		     {
		      Send_Request_flag = 0;	
		   	 }
////////////////////////////////////////////////////////////////////////////////////////////////////
				//Delay_ms(1000); 
			
				TCP_Disconnect();                      //配置下发完成，断开TCP连接
				LiquidDataSend_Flag=1;                 //数据成功发送到服务器标志变量

			  gotoSleep(DeviceConfig.CollectPeriod); 
        
			}
}

/*******************************************************************************
* Function Name  : error_handle(void)
* Description    : 异常处理
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void error_handle(u8* pDevID, u16 NodeAddr)
{
	u8 state[2];
	//u8 readdata[7];
	
	int i;
	//************************************首先判断上次工作状态，采数和发送********************************************************
   DataRead_From_Flash(2,1,0, state ,2);            //读取采数状态和发送状态
   
		if(state[0] ==0 )                                                   //判断是否采集数据，如采集，进入下一步，否则进行采集，采集后睡眠
	 {
		 printf("\r\nTHE DEVICE HAS NOT COLLECTED DATA! \r\n");
		 	if(DataCollectCount < SensorCollectNumber)   //当已采集的数据小于共采集数量
			{   
				printf("\r\nThe Sensor Has DataCollectCount is %d \r\n",DataCollectCount);      //
		    
				SenserDataCollect(&PerceptionData, pDevID, NodeAddr);        //获取传感器数据
      }
			else                                                              //采集完成后，完成数据的备份
			{	
				if(DataCollectCount == SensorCollectNumber)         //完成数据采集后，开始上传数据
				//printf("The Sensor Flash DataCollectCount is %d ",DataCollectCount);      //
				DataCollectCount=0;                                   //对数据进行清零，重新采集
				printf("\r\nThe Sensor Has DataCollectCount is %d \r\n",DataCollectCount);      //
				SenserDataCollect(&PerceptionData, pDevID, NodeAddr);        //获取传感器数据
	    }
 }
	 else
	 {
		 printf("\r\nTHE DEVICE COLLECT DONE!\r\n");
	 }

	 if(state[1] ==0)                                             									//判断是否进行上报成功，继续上报
	 {
		   printf("\r\nTHE DEVICE HAS NOT SENT DATA! \r\n");
			PowerOFF_Sensor();                  //当数据采集完成时，关闭超声波探头电源	
		 if( DataCollectCount  == SensorCollectNumber)          //采集完毕，进行数据上报 ;当采集次数到达时，先判断是否阈值上报过一次
				{

				//Sms_Consult(pDevID, NodeAddr );                                //查阅短信，更新配置参数，便于需要时及时上传液位数据
				PowerOFF_Sensor();                  //当液位数据采集完成时，关闭超声波探头电源
				
				TCP_Connect();                                //重新建立TCP连接
			
			  	for(i=0;i<2 ;i++)
	       {
		      
		       TrapData(pDevID);      //3G模块主动上传数据
		      //Send_Request_flag = DMA_UART3_RecevDetect(DATARECEV,pDevID, NodeAddr);                //每隔5秒发送一次，共发送5次，收到数据时断开TCP连接
		      if(SendData_Flag == 1)
		       {
		      //Send_Request_flag = 0;	
		      break;
		       }
					 Delay_ms(5000);
	       }
////////////////////////////////////////////////////////////////////////////////////////////////////
				//Delay_ms(1000); 
			
				TCP_Disconnect();                      //配置下发完成，断开TCP连接
				LiquidDataSend_Flag=1;                 //液位数据成功发送到服务器标志变量
			 
        }
		 else
		 {
		   
			
				TCP_Connect();                                //重新建立TCP连接
			

				TCP_Disconnect();                      //配置下发完成，断开TCP连接
		 
			 
				}
		 
		}
		else
		{
			 printf("\r\nTHE DEVICE SEND DONE!\r\n");
		}
	//gotoSleep(DeviceConfig.CollectPeriod); 
}

/*******************************************************************************
* Function Name  : int main(void)
* Description    : 主函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main( void )
{
	u8   i=0,j=0;
	u8   DeviceID[6] =SENSORID; 											//初始化设备ID号
  u16  NodeAddr =0x0000;                            //提取设备ID号最后面两个字节作为节点地址
  
  NodeAddr=DeviceID[4]*256 +DeviceID[5];
	PeripheralInit();                            //初始化
	
  //***********************************对采集数量进行处理,一次最多传15个数*****************************************************
	SensorCollectNumber = DeviceConfig.SendCount / DeviceConfig.CollectPeriod ;
	if(SensorCollectNumber>15)
	{ SensorCollectNumber=15;	}
  
   DataRead_From_Flash(0,9,0, &(DeviceConfig.CollectNum ) ,1);            //读取已采集的数据量 
   DataCollectCount = DeviceConfig.CollectNum ;
   if(DataCollectCount > SensorCollectNumber)                     //如果已采集的数量大于需要采集的数量，将已采集数量归零
	 {
		 DataCollectCount=0;
	 }
#if DEBUG_TEST	
	printf("\r\n The Data Collect Number is %d \r\n",DeviceConfig.CollectNum );
#endif
	 
	 //error_handle(DeviceID, NodeAddr);                 //异常处理函数

   Sms_Consult(DeviceID, NodeAddr);  
	 TCP_Connect();                                //重新建立TCP连接
	 
  SendDataToServ(STARTUPREQUEST,NULL,0,DeviceID);  //开机上报信息     
	Delay_ms(1000);  
	 
	TCP_Disconnect();                  //配置下发完成，断开TCP连接

	while(1)
  {
 
		  WWDOG_Feed =0x1FFF;                            //窗口看门狗喂狗,定时4分20秒，第二字节约1秒变化一次，即0x09AF变到0x099F约耗时1秒

 	    for(i=0;i<5;i++)
		{
			 Delay_ms(200);                                //参考探头上电后数据稳定时间，作相应调整
			 if(DMA_UART3_RECEV_FLAG==1)                   //查询数据接收情况
			 {
				 DMA_UART3_RecevDetect(DATARECEV,DeviceID, NodeAddr);   
				 break;
			 }
    }	
		 // Sms_Consult(DeviceID, NodeAddr);  
	
			if(DataCollectCount < SensorCollectNumber)   //当已采集的数据小于共采集数量
			{  
#if DEBUG_TEST				
				printf("\r\nThe Sensor Has DataCollectCount is %d \r\n",DataCollectCount); 
#endif				
		
				SenserDataCollect(&PerceptionData, DeviceID, NodeAddr);        //获取传感器数据
      }
			else                                                              //采集完成后，完成数据的备份
			{	
				if(DataCollectCount == SensorCollectNumber)         //完成数据采集后，开始上传数据
				//printf("The Sensor Flash DataCollectCount is %d ",DataCollectCount);      //
				DataCollectCount=0;                                   //对数据进行清零，重新采集
		
		  }
			if(LiquidDataSend_Flag ==1)            
			{ 
				gotoSleep(DeviceConfig.CollectPeriod );
				//Sms_Consult(DeviceID, NodeAddr);                 //目前注释
			}
		
	}

  
}


/*******************************************************************************
* Function Name  : void PeripheralInit( void )
* Description    : 初始化端口及外设
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PeripheralInit( void )
{

	WWDG_Init(0x7F,0X5F,WWDG_Prescaler_8); 	//首先开启窗口看门狗，计数器值为7f,窗口寄存器为5f,分频数为8	
	SysTick_Init();         //初始化时钟      
	USART1_Config();      /* USART1 配置模式为 9600 8-N-1，  中断接收 */    //用于串口调试助手
	Delay_ms(200);
//	USART2_Config();      /* USART2 配置模式为 9600 8-N-1，中断接收 */    //用于433通信方式
	USART3_Config();      /* USART3 配置模式为 115200 8-N-1，中断接收 */    //用于3G模块通信方式   
	UART_NVIC_Configuration();
	USART3_DMA_Config();
	
	RTC_NVIC_Config();                 /* 配置RTC秒中断优先级 */
	RTC_CheckAndConfig(&systmtime);

	ADCconfig();                       //进行ADC初始化
	
	PowerON_GPRS();                    //打开GPRS模块电源  3.8V	
  PowerON_Sensor();         //打开传感器电压  12V
	Delay_ms(500);
	Start_time=RTC_GetCounter();             //记录传感器打开时间
	Display();
	ConfigData_Init(&DeviceConfig);
 
  if (PowerOffReset ==1)     
  {
    printf("\r\n掉电重启，重新初始化库仑计\r\n");                 //测试使用
		DS2780_CapacityInit();    //掉电后重新写电池容量
		Set_register_ds2780();    //掉电后对库仑计重新初始化
	  set_ACR(1000);           //掉电后对库仑计重新初始化

  }
	
	//GPRS_Init();             //暂时屏蔽
	GPRS_Config();
	Delay_ms(5000);            //等待短信接收   
	//Sms_Consult(pDeviceID, sNodeAddress);
}

/*******************************************************************************
* Function Name  : void Display( void )
* Description    : 打印出厂信息
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Display(void)
{
	u32       TimCount_Current =0;
	u8   DeviceID[6] = SENSORID ; 
	TimCount_Current = RTC_GetCounter();
	printf("\r\n********* 北京航天科瑞电子有限公司 *********");
	printf("\r\n************* 燃气智能监测终端 ***********");
	printf("\r\n************* BIRMM-RTU100 ***********");
	printf("\r\n************* 设备ID号：%x %x %x %x %x %x***********",DeviceID[0],DeviceID[1],DeviceID[2],DeviceID[3],DeviceID[4],DeviceID[5]);
	printf("\r\n************** 硬件类型: 433版本 **********");
	printf("\r\n************** 硬件版本号: %x ***********",HARDVERSION);
	printf("\r\n************** 软件版本号: %x **************\r\n\r\n",SOFTVERSION);
	
	Time_Display(TimCount_Current,&systmtime); 
	DeviceConfig.Time_Sec  =systmtime.tm_sec;
	DeviceConfig.Time_Min  =systmtime.tm_min;
	DeviceConfig.Time_Hour =systmtime.tm_hour;
	DeviceConfig.Time_Mday =systmtime.tm_mday;		
	DeviceConfig.Time_Mon  =systmtime.tm_mon;
	DeviceConfig.Time_Year =systmtime.tm_year-2000; //对上传年份去基数修正				
		    

printf("**************当前时间:%0.4d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d***********\r\n",systmtime.tm_year,systmtime.tm_mon,     //测试使用
				systmtime.tm_mday,systmtime.tm_hour,systmtime.tm_min,systmtime.tm_sec);     //测试使用
  Delay_ms(3000);
}

#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number */
 
  printf("\n\r Wrong parameter value detected on\r\n");
  printf("       file  %s\r\n", file);
  printf("       line  %d\r\n", line);
    
  /* Infinite loop */
  /* while (1)
  {
  } */
}
#endif
/*********************************************END OF FILE**********************/
