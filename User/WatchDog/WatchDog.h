#ifndef __WATCHDOG_H
#define __WATCHDOG_H

#include "stm32f10x.h"

//void IWDG_Config(void);
//void WWDG_Config(void);
//void WWDG_NVIC_Configuration(void);
void WWDG_Init(u8 tr,u8 wr,u32 fprer);
//void WatchDog_Init(u8, u16);     //�������Ź���ʼ������
//void WatchDog_Feed(void);        //����ι������

#endif /* __WATCHDOG_H */
