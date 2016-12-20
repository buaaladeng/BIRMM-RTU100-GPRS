

/***************************本程序为初始化设备********************************/
#include "test.h"


extern struct liquid_set DeviceConfig;

//(1)配置采集相关信息，分别为采集周期、上传周期、重传次数、低浓度阈值、高浓度阈值
void write_collect_parameter_to_flash(void)      //利用该函数可进行燃气智能终端采集配置   配置3G版本
{  
	uint8_t collect_period[2]={00,60};     //采集周期  默认60分钟采集一次
  uint8_t send_out[2]={00,60};           //上传周期   默认60分钟上传一次
  uint8_t retry_num = 3;                //重传次数
	uint8_t first_collect[2]={00,00};     //第一次采集时间
	uint8_t collect_num = 0;              //已采集数量
  uint8_t  error_state[2]={1,1};          //初始化异常信息，第一次初始化均为正常信号
	
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
	DataWrite_To_Flash(2,1,0,error_state ,2);   //写入异常状态
}

//(2)配置3G模块信息, 报警电路号码、IP端口号、PORT端口号
void write_3G_parameter_to_flash(void) 
{
	char    phone_num[16]={"861064617178004"};             //报警电话号码
//	char    ip[16]={"119.254.103.80"};                     //网络服务IP地址
//	char    ip[16]={"117.158.210.189"};                     //网络服务IP地址
	char    ip[16]={"124.42.118.86"};                     //网络服务IP地址
//	char    port[6]={"2020"};                              //网络服务端口号
// 	char    ip[16]={"192.168.1.115"};                     //网络服务IP地址
	char    port[6]={"2017"};                              //网络服务端口号
////////////////////将参数写入FLASH中////////////////////////////
  DataWrite_To_Flash(0,4,0,phone_num,strlen(phone_num));
	DataWrite_To_Flash(0,5,0,ip,strlen(ip));
	DataWrite_To_Flash(0,6,0,port,strlen(port));
}

//(3)配置433模块信息
void write_433_parameter_to_flash(void) 
{
	
	
}
//(4)配置时钟信息
void Set_Time(void)
{
	DeviceConfig.Time_Year = 0x10;              //2016年
	DeviceConfig.Time_Mon  = 0x09;              //8月
	DeviceConfig.Time_Mday = 0x08;              //23 
	DeviceConfig.Time_Hour = 0x08;              //
	DeviceConfig.Time_Min  = 0x34;
	DeviceConfig.Time_Sec  = 0x00;
	
	Time_Auto_Regulate(&DeviceConfig);
}




void test_sensor_to_flash(void)                 //将传感器数据写入FLASH内，并读出来
{
	   struct SenserData TestSensorData;
     uint8_t   sensordata[7]={0x00};               //将传感器数据写入FLASH		
     uint8_t   readdata[7]={0x00};
     uint8_t  i;
     TestSensorData.Ch4Data .Data_Float   = 3.9;               //利用假数据进行调试
	
     //TestSensorData.CollectTime  =(DeviceConfig.Time_Hour)*60 +(DeviceConfig.Time_Min);      //第一组数据采集时间
     TestSensorData.CollectTime = 40;
		 TestSensorData.DataCount  =  1;                       //当前采集的个数
			//Delay_ms(500);
      
///////////////////////////////////////////////////////////////////////////////////////////////////////////////		
      //DataCollectCount = pGetData->DataCount;
			//ConfigData.CollectNum = DataCollectCount;
			//DataWrite_To_Flash(0,9,0,&pGetData.DataCount,1);
////////////////////////////////将采集的传感器数据写入FLASH///////////////////////////////////////////////////
			sensordata[0]= TestSensorData.CollectTime  >> 8;
			sensordata[1]= TestSensorData.CollectTime & 0xff;
      for(i=0;i<4;i++)
      sensordata[2+i] =	TestSensorData.Ch4Data.Data_Hex[i];
      sensordata[6] =   TestSensorData.DataCount;			
      for(i=0;i<7;i++)
		  printf("\r\nwrite this data to flash!\r\n   ->%x",sensordata[i]);
		 
      DataWrite_To_Flash(0,16,0,sensordata,sizeof(sensordata));
			printf("\r\nthe sensor data already send to flash!\r\n");
	 
	   DataRead_From_Flash(0,16,0, readdata ,sizeof(readdata)); //从Flash中读取重传次数
		 for(i=0;i<7;i++)
		 printf("\r\nwrite this data to flash!\r\n   ->%x",readdata[i]);
	   printf("\r\ntest sensor data from Flash!    ->%s\r\n",readdata);
}



