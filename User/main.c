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


#define  COLLECT_NUM 10                                   //һ�βɼ���Ҫ�ɼ�������   
#define  PREPARE_TIME 180                                 //��������ҪԤ��180S
#define  SENSOR_COLLECT_TIME 0.5                           //��������֮��Ĳɼ����


struct    rtc_time             systmtime;               //RTCʱ�����ýṹ��
uint32_t  Start_time;																		//������Ԥ�ȿ�ʼʱ��
uint32_t  End_time;																			//������Ԥ�Ƚ���ʱ��

struct    Sensor_Set           DeviceConfig ={0x00};       //������Ϣ�ṹ��
struct    SMS_Config_RegPara   ConfigData ={0x00};       //��FLASH�����Ľṹ�壬������ϵͳ��������ģʽ֮ǰд��FLASH

uint16_t  WWDOG_Feed =0x1FFF;                  //���ڿ��Ź���λ����Ϊ��XX*1.8s = 7.6min

char   		SendData_Flag=0;                       //�������ݳɹ���־λ�����ͳ�ΪΪ1�����ɹ�Ϊ0��
u8        ReadData_flag=0;                       //��ȡ�������ɹ���־λ���ɹ�Ϊ1��ʧ��Ϊ0��

char      TcpConnect_Flag=0;                     //TCP���ӳɹ���־λ�����ӳɹ�Ϊ1�����ɹ�Ϊ0��

char      PowerOffReset =0;                    //����������־λ

u8        Send_Request_flag=0;                         //������������λ�����յ�����
uint8_t   LiquidDataSend_Flag =0;                 //���ݷ�����ɱ�־λ

uint8_t   SensorCollectNumber=0;                 //һ���ϱ���Ҫ�������ݲɼ�������         �ɼ�����=�ϱ�����/�ɼ�����
uint8_t   DataCollectCount ;                 //���ݲɼ�������   ÿ�γ�ʼ����Ҫ��ȡ��ֵ���ɼ������ݺ�д��FLASH

struct    SenserData   PerceptionData;         //����������   ��u16�ɼ�ʱ��  float����������    u8�ɼ�������

char      Usart1_recev_buff[300] ={'\0'};      //USART1���ջ���
uint16_t  Usart1_recev_count =0;               //USART1���ͼ�����

extern  char      Usart3_recev_buff[1000];
extern  uint16_t  Usart3_recev_count; 

extern  uint8_t   CSQ_OK ;                  //�ź�������־����

        uint8_t   DMA_UART3_RECEV_FLAG =0;             //USART3 DMA���ձ�־����
extern  uint8_t   DMA_USART3_RecevBuff[RECEIVEBUFF_SIZE];
extern  struct    DMA_USART3_RecevConfig   DMA_USART3_RecevIndicator; 

extern __IO uint16_t ADC1ConvertedValue;

/***********************************��������***************************************************/  
unsigned char  DMA_UART3_RecevDetect(unsigned char RecevFlag,u8* pDeviceID, u16 sNodeAddress);     //USART3�������ݼ�������ݽ���
void  PeripheralInit( void);          //��ʼ����Χ�豸
void  DataMessageSend(void);      // ͨ�����ŷ���Һλ����
int   DMA_UART3_RecevDataGet(void);     //��ȡ����3��DMA����
float ReadSensorData(uint16_t data);    //��ADC�ɼ�����������ת��ΪС��

float Filter(float Original[],int num);     //�Բɼ������ݽ����˲�����

void error_handle(u8* pDevID, u16 NodeAddr);  //�쳣������
void Display(void);


/*******************************************************************************
* Function Name  : int  DMA_UART3_RecevDataGet(void)
* Description    : ��DMA���մ洢������ȡ��Ч���ݣ�����Usart3_recev_buff[],���ں������ݽ���
* Input          : None
* Output         : None
* Return         : �������ݳ���
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
	 printf("\r\nDMA UART2 Recev Data Start Num:%d----End Num: %d\r\n",DMA_USART3_RecevIndicator.CurrentDataStartNum,DMA_USART3_RecevIndicator.CurrentDataEndNum);    //������ʼλ������ֹλ��
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
             printf(" %x ",Usart3_recev_buff[i]);               //����ʹ�� ������3���յ���������16���ƴ�ӡ����
        }
				
				if(RecevFlag ==NETLOGIN)
				{
           StateFlag =GPRS_Receive_NetLogin();      //����ע��״̬���飬������Ϊ1ʱע��ɹ�������Ϊ0ʱע��ʧ��
        }
				else if(RecevFlag ==TCPCONNECT)
				{
           StateFlag =GPRS_Receive_TcpConnect();    //SOCKET����: ������Ϊ1ʱSOCKET���ӳɹ�������Ϊ0ʱ����ʧ��
        }	
        else if(RecevFlag ==DATARECEV )
				{
					 StateFlag =GPRS_Receive_DataAnalysis(pDeviceID, sNodeAddress);     //���н������ݷ���
        }	 				
		 }
		 else
		 {
        printf("\r\nNo data\r\n");
     }
		 memset(DMA_USART3_RecevBuff,0x00,RECEIVEBUFF_SIZE);   //��λDMA���ݽ���BUFF
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
void  DataMessageSend(void)                 // ͨ�����ŷ��ʹ���������
{ 
    char SensorDataInquire[50] ={0x00};
    unsigned char readdata[7];
    uint8_t i;
    uint8_t flash_num;                           //��֤�ܹ�����FLASH�ڵ�����
    struct SenserData messagedata;
/////////////////////ȷ����ǰ�ɼ�����������λ�ã�����FLASH��ȡ////////////////////////////////////
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
		printf("\r\nSend to Phone: %s \r\n",SensorDataInquire);                        //�����͵Ķ���������ʾ����
    //Sms_Send( SensorDataInquire );
		Delay_ms(2000);                              //��ʱ2s	
    AlarmSMS_Send(SensorDataInquire );
    Delay_ms(1000);                              //��ʱ2s	
}



/*******************************************************************************
* Function Name  : float ReadSensorData(uint16_t data)
* Description    : ��ADC�ɼ������ݽ���ת��ΪFLOAT
* Input          : ADC�ɼ�������
* Output         : ADC�ɼ���С����ʽ
* Return         : None
*******************************************************************************/
float ReadSensorData(uint16_t data)
{
	float ADC_Data;
	float Result_Data;
	#if DEBUG_TEST
	printf("\r\nThe Original ADC data is %d \r\n",data);          //��ȡADC��ԭʼֵ
	#endif
	ADC_Data = (float)data/4096*3.3;                          //��ѹ�ĸ�����ʽ
	Result_Data = ADC_Data * 40;                              //����2.5V����ת��
	if(Result_Data<0)
		Result_Data=0;                                          //������Ư����
	else if(Result_Data>100.0)
		Result_Data=100.0;                                      //�������޴���
	#if DEBUG_TEST	
	printf("\rThe Sensor Data is %0.2f\r\n",Result_Data);
	#endif
	return(Result_Data);
}
/*******************************************************************************
* Function Name  : ƽ������Filter
* Description    : �Դ������ɼ������ݽ����˲�����
* Input          : �ɼ�������
* Output         : �˲��������
* Return         : None
*******************************************************************************/
float Filter(float Original[],int num)
{
	float max=0;                      //������
	float min=0;											//��С����
	float sum=0;                      //�ܺ�
	float averge=0;                   //ƽ�������
	int i;
	
	
	//�ж��������
	if( num>10 )
		printf("\r\nTHE INPUT NUMBER IS MORE THAN 10,THIS FUNCTION WILL NOT WORK!\r\n");
	else if(num < 3)
		printf("\r\nTHE INPUT NUMBER IS LESS THAN 3,THIS FUNCTION WILL NOT WORK!\r\n");
	else
	{
		max=Original[0];
		min=Original[0];                       //���г�ʼ��
		for(i=0;i<num;i++)
		{ 
			if(max<Original[i])
		  max=Original[i];
			if(min>Original[i])
			min=Original[i];
		}                                   //����������������С��
		
		printf("\r\n THE MAX is %0.2f, The MIN is %0.2f",max,min);
	 if(max-min>8.0)                       //��������ɸѡ
	 {
		 for(i=1;i<num-1;i++)
		 sum=sum+Original[i];
		 averge=sum /(num-2);                //ȥ�����ֵ����Сֵ 
		 
   }
	 else
	 {
		 for(i=0;i<num;i++)
		 sum=sum+Original[i];
		 averge=sum /num;                //�������ݵ�ƽ����
	 }
	printf("\r\nTHE averge is %0.2f\r\n",averge);	
		return averge;
		
		
	}
}


/////////////////////////�������ɼ�����////////////////////////////////////////////
void SenserDataCollect(struct SenserData* pGetData, u8* pDevID, u16 NodeAddr)
{
      
			u32       TimCount_Current =0;
      u32				Prepare_time;                   //������Ԥ��ʱ��
	    char      Alarm_Flag=0;
	    struct TagStruct AlarmTAG;                //�����ṹ��
	    uint8_t   sensordata[7]={0x00};               //������������д��FLASH
   		u8        i=0;   //����ʹ��
      float     result[COLLECT_NUM];
			TimCount_Current = RTC_GetCounter();
      End_time=TimCount_Current;                //��ʼԤ��
			
		  Prepare_time=End_time -Start_time;                           //����Ԥ��ʱ��
#if DEBUG_TEST
		  printf("\r\nTHE SENSOR HAS WORKING %d S!\r\n",Prepare_time);   //���׼��ʱ��
#endif 
		if(Prepare_time< PREPARE_TIME)
		{
#if DEBUG_TEST			
			printf("\r\nTHE SENSOR NEED WAIT  180 S BEFORE COLLECT DATA!\r\n");   //������Ԥ��ʱ��
#endif
      Delay_ms( (PREPARE_TIME - Prepare_time)*1000 );              //�����ӳ٣��Դﵽ׼��ʱ��
			
		}

			for(i=0;i<COLLECT_NUM;i++)
			{
				result[i]=ReadSensorData(ADC1ConvertedValue);                     //ÿ���1����вɼ�
				printf("\r\nNO:%d  sensor_collect: %0.2f",i,result[i]);            //���ɼ������ݽ��д�ӡ
				
				Delay_ms( (SENSOR_COLLECT_TIME) *1000);                                                    //���1S���в���
			}
		
			PowerOFF_Sensor();                  //�����ݲɼ����ʱ���رճ�����̽ͷ��Դ
		
    	pGetData->Ch4Data.Data_Float  = Filter(result,COLLECT_NUM);   
	
      pGetData->CollectTime =(DeviceConfig.Time_Hour)*60 +(DeviceConfig.Time_Min);      //��һ�����ݲɼ�ʱ��
#if DEBUG_TEST    
			printf("\r\npGetData->CollectTime:%d\r\n",pGetData->CollectTime);
#endif			
			pGetData->DataCount =  DataCollectCount+1;                       //��ǰ�ɼ��ĸ���
			Delay_ms(500);
      
///////////////////////////////////////////////////////////////////////////////////////////////////////////////		
      DataCollectCount = pGetData->DataCount;
			ConfigData.CollectNum = DataCollectCount;
			DataWrite_To_Flash(0,9,0,&ConfigData.CollectNum,1);
////////////////////////////////���ɼ��Ĵ���������д��FLASH///////////////////////////////////////////////////
			sensordata[0]= pGetData->CollectTime >> 8;
			sensordata[1]= pGetData->CollectTime & 0xff;
      for(i=0;i<4;i++)
			{
      sensordata[2+i] =	pGetData->Ch4Data.Data_Hex[i];
			}
      sensordata[6] = pGetData->DataCount;			
      DataWrite_To_Flash(1,pGetData->DataCount,0,(uint8_t*)sensordata,sizeof(sensordata));       //���ݴ�Sector1��ʼд��
			
			ReadData_flag=1;                                                                        //�ɼ��������
#if DEBUG_TEST
			printf("\r\nthe sensor data already send NO.%d to flash!\r\n",pGetData->DataCount);
#endif
//////////////�����������Ũ�ȴﵽ�˱�����ֵ����������������ȵ��ϱ�ʱ��ĵ���///////////////////////		
	
			if((pGetData->Ch4Data.Data_Float >=DeviceConfig.HighAlarmLevel .Data_Float) ||(pGetData->Ch4Data.Data_Float>=DeviceConfig.LowAlarmLevel .Data_Float)  )
			
			{
			 //DataMessageSend();		                     //���ŷ���	,Ŀǰע�͵�
		    Alarm_Flag=1;                              //�����ź�
				TCP_Connect();                                //���½���TCP����
				
				
				AlarmTAG.OID_Command=(DeviceConfig.CollectPeriod<<11)+pGetData->CollectTime  +((0xC0 + REPORTDATA )<<24);
				AlarmTAG.OID_Command=ntohl(AlarmTAG.OID_Command);
				AlarmTAG.Width=4;
				for(i=0;i<AlarmTAG.Width;i++)
				AlarmTAG.Value[i]=pGetData->Ch4Data.Data_Hex[i];                                 //��ɱ�����TAG
				SendDataToServ(TRAPREQUEST,&AlarmTAG,1,pDevID);                //433ģ�������ϴ�����
		    //AlarmTrap(Usart3_send_buff, pDevID, NodeAddr, &PerceptionData);             //3Gģ�������ϴ�����
		    Send_Request_flag = DMA_UART3_RecevDetect(DATARECEV,pDevID, NodeAddr);       //ÿ��5�뷢��һ�Σ�������3�Σ��յ�����ʱ
			  if(Send_Request_flag == 1)
		     {
		      Send_Request_flag = 0;	
		   	 }
				 
				TCP_Disconnect();                      //�����·���ɣ��Ͽ�TCP����
				LiquidDataSend_Flag=1;                 //Һλ���ݳɹ����͵���������־����
			 
     }		
///////////////////////////////////////////////////////////////////////////////////////////////////////			

			//printf("\r\nthe sensor end collect data! waiting for send!\r\n");                       //������ʾ
			
			 if(DataCollectCount  < SensorCollectNumber)
			 {    
				 Sms_Consult(pDevID, NodeAddr );   
				 if(Alarm_Flag==0)
				 {  SendData_Flag=1; 	}	                         //û�б������ߴﵽ�ϱ�ʱ�����÷��ͳɹ�
				 else
				 {
         //SendData_Flag=0; 
         }                          //�б����ź�ʱ���Ͳ��ɹ�
				 gotoSleep(DeviceConfig.CollectPeriod  ); 
        }                                       //���δ�ﵽ�ϱ�ʱ�䣬�Բɼ�ʱ��˯�� 
			 else
				 
				if( DataCollectCount  == SensorCollectNumber)          //�ɼ���ϣ����������ϱ� ;���ɼ���������ʱ�����ж��Ƿ���ֵ�ϱ���һ��
				{

				Sms_Consult(pDevID, NodeAddr );                                //���Ķ��ţ��������ò�����������Ҫʱ��ʱ�ϴ�����
			  Delay_ms(100); 
       	DeviceConfig.BatteryCapacity = DS2780_Test(); //����ص���

//////////////////////////////////////////////////////////////////////				
				if(ConfigData.SensorDataInquireFlag  ==1)
				{
					ConfigData.SensorDataInquireFlag =0;
					//DataMessageSend();                   //Ŀǰ�����ŷ���
				}
	
				TCP_Connect();                                //���½���TCP����
			
        TrapData(pDevID);  
									
		    Send_Request_flag = DMA_UART3_RecevDetect(DATARECEV,pDevID, NodeAddr);                //ÿ��5�뷢��һ�Σ�������5�Σ��յ�����ʱ�Ͽ�TCP����
		    if(Send_Request_flag == 1)
		     {
		      Send_Request_flag = 0;	
		   	 }
////////////////////////////////////////////////////////////////////////////////////////////////////
				//Delay_ms(1000); 
			
				TCP_Disconnect();                      //�����·���ɣ��Ͽ�TCP����
				LiquidDataSend_Flag=1;                 //���ݳɹ����͵���������־����

			  gotoSleep(DeviceConfig.CollectPeriod); 
        
			}
}

/*******************************************************************************
* Function Name  : error_handle(void)
* Description    : �쳣����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void error_handle(u8* pDevID, u16 NodeAddr)
{
	u8 state[2];
	//u8 readdata[7];
	
	int i;
	//************************************�����ж��ϴι���״̬�������ͷ���********************************************************
   DataRead_From_Flash(2,1,0, state ,2);            //��ȡ����״̬�ͷ���״̬
   
		if(state[0] ==0 )                                                   //�ж��Ƿ�ɼ����ݣ���ɼ���������һ����������вɼ����ɼ���˯��
	 {
		 printf("\r\nTHE DEVICE HAS NOT COLLECTED DATA! \r\n");
		 	if(DataCollectCount < SensorCollectNumber)   //���Ѳɼ�������С�ڹ��ɼ�����
			{   
				printf("\r\nThe Sensor Has DataCollectCount is %d \r\n",DataCollectCount);      //
		    
				SenserDataCollect(&PerceptionData, pDevID, NodeAddr);        //��ȡ����������
      }
			else                                                              //�ɼ���ɺ�������ݵı���
			{	
				if(DataCollectCount == SensorCollectNumber)         //������ݲɼ��󣬿�ʼ�ϴ�����
				//printf("The Sensor Flash DataCollectCount is %d ",DataCollectCount);      //
				DataCollectCount=0;                                   //�����ݽ������㣬���²ɼ�
				printf("\r\nThe Sensor Has DataCollectCount is %d \r\n",DataCollectCount);      //
				SenserDataCollect(&PerceptionData, pDevID, NodeAddr);        //��ȡ����������
	    }
 }
	 else
	 {
		 printf("\r\nTHE DEVICE COLLECT DONE!\r\n");
	 }

	 if(state[1] ==0)                                             									//�ж��Ƿ�����ϱ��ɹ��������ϱ�
	 {
		   printf("\r\nTHE DEVICE HAS NOT SENT DATA! \r\n");
			PowerOFF_Sensor();                  //�����ݲɼ����ʱ���رճ�����̽ͷ��Դ	
		 if( DataCollectCount  == SensorCollectNumber)          //�ɼ���ϣ����������ϱ� ;���ɼ���������ʱ�����ж��Ƿ���ֵ�ϱ���һ��
				{

				//Sms_Consult(pDevID, NodeAddr );                                //���Ķ��ţ��������ò�����������Ҫʱ��ʱ�ϴ�Һλ����
				PowerOFF_Sensor();                  //��Һλ���ݲɼ����ʱ���رճ�����̽ͷ��Դ
				
				TCP_Connect();                                //���½���TCP����
			
			  	for(i=0;i<2 ;i++)
	       {
		      
		       TrapData(pDevID);      //3Gģ�������ϴ�����
		      //Send_Request_flag = DMA_UART3_RecevDetect(DATARECEV,pDevID, NodeAddr);                //ÿ��5�뷢��һ�Σ�������5�Σ��յ�����ʱ�Ͽ�TCP����
		      if(SendData_Flag == 1)
		       {
		      //Send_Request_flag = 0;	
		      break;
		       }
					 Delay_ms(5000);
	       }
////////////////////////////////////////////////////////////////////////////////////////////////////
				//Delay_ms(1000); 
			
				TCP_Disconnect();                      //�����·���ɣ��Ͽ�TCP����
				LiquidDataSend_Flag=1;                 //Һλ���ݳɹ����͵���������־����
			 
        }
		 else
		 {
		   
			
				TCP_Connect();                                //���½���TCP����
			

				TCP_Disconnect();                      //�����·���ɣ��Ͽ�TCP����
		 
			 
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
* Description    : ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main( void )
{
	u8   i=0,j=0;
	u8   DeviceID[6] =SENSORID; 											//��ʼ���豸ID��
  u16  NodeAddr =0x0000;                            //��ȡ�豸ID������������ֽ���Ϊ�ڵ��ַ
  
  NodeAddr=DeviceID[4]*256 +DeviceID[5];
	PeripheralInit();                            //��ʼ��
	
  //***********************************�Բɼ��������д���,һ����ഫ15����*****************************************************
	SensorCollectNumber = DeviceConfig.SendCount / DeviceConfig.CollectPeriod ;
	if(SensorCollectNumber>15)
	{ SensorCollectNumber=15;	}
  
   DataRead_From_Flash(0,9,0, &(DeviceConfig.CollectNum ) ,1);            //��ȡ�Ѳɼ��������� 
   DataCollectCount = DeviceConfig.CollectNum ;
   if(DataCollectCount > SensorCollectNumber)                     //����Ѳɼ�������������Ҫ�ɼ������������Ѳɼ���������
	 {
		 DataCollectCount=0;
	 }
#if DEBUG_TEST	
	printf("\r\n The Data Collect Number is %d \r\n",DeviceConfig.CollectNum );
#endif
	 
	 //error_handle(DeviceID, NodeAddr);                 //�쳣������

   Sms_Consult(DeviceID, NodeAddr);  
	 TCP_Connect();                                //���½���TCP����
	 
  SendDataToServ(STARTUPREQUEST,NULL,0,DeviceID);  //�����ϱ���Ϣ     
	Delay_ms(1000);  
	 
	TCP_Disconnect();                  //�����·���ɣ��Ͽ�TCP����

	while(1)
  {
 
		  WWDOG_Feed =0x1FFF;                            //���ڿ��Ź�ι��,��ʱ4��20�룬�ڶ��ֽ�Լ1��仯һ�Σ���0x09AF�䵽0x099FԼ��ʱ1��

 	    for(i=0;i<5;i++)
		{
			 Delay_ms(200);                                //�ο�̽ͷ�ϵ�������ȶ�ʱ�䣬����Ӧ����
			 if(DMA_UART3_RECEV_FLAG==1)                   //��ѯ���ݽ������
			 {
				 DMA_UART3_RecevDetect(DATARECEV,DeviceID, NodeAddr);   
				 break;
			 }
    }	
		 // Sms_Consult(DeviceID, NodeAddr);  
	
			if(DataCollectCount < SensorCollectNumber)   //���Ѳɼ�������С�ڹ��ɼ�����
			{  
#if DEBUG_TEST				
				printf("\r\nThe Sensor Has DataCollectCount is %d \r\n",DataCollectCount); 
#endif				
		
				SenserDataCollect(&PerceptionData, DeviceID, NodeAddr);        //��ȡ����������
      }
			else                                                              //�ɼ���ɺ�������ݵı���
			{	
				if(DataCollectCount == SensorCollectNumber)         //������ݲɼ��󣬿�ʼ�ϴ�����
				//printf("The Sensor Flash DataCollectCount is %d ",DataCollectCount);      //
				DataCollectCount=0;                                   //�����ݽ������㣬���²ɼ�
		
		  }
			if(LiquidDataSend_Flag ==1)            
			{ 
				gotoSleep(DeviceConfig.CollectPeriod );
				//Sms_Consult(DeviceID, NodeAddr);                 //Ŀǰע��
			}
		
	}

  
}


/*******************************************************************************
* Function Name  : void PeripheralInit( void )
* Description    : ��ʼ���˿ڼ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PeripheralInit( void )
{

	WWDG_Init(0x7F,0X5F,WWDG_Prescaler_8); 	//���ȿ������ڿ��Ź���������ֵΪ7f,���ڼĴ���Ϊ5f,��Ƶ��Ϊ8	
	SysTick_Init();         //��ʼ��ʱ��      
	USART1_Config();      /* USART1 ����ģʽΪ 9600 8-N-1��  �жϽ��� */    //���ڴ��ڵ�������
	Delay_ms(200);
//	USART2_Config();      /* USART2 ����ģʽΪ 9600 8-N-1���жϽ��� */    //����433ͨ�ŷ�ʽ
	USART3_Config();      /* USART3 ����ģʽΪ 115200 8-N-1���жϽ��� */    //����3Gģ��ͨ�ŷ�ʽ   
	UART_NVIC_Configuration();
	USART3_DMA_Config();
	
	RTC_NVIC_Config();                 /* ����RTC���ж����ȼ� */
	RTC_CheckAndConfig(&systmtime);

	ADCconfig();                       //����ADC��ʼ��
	
	PowerON_GPRS();                    //��GPRSģ���Դ  3.8V	
  PowerON_Sensor();         //�򿪴�������ѹ  12V
	Delay_ms(500);
	Start_time=RTC_GetCounter();             //��¼��������ʱ��
	Display();
	ConfigData_Init(&DeviceConfig);
 
  if (PowerOffReset ==1)     
  {
    printf("\r\n�������������³�ʼ�����ؼ�\r\n");                 //����ʹ��
		DS2780_CapacityInit();    //���������д�������
		Set_register_ds2780();    //�����Կ��ؼ����³�ʼ��
	  set_ACR(1000);           //�����Կ��ؼ����³�ʼ��

  }
	
	//GPRS_Init();             //��ʱ����
	GPRS_Config();
	Delay_ms(5000);            //�ȴ����Ž���   
	//Sms_Consult(pDeviceID, sNodeAddress);
}

/*******************************************************************************
* Function Name  : void Display( void )
* Description    : ��ӡ������Ϣ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Display(void)
{
	u32       TimCount_Current =0;
	u8   DeviceID[6] = SENSORID ; 
	TimCount_Current = RTC_GetCounter();
	printf("\r\n********* �����������������޹�˾ *********");
	printf("\r\n************* ȼ�����ܼ���ն� ***********");
	printf("\r\n************* BIRMM-RTU100 ***********");
	printf("\r\n************* �豸ID�ţ�%x %x %x %x %x %x***********",DeviceID[0],DeviceID[1],DeviceID[2],DeviceID[3],DeviceID[4],DeviceID[5]);
	printf("\r\n************** Ӳ������: 433�汾 **********");
	printf("\r\n************** Ӳ���汾��: %x ***********",HARDVERSION);
	printf("\r\n************** ����汾��: %x **************\r\n\r\n",SOFTVERSION);
	
	Time_Display(TimCount_Current,&systmtime); 
	DeviceConfig.Time_Sec  =systmtime.tm_sec;
	DeviceConfig.Time_Min  =systmtime.tm_min;
	DeviceConfig.Time_Hour =systmtime.tm_hour;
	DeviceConfig.Time_Mday =systmtime.tm_mday;		
	DeviceConfig.Time_Mon  =systmtime.tm_mon;
	DeviceConfig.Time_Year =systmtime.tm_year-2000; //���ϴ����ȥ��������				
		    

printf("**************��ǰʱ��:%0.4d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d***********\r\n",systmtime.tm_year,systmtime.tm_mon,     //����ʹ��
				systmtime.tm_mday,systmtime.tm_hour,systmtime.tm_min,systmtime.tm_sec);     //����ʹ��
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
