
#include "SensorInit.h"
#include "test.h"
#include "SPI_Flash.h"

/**********************************将配置信息人工写入FLASH内*******************************/

void SensorInit(void)
{
	
     write_collect_parameter_to_flash();         //测试函数   进行设备采集参数的初始化设置
     //Set_Time();																 //测试函数   进行设备时间的设置
     //Set_DS2780();
}




