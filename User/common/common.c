/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   串口中断接收测试
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 iSO-MINI STM32 开发板 
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */ 
#include "stm32f10x.h"
#include "misc.h"
#include "stm32f10x_it.h"
#include "bsp_usart.h"
#include "bsp_TiMbase.h" 
#include "bsp_SysTick.h"
#include "string.h"
#include "bsp_rtc.h"
#include "bsp_date.h"
#include "WatchDog.h"
#include "common.h"
#include "gprs.h"
#include "SPI_Flash.h"

extern struct Sensor_Set  DeviceConfig;     //传感器配置信息结构体
extern struct SMS_Config_RegPara   ConfigData; //FLASH读取结构
extern u8         Send_Request_flag;                         //发送数据至上位机后收到反馈
extern u8        ReadData_flag;                       //读取传感器成功标志位，成功为1，失败为0；
extern char   		SendData_Flag;                       //发送数据成功标志位，发送成为为1，不成功为0；
extern char      TcpConnect_Flag;                     //TCP连接成功标志位，连接成功为1，不成功为0；

/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void gotoSleep(uint16_t SendCount)
{
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	
  
	if(SendCount<1) 
	{
		SendCount = 1;                         //防止程序出错，导致设备长期处于休眠状态而无法唤醒，目前设置最长休眠时间为24h
	}
	//Sms_Consult();                          //查阅短信，更新配置参数，后续有待完善
	
	//RTC_SetAlarm(RTC_GetCounter()+(24*60*60/SendCount));   //采集时间到开始唤醒

		
	RTC_SetAlarm(RTC_GetCounter()+(DeviceConfig.SendCount ) * 60 );      //休眠，调试使用
		
		
	RTC_WaitForLastTask();
  if(DeviceConfig.MessageSetFlag ==1)	     //有短信修改参数时，将数据写入存储区
	{
//    ParaUpdateCheck();                     //写配置数据
#if DEBUG_TEST
		printf("\r\nDevice Set Upload Success According to Messages!!\r\n");                    //测试使用
#endif
  }
	Delay_ms(800);  
	PowerOFF_GPRS();                         //关闭GPRS模块电源
	Delay_ms(100);
  PowerOFF_Sensor();             //关闭超声波探头电源
	Delay_ms(100);
  PowerOFF_485();                          //关闭485电源
	Delay_ms(100);
  PowerOFF_Flash();                        //关闭Flash电源
	
	 /*********************************将工作的次数写入FLASH***************************************************/			
	DeviceConfig.WorkNum  += 1;
	ConfigData.WorkNum_Byte [0]=DeviceConfig.WorkNum>>8;
	ConfigData.WorkNum_Byte [1]=DeviceConfig.WorkNum & 0x00FF;
	DataWrite_To_Flash(0,12,0,ConfigData.WorkNum_Byte,2);                             //将已工作的次数写入FLASH		
	
  #if DEBUG_TEST	 

  printf("\r\nCollectPeriod:%d-----SendCount:%d\r\n",DeviceConfig.CollectPeriod,DeviceConfig.SendCount);    //测试使用
	printf("SLEEP OK!\r\n%dmin later wakeup!!",SendCount);	        //应该考虑两个参数乘积
	#endif
	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR , ENABLE);
//	PWR_WakeUpPinCmd(ENABLE); //使能WAKE-UP管脚
	 PWR_EnterSTANDBYMode();	

}


