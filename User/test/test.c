

/***************************������Ϊ��ʼ���豸********************************/
#include "test.h"


extern struct liquid_set DeviceConfig;

//(1)���òɼ������Ϣ���ֱ�Ϊ�ɼ����ڡ��ϴ����ڡ��ش���������Ũ����ֵ����Ũ����ֵ
void write_collect_parameter_to_flash(void)      //���øú����ɽ���ȼ�������ն˲ɼ�����   ����3G�汾
{  
	uint8_t collect_period[2]={00,60};     //�ɼ�����  Ĭ��60���Ӳɼ�һ��
  uint8_t send_out[2]={00,60};           //�ϴ�����   Ĭ��60�����ϴ�һ��
  uint8_t retry_num = 3;                //�ش�����
	uint8_t first_collect[2]={00,00};     //��һ�βɼ�ʱ��
	uint8_t collect_num = 0;              //�Ѳɼ�����
  uint8_t  error_state[2]={1,1};          //��ʼ���쳣��Ϣ����һ�γ�ʼ����Ϊ�����ź�
	
	union Hex_Float low_alarm;
	union Hex_Float high_alarm;
   low_alarm.Data_Float =25.0;
   high_alarm.Data_Float =50.0;
	
  //PowerON_Flash(); 
	DataWrite_To_Flash(0,2,0,collect_period,2);  
	DataWrite_To_Flash(0,3,0,send_out,2);
  DataWrite_To_Flash(0,7,0,low_alarm.Data_Hex ,4);
	DataWrite_To_Flash(0,8,0,first_collect,2);
	DataWrite_To_Flash(0,9,0,&collect_num,1);
	DataWrite_To_Flash(0,10,0,&(retry_num),1);
	DataWrite_To_Flash(0,11,0,high_alarm.Data_Hex ,4);
	DataWrite_To_Flash(2,1,0,error_state ,2);   //д���쳣״̬
}

//(2)����3Gģ����Ϣ, ������·���롢IP�˿ںš�PORT�˿ں�
void write_3G_parameter_to_flash(void) 
{
	char    phone_num[16]={"861064617178004"};             //�����绰����
//	char    ip[16]={"119.254.103.80"};                     //�������IP��ַ
//	char    ip[16]={"117.158.210.189"};                     //�������IP��ַ
	char    ip[16]={"124.42.118.86"};                     //�������IP��ַ
//	char    port[6]={"2020"};                              //�������˿ں�
// 	char    ip[16]={"192.168.1.115"};                     //�������IP��ַ
	char    port[6]={"2017"};                              //�������˿ں�
////////////////////������д��FLASH��////////////////////////////
  DataWrite_To_Flash(0,4,0,phone_num,strlen(phone_num));
	DataWrite_To_Flash(0,5,0,ip,strlen(ip));
	DataWrite_To_Flash(0,6,0,port,strlen(port));
}

//(3)����433ģ����Ϣ
void write_433_parameter_to_flash(void) 
{
	
	
}
//(4)����ʱ����Ϣ
void Set_Time(void)
{
	DeviceConfig.Time_Year = 0x10;              //2016��
	DeviceConfig.Time_Mon  = 0x09;              //8��
	DeviceConfig.Time_Mday = 0x08;              //23 
	DeviceConfig.Time_Hour = 0x08;              //
	DeviceConfig.Time_Min  = 0x34;
	DeviceConfig.Time_Sec  = 0x00;
	
	Time_Auto_Regulate(&DeviceConfig);
}




void test_sensor_to_flash(void)                 //������������д��FLASH�ڣ���������
{
	   struct SenserData TestSensorData;
     uint8_t   sensordata[7]={0x00};               //������������д��FLASH		
     uint8_t   readdata[7]={0x00};
     uint8_t  i;
     TestSensorData.Ch4Data .Data_Float   = 3.9;               //���ü����ݽ��е���
	
     //TestSensorData.CollectTime  =(DeviceConfig.Time_Hour)*60 +(DeviceConfig.Time_Min);      //��һ�����ݲɼ�ʱ��
     TestSensorData.CollectTime = 40;
		 TestSensorData.DataCount  =  1;                       //��ǰ�ɼ��ĸ���
			//Delay_ms(500);
      
///////////////////////////////////////////////////////////////////////////////////////////////////////////////		
      //DataCollectCount = pGetData->DataCount;
			//ConfigData.CollectNum = DataCollectCount;
			//DataWrite_To_Flash(0,9,0,&pGetData.DataCount,1);
////////////////////////////////���ɼ��Ĵ���������д��FLASH///////////////////////////////////////////////////
			sensordata[0]= TestSensorData.CollectTime  >> 8;
			sensordata[1]= TestSensorData.CollectTime & 0xff;
      for(i=0;i<4;i++)
      sensordata[2+i] =	TestSensorData.Ch4Data.Data_Hex[i];
      sensordata[6] =   TestSensorData.DataCount;			
      for(i=0;i<7;i++)
		  printf("\r\nwrite this data to flash!\r\n   ->%x",sensordata[i]);
		 
      DataWrite_To_Flash(0,16,0,sensordata,sizeof(sensordata));
			printf("\r\nthe sensor data already send to flash!\r\n");
	 
	   DataRead_From_Flash(0,16,0, readdata ,sizeof(readdata)); //��Flash�ж�ȡ�ش�����
		 for(i=0;i<7;i++)
		 printf("\r\nwrite this data to flash!\r\n   ->%x",readdata[i]);
	   printf("\r\ntest sensor data from Flash!    ->%s\r\n",readdata);
}



