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

extern  struct Sensor_Set  DeviceConfig;          //Һλ��������Ϣ�ṹ��
extern  struct    SMS_Config_RegPara   ConfigData;

extern u8     DataCollectCount;                   //���ݲɼ�������
extern char   SendData_Flag; 

extern struct    rtc_time        systmtime;        //ϵͳʱ��
extern uint8_t   Usart3_send_buff[300];
extern uint8_t   DMA_UART3_RECEV_FLAG ;      //USART3 DMA���ձ�־����

u8 phone_number_next=0;                   //�绰�������һ��ָʾ
u8 ip_number_next=0;                     //����IP����һ��ָʾ
u8 port_number_next=0;                   //����˿ڵ���һ��ָʾ              

/*******************************************************************************
* Function Name  : UploadFlash
* Description    : ������Ҫ�洢���ַ����������洢λ������
* Input          : None
* Output         : ���ش洢�ɹ���־
* Return         : None
*******************************************************************************/
uint8_t  UploadFlash(char* pSetPara, uint8_t  InstructCode)
{
 
 
  uint8_t   j=0;
  uint8_t   Counter=0;
  
	switch(InstructCode)
	{	
		case 2:          //�洢�ɼ�����
		{   
			 for(j=0;j<strlen(pSetPara);j++)		
			 //printf("\r\n---------%x----------\r\n",pSetPara[j]);
		   DataWrite_To_Flash(0,2,0,(uint8_t*)pSetPara,2);      //���ɼ�����д��Flash
			 DeviceConfig.MessageSetFlag =1;     //��ʾ��ǰҺλ�ǲ���ͨ�������޸�	
       printf("\r\n�洢�ɼ����ڣ�%s\r\n",pSetPara);			          //����ʹ��						
			 return 1;
		}
		case 3:          //�洢�ϴ�����
		{  
			 for(j=0;j<strlen(pSetPara);j++)
			 //printf("\r\n---------%x----------\r\n",pSetPara[j]);
			
		   DataWrite_To_Flash(0,3,0,(uint8_t*)pSetPara,2);      //���ϴ�����д��Flash
			 DeviceConfig.MessageSetFlag =1;     //��ʾ��ǰҺλ�ǲ���ͨ�������޸�	
       printf("\r\n�洢�ϱ����ڣ�%s\r\n",pSetPara);			          //����ʹ��						
			 return 1;
		}
		
		case 4:            //�洢��������
		{
			  //�ֻ�������б�������������Ҫ���ӡ�
			 DataWrite_To_Flash(0,4,0,(uint8_t*)pSetPara,strlen(pSetPara));   //����������д��Flash 
       DeviceConfig.MessageSetFlag =1;                                  //��ʾ��ǰҺλ�ǲ���ͨ�������޸�		 
       printf("\r\n�洢�����绰:%s\r\n",pSetPara);			          //����ʹ��
			 return 1;
		}
		
		case 5:              //�洢������IP
		{

			 for(j=0,Counter=0;j<strlen(pSetPara);j++)                       //���ú����Ѿ����˷��������,strlen(pSetPara)<=15
			 {
          if((pSetPara[j]>='0')&&(pSetPara[j]<='9'))
					{
//             CharTemp[j] =pSetPara[j];
						 ;
          }
				  else if(pSetPara[j]=='.')                                    //�ָ���ͳ��
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
          DataWrite_To_Flash(0,5,0, (uint8_t*)pSetPara,strlen(pSetPara));   //��������IPд��Flash 
       }
			 else
			 {
          printf("\r\nInput Server IP ERROR!!\r\n");
       }
			 DeviceConfig.MessageSetFlag =1;     //��ʾ��ǰҺλ�ǲ���ͨ�������޸�		 
			  printf("\r\n�洢����IP��: %s\r\n",pSetPara);			          //����ʹ��
			 return 1;
			 
		}
		
		case 6:           //�洢�������˿ں�
		{
			 DataWrite_To_Flash(0,6,0,(uint8_t*)pSetPara,strlen(pSetPara));      //���������˿ں�д��Flash
			 DeviceConfig.MessageSetFlag =1;     //��ʾ��ǰҺλ�ǲ���ͨ�������޸�	
        printf("\r\n�洢����˿ں�: %s\r\n",pSetPara);			          //����ʹ��			
			 return 1;
			
		}

		case 10:          //�洢�ش�����
		{
		   DataWrite_To_Flash(0,10,0,(uint8_t*)pSetPara,1);      //���ش�����д��Flash
			 DeviceConfig.MessageSetFlag =1;     //��ʾ��ǰҺλ�ǲ���ͨ�������޸�		 
			 printf("\r\n �洢�ش�����: %x\r\n",pSetPara[0]);			          //����ʹ��			
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
* Description    : ��ȡ0~1000�������
* Input          : None
* Output         : ���������
* Return         : None
*******************************************************************************/
unsigned int Get_Rand_Num(void)
{    
	    unsigned int rand_num;
      srand((unsigned)RTC_GetCounter());                                     //��������
		  rand_num = rand()%1000;                                          //��ȡ0~1000�������
	    return(rand_num);
}

/**************************************************************************
* Function Name  : SendDataToServ()
* Description    : ͨ��Э��ķ��Ͳ���ʵ�֣����øú���֮ǰ��������豸���͡��ϱ����ݸ�ʽ�����䷽ʽ�Լ�����汾�Ķ��塣
                   ��صĶ����SensorInit.h�� AiderProtocol.h
                   ��������TAG�������з��ͣ�������Ͳ��ɹ��������趨ֵ�ش���
* Input          : 
                   �������ͣ�    �ο�AiderProtocol.h�ڲ������ͣ���Ҫ�����豸��ӦSET/GET����GETRESPONSE���豸�����ϱ�TRAPREQUEST��
                                 �豸�����ϱ���ϢSTARTUPREQUEST���豸��Ӧ����������WAKEUPRESPONSE���豸�������ONLINEREQUEST      
                   ����TAG�б�:  �ο�AiderProtocol.h��TAG�ṹ�壬����Ҫ���͵�TAG���ݽ�����������ʱ�ɴ���NULL
                   TAG����:			 ��Ҫ���͵�TAG��������TAG����������20��������ʱΪ0
                   �豸ID:       6�ֽڵ��豸ID��  
* Output         : None
* Return         : ���ͳɹ���־
**************************************************************************/
uint8_t SendDataToServ(CommunicateType CommType,struct TagStruct TagList[],uint8_t TagNum,u8* pDeviceID)
{
  DeviceType DevType 				= DEVICE;        //��ʼ���豸���
	ReportDataType	RDataType =	REPORTDATA;    //��ʼ���ϱ�����
  SendType   SType = TYPESEND; 	            //ͨ�ŷ�ʽΪ433ͨ��
	struct   DataFrame  SendData;             //�������ݸ�ʽ
	struct   DataFrame* pDataFrame =&SendData;  //��������ָ��
	struct   TagStruct* pTag=TagList;           //����TAGָ��
	uint8_t  pSendBuff[300];                    //��Ҫ���͵�����
	char*    pChar =NULL;
  u16      RecevFlag =0;           //���������ݽ��ձ�־����
	uint16_t NodeAddress;            //�豸��ַ
  uint8_t  SendCounter;            //���ʹ���
	uint16_t rand_num;               //�������
	u16      CrcData=0;
	u8       i=0,j=0;
	uint8_t  RevTagNum = 0;       //TAG������
	u8       ValidLength =0;     //��Ч���ݳ��ȣ�Tag�ռ���ʵ�ʴ洢��Ч���ݵĳ��ȡ�
	const u8 PreLength   =16;    //֡ǰ׺���ȣ�ָ��֡ǰ���뵽OID���У�����Tag���У������ݳ��ȣ�����֡ǰ���룬��������OID���У�����Tag���У�
	const u8 CoreLength  =12;    //�ؼ���Ϣ���ȣ�ָ���ݾ��ɲ��ֳ����ֶ�ָʾ����ֵ��ȥ��OID���У�����Tag���У���ʣ�����ݵĳ���
	const u8 TagPreLength=6;     //TAG��OID����Ϊ4+TAG���ݳ���2
	uint8_t  TagLength=0;        //TAG���ܳ���
	uint8_t  Offset=0;           //ָ��ƫ����
	u8       TotalLength;        //��ȥCRC�������ܳ���
	NodeAddress = pDeviceID[4]*256 +pDeviceID[5];
	pChar =(char*)pDataFrame;
	
	memset(pChar,0x00,sizeof(struct DataFrame));   //��ʼ���ṹ��
	SendData.Preamble =0xA3;
	SendData.Version  = SOFTVERSION;
	for(i=0;i<6;i++)
	{
   SendData.DeviceID[i] = pDeviceID[i];
  }
	SendData.RouteFlag =SType;                              //���ݷ��ͷ�ʽ
	SendData.NodeAddr  =ntohs( NodeAddress);               //����Ϊ�����򣬼����ֽ���ǰ�����ֽ��ں�
	SendData.PDU_Type  =(CommType<<8)+(1<<7)+DEVICE;
	SendData.PDU_Type  =ntohs(SendData.PDU_Type);        //����Ϊ�����򣬼����ֽ���ǰ�����ֽ��ں�
	SendData.Seq       =1;
	
	
	if(pTag!=NULL)
		{
				RevTagNum=TagNum;                     //���ָ�벻�գ���֤�������ݴ������������TAG������
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
			
				SendData.TagList[0].OID_Command = ntohl(DEVICE_QTY);   //����Ϊ�����򣬼����ֽ���ǰ�����ֽ��ں�
				SendData.TagList[0].Width =1;                         //
				SendData.TagList[0].Value[0] =DeviceConfig.BatteryCapacity;  //�����ϴ�ʱ�ĵ��ʣ�����		  
	
				SendData.TagList[1].OID_Command= ntohl(SYSTERM_DATA);    //����Ϊ�����򣬼����ֽ���ǰ�����ֽ��ں�
				SendData.TagList[1].Width =3;                            //
				SendData.TagList[1].Value[0] =systmtime.tm_year-2000;       //ϵͳ���ڣ���
				SendData.TagList[1].Value[1] =systmtime.tm_mon ;            //ϵͳ���ڣ���
				SendData.TagList[1].Value[2] =systmtime.tm_mday ;           //ϵͳ���ڣ���
			
			  
				RevTagNum =2;                     //TAG���������ڵ���+����
			  
		 	  break;
				
		}			
		case ONLINEREQUEST:
		{
			
			break;
		}
			
		case WAKEUPRESPONSE:
		{
	  SendData.TagList[0].OID_Command =ntohl(DEVICE_WAKEUP); //����Ϊ�����򣬼����ֽ���ǰ�����ֽ��ں�
	  SendData.TagList[0].Width =1;
	  SendData.TagList[0].Value[0]=1;
	  RevTagNum =1;
	  		
			break;
		}
			
		case STARTUPREQUEST:
		{
			
			SendData.TagList[0].OID_Command =ntohl(DEVICE_STATE); //����Ϊ�����򣬼����ֽ���ǰ�����ֽ��ں�
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
			memcpy(pSendBuff,pChar,PreLength );                             //���г�TAG�����ݸ���
			
			SendData.CrcCode =0xffff;                                        //CRC�ֶθ���ֵ
			TotalLength = PreLength + ValidLength;
			
			Offset=	PreLength;		
			for(i=0; i<RevTagNum;i++)
			{
			
		  pChar = (char*)&(SendData.TagList [i].OID_Command); 
      TagLength = SendData.TagList[i].Width +TagPreLength;				//����TAG�ĸ��� 
			SendData.TagList[i].Width=ntohs(SendData.TagList[i].Width);
     	memcpy((pSendBuff+Offset),pChar,TagLength );
			
			Offset+=TagLength;	
			}
			
			CrcData = CRC16(pSendBuff, TotalLength);   // Update the CRC value
	    SendData.CrcCode =CrcData;
	    pSendBuff[TotalLength+1] = CrcData&0xff;   //CRC���ֽ���ǰ
	    pSendBuff[TotalLength+2] = CrcData>>8;     //CRC���ֽ��ں�
						
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
		 if(RecevFlag !=0)                     //�ɹ����յ�����
		 {
        #if DEBUG_TEST	 
				printf("\r\nReceive TrapResponse success!\r\n"); //����ʹ��
				#endif
			  break;     
     }
		   rand_num=Get_Rand_Num();
#if DEBUG_TEST
		 	 printf("\r\nthe rand_num is %d\r\n",rand_num);
#endif
		  Delay_ms(3000+2*rand_num);                        //�������
		
  }	
}
/*******************************************************************************
* Function Name  : TrapData
* Description    : ���д��������ݵ������ϱ�
* Input          : �豸�ڵ�
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
	
		Taglist[0].OID_Command = ntohl(DEVICE_QTY);   //����Ϊ�����򣬼����ֽ���ǰ�����ֽ��ں�
		Taglist[0].Width =1;                         //
		Taglist[0].Value[0] =DeviceConfig.BatteryCapacity;  //�����ϴ�ʱ�ĵ��ʣ�����		  
	
	  Taglist[1].OID_Command= ntohl(SYSTERM_DATA);    //����Ϊ�����򣬼����ֽ���ǰ�����ֽ��ں�
		Taglist[1].Width =3;                            //
		Taglist[1].Value[0] =systmtime.tm_year-2000;       //ϵͳ���ڣ���
		Taglist[1].Value[1] =systmtime.tm_mon ;            //ϵͳ���ڣ���
		Taglist[1].Value[2] =systmtime.tm_mday ;           //ϵͳ���ڣ���
			
			  
	  Tag_Count =2;                     //TAG���������ڵ���+����
	
		for(i=0;i< DataCollectCount;i++)    //�����ϱ�����TAG�����
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
	
	 SendDataToServ(TRAPREQUEST,Taglist,Tag_Count,pDevID);                //433ģ�������ϴ�����
}


/*******************************************************************************
* Function Name  : ��ԺЭ�����ݷ���
* Description    : ���ڽ��շ��������ã�û��9��ͷ��־
* Input          : None                       
* Output         : None
* Return         : None
*******************************************************************************/
void Treaty_Data_Analysis(u8* pTreatyBuff, u16* pFlag, u8* pDeviceID, u16 NodeAddress)	
{
	u8   DataLen =0;          //Tag���л���OID���еĳ���
	u16  PduType =0;
  
	u8   i=0,j=0,k=0;
	u8*  pChar =NULL;
	u32* pOid  =NULL;
  u8   next = 0;
	struct DataFrame ParaRequest;
  struct TagStruct RecTagList[MAX];           //���յ���TAG
	uint8_t RecTagNum;                          //���յ�TAG������
  PduType =pTreatyBuff[13]*256 +pTreatyBuff[14];   //�����������PDU����
  switch(PduType)
	{
   /******************************************����������********************************************/	     
        case ((GETREQUEST<<8)+(1<<8)+DEVICE):
				{
						printf("\r\nReceive Get Request Command from Server.\r\n");    //��⵽���������͵���������
						*pFlag =PduType;
						DataLen =pTreatyBuff[2]*256 +pTreatyBuff[3]-12; //���յ�OID���ܳ���
					  ParaRequest.Tag_Count =DataLen/4;               //���յ�OID������,��TAG����
					  if((ParaRequest.Tag_Count>0)&&(ParaRequest.Tag_Count <= MAX))   //�޶�OID��������������20��
						{
							  pOid = (u32*)&pTreatyBuff[16];
							  j    =ParaRequest.Tag_Count;
							  RecTagNum = j;                            //����TAG������
								for(i=0;i<ParaRequest.Tag_Count;i++)
								{ 
									  ParaRequest.TagList[i].OID_Command =(CommandType)*pOid;                    //����OID�ɼ�   
									  ParaRequest.TagList[i].OID_Command =ntohl( ParaRequest.TagList[i].OID_Command );      //�����������еı仯
									  ParaRequest.TagList[i].Width=0;
									
#if DEBUG_TEST									
									  printf("\r\n--Cycle--%4x----.\r\n", ParaRequest.OID_List[i]);   //��ʾOID
#endif									
										j--;
									  if(j==0)   
										{
                       break;
                    }
									  pOid++;
								}
								printf("\r\n--i:%d--j:%d--%4x----.\r\n",i,j, ParaRequest.TagList[i].OID_Command);    //��ʾOID
								
            }
					  else
						{
#if DEBUG_TEST	 
							  printf("\r\nReceive Command OID not correct.\r\n");    //????
#endif
            }

						DMA_UART3_RECEV_FLAG =0;     //����ձ�־����
						SendDataToServ(GETRESPONSE, ParaRequest.TagList,RecTagNum,pDeviceID);    //������Ӧ
					  //GetResponse( ParaRequest, Usart3_send_buff, pDeviceID,  NodeAddress);
					  break;
        }	

/******************************************�����豸��������********************************************/	
        case ((SETREQUEST<<8)+(1<<8)+DEVICE):                  //���շ������·�������
				{
            #if DEBUG 
						printf("\r\nReceive Set Request Command from Server.\r\n");    //����ʹ��
						#endif
						*pFlag =PduType;
					  DataLen =pTreatyBuff[2]*256 +pTreatyBuff[3]-12;    //���յ���Tag���е��ܳ���
					  if(DataLen >6)           //���ٴ���һ���Ϸ������ò���
						{
                pOid = (u32*)(pTreatyBuff+16);    //�������·��ĵ�һ��OID
							  i=0;
							  while( DataLen >6 )
								{
									 printf("\r\n--Cycle--%4x----.\r\n",ntohl(*pOid));         //����ʹ��
									 pChar = (u8*)pOid;
                   switch(ntohl(*pOid))
									 {										
											case SYSTERM_TIME:     //ϵͳʱ��
											{
													DeviceConfig.Time_Year =*(pChar+6);
												  DeviceConfig.Time_Mon  =*(pChar+7);
												  DeviceConfig.Time_Mday =*(pChar+8);
												  DeviceConfig.Time_Hour =*(pChar+9);
												  DeviceConfig.Time_Min  =*(pChar+10);
												  DeviceConfig.Time_Sec  =*(pChar+11);
												  printf("\r\n-��-��-��-ʱ-��-�룺-%d--%d--%d--%d--%d--%d--.\r\n",DeviceConfig.Time_Year,DeviceConfig.Time_Mon,DeviceConfig.Time_Mday,
												                 DeviceConfig.Time_Hour,DeviceConfig.Time_Min, DeviceConfig.Time_Sec  );         //����ʹ��
												  if((DeviceConfig.Time_Mon<=12)&&(DeviceConfig.Time_Mday<=31)&&(DeviceConfig.Time_Hour<=23)&&(DeviceConfig.Time_Min<=60)&&(DeviceConfig.Time_Sec<=60)) //�����Ϸ����ж�
													{
                            Time_Auto_Regulate(&DeviceConfig);             //ͨ���������·���������RTCʱ��У׼��
                          }
												  ParaRequest.TagList[i].OID_Command=SYSTERM_TIME;
                         	ParaRequest.TagList[i].Width = 6;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
													
												  pOid =(u32*)(pChar+12);                          //ָ�����1��Tag
												  DataLen = DataLen-12;
												 	i++;        //�Ϸ�OID������
													break;							
											}	
											case CLT1_ITRL1:       //һʱ���ɼ����
											{
													DeviceConfig.CollectPeriod =pChar[6]*256+pChar[7];
												  ConfigData.CollectPeriod_Byte [0]=*(pChar+6);
												  ConfigData.CollectPeriod_Byte [1]=*(pChar+7);
												
												  printf("\r\n---CollectPeriod:-%d---.\r\n", DeviceConfig.CollectPeriod );         //����ʹ��
												  if(3<DeviceConfig.CollectPeriod<=1440)              //�����Ϸ����ж�
													{
                             UploadFlash((char*)ConfigData.CollectPeriod_Byte,2);  //��������Flash
                          }
													
													ParaRequest.TagList[i].OID_Command=CLT1_ITRL1;
                         	ParaRequest.TagList[i].Width = 2;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
											
												  pOid =(u32*)(pChar+8);                         //ָ�����1��Tag
												  DataLen = DataLen-8;
												 	i++;        //�Ϸ�OID������
													break;
											}	
//											case CLT1_CNT1:        //һʱ���ɼ�����
//											{
//													DeviceConfig.CollectNum =pChar[6]*256+pChar[7];
//													printf("\r\n-CollectNum:-%2x---.\r\n", DeviceConfig.CollectNum );         //����ʹ��
//												  if(DeviceConfig.CollectNum<=1440)              //�����Ϸ����ж�
//													{
//                             UploadFlash((char*)&(DeviceConfig.CollectNum), 9);  //��������Flash
//                          }
//												  ParaRequest.OID_List[i] =CLT1_CNT1;            
//												  pOid =(u32*)(pChar+8);                         //ָ�����1��Tag
//												  DataLen = DataLen-8;
//												 	i++;        //�Ϸ�OID������
//													break;
//											}
											
//									
											case UPLOAD_CYCLE:     //�����ϱ�����
											{
													DeviceConfig.SendCount  =pChar[6]*256+pChar[7];
												  ConfigData.SendCount_Byte  [0]=*(pChar+6);
												  ConfigData.SendCount_Byte [1]=*(pChar+7);
												 
												  if(0<DeviceConfig.SendCount <=1440)              //�����Ϸ����ж�
													{
                             UploadFlash((char*) ConfigData.SendCount_Byte , 3);  //��������Flash
                          }
											  	printf("\r\n-UploadCycle:-%d---.\r\n", DeviceConfig.SendCount  ); //����ʹ��
													
													ParaRequest.TagList[i].OID_Command=UPLOAD_CYCLE;
                         	ParaRequest.TagList[i].Width = 2;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
												
												  pOid =(u32*)(pChar+8);                           //ָ�����1��Tag
												  DataLen = DataLen-8;
											  	i++;        //�Ϸ�OID������
													break;
											}	
///////////////////////////////////////////////////////////////////////////////////////////add by gao 											
										
											 case SMS_PHONE:     //�����绰����
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
													
												  if(11<strlen(DeviceConfig.AlarmPhoneNum)<16 )              //�����Ϸ����ж�
													{
                             UploadFlash((char*)DeviceConfig.AlarmPhoneNum  ,4);  //��������Flash
                          }
											  	printf("\r\n-Server set SMS_PHONE:-%s---.\r\n", DeviceConfig.AlarmPhoneNum   ); //����ʹ��
												  ParaRequest.TagList[i].OID_Command=SMS_PHONE;
                         	ParaRequest.TagList[i].Width = phone_number_next;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
												
												  pOid =(u32*)(pChar+next);                           //ָ�����1��Tag
												  DataLen = DataLen-next;
											  	i++;        //�Ϸ�OID������
													break;
											}
											
                       	 case BSS_IP:     //�������IP��ַ
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
												  if(8<strlen(DeviceConfig.ServerIP )<16)              //�����Ϸ����ж�
													{
                             UploadFlash((char*)DeviceConfig.ServerIP  , 5);  //��������Flash
                          }
											  	printf("\r\n-BSS_IP:-%s---.\r\n", DeviceConfig.ServerIP   ); //����ʹ��
													ParaRequest.TagList[i].OID_Command=BSS_IP;
                         	ParaRequest.TagList[i].Width = ip_number_next;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
										
												  pOid =(u32*)(pChar+next);                           //ָ�����1��Tag
												  DataLen = DataLen-next;
											  	i++;        //�Ϸ�OID������
													break;
											}
												 case BSS_PORT:     //�������˿ں�
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
												  if(strlen(DeviceConfig.ServerPort )!=0)              //�����Ϸ����ж�
													{
                             UploadFlash((char*)DeviceConfig.ServerPort  , 6);  //��������Flash
                          }
											  	printf("\r\n-BSS_PORT:-%s---.\r\n", DeviceConfig.ServerPort   ); //����ʹ��
													ParaRequest.TagList[i].OID_Command=BSS_PORT;
                         	ParaRequest.TagList[i].Width = port_number_next;
                          for(k=0;k<ParaRequest.TagList[i].Width;k++)
													ParaRequest.TagList[i].Value[k]=pChar[6+k];
												 
												  pOid =(u32*)(pChar+next);                           //ָ�����1��Tag
												  DataLen = DataLen-next;
											  	i++;        //�Ϸ�OID������
													break;
											}


											case DEF_NR:            //�ش�����
											{
													DeviceConfig.RetryNum =*(pChar+6);
												  if(1<= DeviceConfig.RetryNum <10)                         //�����Ϸ����ж�
													{
                             UploadFlash((char*)&(DeviceConfig.RetryNum), 10);  //��������Flash
                          }
												  printf("\r\n-Retry Num-%x---.\r\n", DeviceConfig.RetryNum);   //����ʹ��
//												  else               //�����ò������Ϸ�ʱ��ʹ��Ĭ�ϲ�����ͬʱ������Flash����
//													{
//                             DeviceConfig.RetryNum =1;             //Ϊ�˷�����ʵ���������Ĭ�ϲ���
//                          }
													ParaRequest.TagList[i].OID_Command=DEF_NR;
                         	ParaRequest.TagList[i].Width = 1;
													for(k=0;k<ParaRequest.TagList[i].Width;k++)
												  ParaRequest.TagList[i].Value[k] = pChar[6+k];
												  pOid =(u32*)(pChar+7);                     //ָ�����1��Tag
												  DataLen = DataLen-7; 
 	                        i++;               //�Ϸ�OID������												
													break;
											}	
											

////////////////////////////////////////////////////////////////////////////////////////////add by gao 											
											default:
											{
												 #if DEBUG_TEST	
												 printf("\r\nWarning!!Tag OID not recognition!\r\n"); //����ʹ��
												 #endif
												 pOid =(u32*)(pChar+1);  //ָ�����һ���ֽڣ���ѯ�������޺Ϸ�OID
												 DataLen = DataLen-1;    //ָ�����һ���ֽڣ���ѯ�������޺Ϸ�OID
												 break;
											}
									 }
                }
						}
						if((i>0)&&(i<=20))
						{
               ParaRequest.Tag_Count =i;           //��OID���н��м���	
							 RecTagNum=i;  
            }
            else
						{
               ParaRequest.Tag_Count  =7;           //��OID������ֵ����20ʱ,��������ֵ���ó�Ĭ��ֵ��Ĭ��7��
            }	
		
            DMA_UART3_RECEV_FLAG =0;     //���ձ�־������λ						
					  SendDataToServ(GETRESPONSE,ParaRequest.TagList,RecTagNum,pDeviceID);    //������Ӧ				
					  break;
        }	
				
/******************************************�豸�����ϱ���Ϣ��Ӧ*********************************************/	
				case ((TRAPRESPONSE<<8)+(1<<8)+DEVICE):                  //�յ��ϱ����ݵ���Ӧ
				{
            #if DEBUG_TEST	 
						printf("\r\nReceive Trap Response Command from Server.\r\n");    //����ʹ��
						#endif
					  /////////////////////////
					  //������������������֤������һ���ϴ�һ֡���ݵ������ֻ��ͨ��PDUType������֤�����յ�Ӧ��֡�������������
						*pFlag =PduType;
					  break;
        }	
				
/******************************************�豸���������Ӧ*********************************************/	
				case ((ONLINERESPONSE<<8)+(1<<8)+DEVICE):                      //�豸���������Ӧ
				{
            #if DEBUG_TEST
						printf("\r\nReceive Online Response Command from Server.\r\n");    //����ʹ��
            #endif
					
						*pFlag =PduType;
					  break;
        }	
/******************************************�豸�����ϱ���Ϣ��Ӧ*********************************************/				
				case ((STARTUPRESPONSE<<8)+(1<<8)+DEVICE):                       //�յ��豸�����ϱ���Ϣ��Ӧ
				{
            #if DEBUG_TEST	 
						printf("\r\nReceive Startup Response Command from Server.\r\n");    //����ʹ��
						#endif
						*pFlag =PduType;
					  break;
        }	
				
/******************************************�����������豸����*********************************************/				
				case ((WAKEUPREQUEST<<8)+(1<<8)+DEVICE):
				{
            #if DEBUG_TEST	 
						printf("\r\nReceive Wakeup Request Command from Server.\r\n");    //����ʹ��
						#endif
						*pFlag =PduType;
						
					  break;
        }	

        default:
				{
					 #if DEBUG_TEST	
           printf("\r\nWarning!!PDU Type not recognition!\r\n"); //����ʹ��
					 #endif
					 break;
        }
   }      
}

