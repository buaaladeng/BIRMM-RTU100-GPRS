#include "stm32f10x.h"
#include "SPI_Flash.h"
#include "AiderProtocol.h" 
#include "common.h"
#include "bsp_rtc.h"


#include "gprs.h"
#include "bsp_SysTick.h"
#include "modbus.h"
#include "string.h"
#include "433_Wiminet.h"
#include "API-Platform.h"

void write_collect_parameter_to_flash(void);      //进行采集信息的配置
void write_3G_parameter_to_flash(void);          //进行3G模块的配置
void write_433_parameter_to_flash(void);         //进行433模块进行配置
void Set_Time(void);                             //进行设备时间的初始化
void SetData(u8* pSendBuff, u8* pDeviceID, u16 NodeAddress);      //模拟上位机进行测试

void test_sensor_to_flash(void);