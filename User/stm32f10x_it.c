/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTI
  
  AL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include <stdio.h>
#include "bsp_TiMbase.h" 
#include "bsp_usart.h"


extern  char      Usart1_recev_buff[300];
extern  uint16_t  Usart1_recev_count;
//extern  uint32_t  time ;                       // ms ��ʱ����  
//extern  char      Usart2_recev_buff[50];         //USART2���ջ���
//extern  uint8_t   Usart2_recev_count;            //USART2���ռ�����
extern  char      Usart3_recev_buff[1000];
extern  uint16_t  Usart3_recev_count;
extern  uint32_t  TimeDisplay;                   //RTC��ʱ��־����
extern  u8        WWDG_CNT;                      //����WWDG������������ֵ,Ĭ��Ϊ���
extern  uint16_t  WWDOG_Feed ;                   //���ڿ��Ź���λ����Ϊ��255*1.8s = 7.6min
extern  u8        Uart4_send_buff[100]; 
extern  u8        Uart4_rev_buff[100];
extern  vs8       Uart4_send_counter;
extern  vu8       Uart4_rev_count;               //RS485���ڽ��ռ�����
extern  vu8       Uart4_rev_comflag;             //RS485���ڽ�����ɱ�־����
extern  vu8       Crc_counter;                   //RS485У�������
extern  vu8*      Uart4_send_pointer ;           //RS485���ڷ���ָ��
extern  uint8_t   DMA_UART3_RECEV_FLAG;          //USART3 DMA���ձ�־����
//extern  char  Usart1_send_buff[];
//extern  uint8_t  Usart1_send_count;
//extern  uint32_t  Tic_IWDG;
//extern  char  Usart3_send_buff[];
//extern  uint8_t  Usart3_send_count;
//extern void Delay(uint32_t nCount);

extern void TimingDelay_Decrement(void);

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	printf("\r\nWarnning!! NMI Occured\r\n");// ����ʹ��
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */

	while (1)
  {
		printf("\r\nWarnning!! Hard Fault Occured\r\n");// ����ʹ��
		break;
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
		printf("\r\nWarnning!! MemManage Occured\r\n");// ����ʹ��
		break;
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
		printf("\r\nWarnning!! BusFault Occured\r\n");// ����ʹ��
		break;
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
		printf("\r\nWarnning!! UsageFault Occured\r\n");// ����ʹ��
		break;
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
	printf("\r\nWarnning!! SVCall exception Occured\r\n");// ����ʹ��
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
	printf("\r\nWarnning!! Debug Monitor exception Occured\r\n");// ����ʹ��
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	printf("\r\nWarnning!! PendSVC exception Occured\r\n");// ����ʹ��
}


/**
  * @brief  This function handles WWDG Handler.
  * @param  None
  * @retval None
  */
//���ڿ��Ź��жϷ������ 
void WWDG_IRQHandler(void)
{
	if(WWDOG_Feed>0)
	{
		WWDG_SetCounter(WWDG_CNT);   //��װ����ֵ
		WWDG_ClearFlag();//�����ǰ�����жϱ�־λ
		WWDOG_Feed--;
	}
	
}
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)  //��ǰ���ã�10us�ж�һ��
{
	TimingDelay_Decrement();	
	
//	Tic_IWDG++;                //���Ź�ι��ʱ����������
//  if(Tic_IWDG>=50000)        //ι��ʱ��Ϊ500ms
//  {  
//    
//	 IWDG_ReloadCounter();     //ι��
//   Tic_IWDG=0;               //��λ������
//  }
}

/**
  * @brief  This function handles RTCAlarm interrupt request.
  * @param  None
  * @retval None
  */
//void RTCAlarm_IRQHandler(void)
//{				
//  //�ȴ�RTC_CTL�Ĵ����е�RSFλ(�Ĵ���ͬ����־)��Ӳ����1
//  RTC_WaitForSynchro();
//  if (RTC_GetITStatus(RTC_IT_ALR) != RESET)
//  {
//    printf("Wakeup!!");
//    EXTI_ClearITPendingBit(EXTI_Line17);          // ��EXTI_Line17����λ
//    if(PWR_GetFlagStatus(PWR_FLAG_WU) != RESET)   // ��黽�ѱ�־�Ƿ�����
//    {   
//        PWR_ClearFlag(PWR_FLAG_WU);               // ������ѱ�־
//    }

//    /* Clear the RTC Second interrupt */
////  RTC_SetCounter(0);
////  RTC_WaitForLastTask();
//    RTC_ClearITPendingBit(RTC_IT_ALR);
//    RTC_WaitForLastTask();
////    RTC_SetAlarm(RTC_GetCounter()+10);
////    RTC_WaitForLastTask();
//  }
//}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles RTC interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
	  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)       //���жϷ���
	  {
	    /* Clear the RTC Second interrupt */
	    RTC_ClearITPendingBit(RTC_IT_SEC);
	
//	    /* Enable time update */
//	    TimeDisplay = 1;
	
	    /* Wait until last write operation on RTC registers has finished */
	    RTC_WaitForLastTask();
	  }
}


///**
//  * @brief  This function handles TIM2 interrupt request.
//  * @param  None
//  * @retval None
//  */
//void TIM2_IRQHandler(void)           //1ms��ʱ������
//{
//	if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET ) 
//	{	
//		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);  	
//    time++;		
//	}		 	
//}

/**
  * @brief  This function handles TIM3 interrupt request.
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)         //20ms��ʱ������
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
    	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);    
		  TIM_Cmd(TIM3, DISABLE);                      //�ر�TIM3 
		  TIM_DeInit(TIM3);                            //��λTIM3��ʱ��
		  Uart4_rev_comflag = 1;                       //RS485������ɱ�־
    	Crc_counter = Uart4_rev_count;               //crcУ���������ֵ
    	Uart4_rev_count = 0;                         //RS485���ռ���������

	}	
}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
void USART1_IRQHandler(void)
{   
	
	  if(Usart1_recev_count == (sizeof(Usart1_recev_buff)-1))                         //USART1����Buff�������ý��ռ����������ݴ�Usart1_recev_buff[0]��ʼ�棬��ֹ����Խ�����
		{
      printf("\r\nUSART1 Receive Buff Full!!\r\n");                      //����ʹ��
			Usart1_recev_count =0;
    }
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)                //USART1���ջ�����������
		{ 
			 USART_ClearITPendingBit(USART1,USART_IT_RXNE);
			 Usart1_recev_buff[Usart1_recev_count++] = USART_ReceiveData(USART1);
       while (USART_GetITStatus(USART1, USART_FLAG_RXNE) == SET){; }     //�ȴ��������	
		} 
		if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) == SET)               //���ջ�����������������յ�������
		{				
			USART_ClearFlag(USART1, USART_FLAG_ORE);
			USART_ReceiveData(USART1);
		}
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
//	{ 	
//	    //ch = USART1->DR;
//			USART_ReceiveData(USART1);
//	}	
}	
void USART2_IRQHandler(void)
{
//	  if(Usart2_recev_count == (sizeof(Usart2_recev_buff)-1))              //USART2����Buff�������ý��ռ����������ݴ�Usart2_recev_buff[0]��ʼ�棬��ֹ����Խ�����
//		{
//      printf("\r\nUSART2 Receive Buff Full!!\r\n");                      //����ʹ��
//			Usart2_recev_count =0;
//    }
//		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)                 //USART2���ջ�����������
//		{ 
//			 USART_ClearITPendingBit(USART2,USART_IT_RXNE);
//			 Usart2_recev_buff[Usart2_recev_count++] =USART_ReceiveData(USART2);
//       while (USART_GetITStatus(USART2, USART_FLAG_RXNE) == SET){; }      //�ȴ��������	
//		} 
//		if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) == SET)                //���ջ�����������������յ�������
//		{				
//			USART_ClearFlag(USART2, USART_FLAG_ORE);
//			USART_ReceiveData(USART2);
//		}
	
}	
void USART3_IRQHandler(void)
{
    u16 i =0;  
	
	  printf("\r\nEnter USART3_IRQ!! \r\n");     //����ʹ��
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)   //���Ϊ���������ж�
    {  
       /* IDLE bit is cleared by a software sequence(an read to the USART_SR register followed by a read to the USART_DR register)*/ 
		  	i = USART3->SR;  
        i = USART3->DR;  
        USART_ClearITPendingBit(USART3, USART_IT_IDLE);  
//			  USART_ITConfig(USART3, USART_IT_IDLE, DISABLE); 
        DMA_Cmd(DMA1_Channel3, DISABLE);           //�ر�DMA��ֹ�����ڼ�������
			  DMA_UART3_RECEV_FLAG =1;                  //���ñ�־����
			  printf("\r\nOK!! \r\n");     //����ʹ��
			  DMA_Cmd(DMA1_Channel3, ENABLE);          //����DMA 
		}

}
//void USART3_IRQHandler(void)
//{
//		 /* TIM2 ���¿�ʱ�ӣ���ʼ��ʱ */
//	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
//	
//	  if(Usart3_recev_count == (sizeof(Usart3_recev_buff)-1))              //USART3����Buff�������ý��ռ����������ݴ�Usart3_recev_buff[0]��ʼ�棬��ֹ����Խ�����
//		{
//      printf("\r\nUSART3 Receive Buff Full!!\r\n");                      //����ʹ��
//			Usart3_recev_count =0;
//    }
//		if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)                  //USART3���ջ�����������
//		{ 
//			 USART_ClearITPendingBit(USART3,USART_IT_RXNE);
//			 Usart3_recev_buff[Usart3_recev_count++] =USART_ReceiveData(USART3);       
//       while (USART_GetITStatus(USART3, USART_FLAG_RXNE) == SET){; }       //�ȴ��������	
//			 
//		} 
//		if(USART_GetFlagStatus(USART3, USART_FLAG_ORE) == SET)                 //���ջ�����������������յ�������
//		{				
//			USART_ClearFlag(USART3, USART_FLAG_ORE);
//			USART_ReceiveData(USART3);
//		}
//		time=0;
//}

void UART4_IRQHandler(void)
{
		if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	  {
		   USART_ClearITPendingBit(UART4,USART_IT_RXNE);
//    	 if(Uart4_rev_comflag != 1)       
//    	 {
        	if(Uart4_rev_count == (sizeof(Uart4_rev_buff)-1))              //UART4����Buff�������ý��ռ����������ݴ�Uart4_recev_buff[0]��ʼ�棬��ֹ����Խ�����
					{
						printf("\r\nUART4 Receive Buff Full!!\r\n");                      //����ʹ��
						Uart4_rev_count =0;
					}			 
				  else
        	{       
						 Uart4_rev_buff[Uart4_rev_count++]= USART_ReceiveData(UART4);	 
						 while (USART_GetITStatus(UART4, USART_FLAG_RXNE) == SET){; }    //RS485�ȴ��������	
//			 	 	   TIM3_Configuration();                                           //�򿪽��ն�ʱ��
        	}   
//    	 }    
//     	 else
//    	 {
//        	USART_ReceiveData(UART4);   //�������û�д����������յ�����
//    	 }
   }  
	 if(USART_GetFlagStatus(UART4, USART_FLAG_ORE) == SET)   //���ջ�����������������յ�������
	 {			
			USART_ClearFlag(UART4, USART_FLAG_ORE);
			USART_ReceiveData(UART4);
	 }		
}		
		
	
	
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
 */ 
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
