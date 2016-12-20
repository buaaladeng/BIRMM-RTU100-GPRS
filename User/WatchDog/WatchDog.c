/**
  ******************************************************************************
  * @file    WatchDog.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   ϵͳ���Ź�   
  ******************************************************************************
  * @attention
  *
  * ��̳    :http://www.chuxue123.com
  *
  ******************************************************************************
  */
  
#include "WatchDog.h"



//����WWDG������������ֵ,Ĭ��Ϊ���. 
u8 WWDG_CNT=0X7F;
//��ʼ�����ڿ��Ź� 	
//tr   :T[6:0],������ֵ 
//wr   :W[6:0],����ֵ 
//fprer:��Ƶϵ����WDGTB��,�����2λ��Ч 
//Fwwdg=PCLK1/(4096*2^fprer). PCLK1=36Mhz
//���Ź���λ����Ϊ��������0x7F����0x40��ʱ�䣬��(4096*2^fprer)*32/36*1000    (ms)

void WWDG_Init(u8 tr,u8 wr,u32 fprer)                 //���Ź����ڣ�1.8s
{
 
	NVIC_InitTypeDef NVIC_InitStructure;
 	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG,ENABLE); //ʹ�ܴ��ڿ��Ź�ʱ��
	
	WWDG_CNT=tr&WWDG_CNT;   //��ʼ��WWDG_CNT. 
	WWDG_SetPrescaler(fprer); //���÷�Ƶֵ
	WWDG_SetWindowValue(wr); //���ô���ֵ
//	WWDG_SetCounter(WWDG_CNT);//���ü���ֵ
	WWDG_Enable(WWDG_CNT);  //�������Ź�
	
	NVIC_InitStructure.NVIC_IRQChannel=WWDG_IRQn;  //���ڿ��Ź��ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;  //��ռ���ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;					//�����ȼ�Ϊ3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;  //ʹ�ܴ��ڿ��Ź�
	NVIC_Init(&NVIC_InitStructure);
	
	WWDG_ClearFlag();//�����ǰ�����жϱ�־λ
  WWDG_EnableIT();//������ǰ�����ж�
}


///**
//  * @brief  ���ڿ��Ź����ó�ʼ����WWDG_Config
//  * @param  ��
//  * @retval ��
//  * ���ʱ����㣺Tout=(1/(PCLK1/4096/WWDG_Prer))*rlr
//  */

//void WWDG_Config(void)
//{  
//  WWDG_SetPrescaler(WWDG_Prescaler_8);          //ʱ��8��Ƶ4ms,(PCLK1/4096)/8=244Hz
//  WWDG_SetWindowValue(65);                      //��������ֵ�����¼����������ʱ��Ϊ��4ms*64=262ms
//  WWDG_Enable(127);                             //����������������ι��ʱ��
//	WWDG_ClearFlag();                             //�����־λ
//	WWDG_EnableIT();                              //�����ж�
//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
//}
//void WWDG_NVIC_Configuration(void)
//{
//	NVIC_InitTypeDef NVIC_InitStructure; 
//    
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
//	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//}

/**
  * @brief  �������Ź����ó�ʼ����IWDG_Config
  * @param  ��
  * @retval ��
  * ���ʱ����㣺Tout=((4*IWDG_Prer)*rlr)/40
  */
//void IWDG_Config(void)
//{ 

//   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //�����Ĵ�����д
//   IWDG_SetPrescaler(IWDG_Prescaler_32);          //40Kʱ��32��Ƶ
//   IWDG_SetReload(1200);                           //��������ֵ�����¼����������ʱ��Ϊ3840ms
//   IWDG_ReloadCounter();                          //����������
//   IWDG_Enable();                                 //�������Ź�
//}

///**
//  * @brief  �������Ź���ʼ����WatchDog_Init
//  * @param  prer-��Ƶ��reld-��������װ��ֵ�����¼�����
//  * @retval ��
//	* ���ʱ����㣺Tout=((4*2^prer)*rlr)/40
//  */
//void WatchDog_Init(u8 prer, u16 reld)
//{
//   IWDG->KR=0x5555; //�������PR��RLR�Ĵ���
//   IWDG->PR=prer;   //���÷�Ƶ
//   IWDG->RLR=reld;  //�趨��������ֵ�����¼���
//   IWDG->KR=0xaaaa; //����װ��ֵ
//   IWDG->KR=0xcccc; //�������Ź���ʱ��
//}

///**
//  * @brief  �������Ź�ι����WatchDog_Feed
//  * @param  ��
//  * @retval ��
//  */
//void WatchDog_Feed(void)  //�ȼ���:IWDG_ReloadCounter
//{
// IWDG->KR=0xaaaa;
//}

/*********************************************END OF FILE**********************/
