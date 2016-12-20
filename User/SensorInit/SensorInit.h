#ifndef _SENSORINIT_H_
#define _SENSORINIT_H_
#include "stm32f10x.h"
#include "stdlib.h"


#define DEVICE   BIRMM_RTU100                            //设备类型为燃气智能监测终端
#define REPORTDATA  GAS                                  //上报数据为气体浓度
#define TYPESEND   TYPE_433                              //传输类型选型433通信
#define SENSORID {0x31,0x20,0x16,0x08,0x31,0x02}         //传感器ID号
#define SOFTVERSION 0x20                                 //软件版本号
#define HARDVERSION 0x10                                 //硬件版本号
#define SETTIME {16,12,12,19,11,00}

#define CONFIG 0                                      //是否需要出厂配置参数标志
#define DEBUG_TEST 0   																//打印功能，1应用于调试 0应用于正常工作



/* Private function prototypes -----------------------------------------------*/
void SensorInit(void);                                        //设备初始化设置
#endif             
/* end of SensorInit.h */
