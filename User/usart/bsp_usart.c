/**
  ******************************************************************************
  * @file    bsp_usart1.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   usartӦ��bsp
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� iSO-MINI STM32 ������ 
  * ��̳    :http://www.chuxue123.com
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
#include "stm32f10x.h"
#include "bsp_usart.h"
extern void Delay(uint32_t nCount);

uint8_t SendBuff[SENDBUFF_SIZE];
uint8_t DMA_USART3_RecevBuff[RECEIVEBUFF_SIZE];
struct  DMA_USART3_RecevConfig   DMA_USART3_RecevIndicator; 
/**
  * @brief  USART1 GPIO ����,����ģʽ���á�9600 8-N-1
  * @param  ��
  * @retval ��
  */
void USART1_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* config USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	
	/* USART1 GPIO config */
	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  /* Configure PowerEN_433M(PA.07) as output push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Configure PowerEN_12V(PA.08) as output push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  /* Configure PowerEN_485(PA.11) as output push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Configure PowerEN_Flash(PA.12) as output push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = 9600;    //Һλ��ͨ�Ų�����Ĭ��9600
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	/* ʹ�ܴ���1�����ж� */
//	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);         //�ر�UART1���ڽ����ж�
	USART_ClearITPendingBit(USART1,USART_IT_TC);
	USART_Cmd(USART1, ENABLE);
}

/**
  * @brief  USART5 GPIO ����,����ģʽ���á�9600 8-N-1
  * @param  ��
  * @retval ��
  */
void USART5_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* config USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB1Periph_UART5 | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
	
	/* USART1 GPIO config */
	/* Configure USART1 Tx (PC.12) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);    
	/* Configure USART1 Rx (PD.02) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = 9600;    //Һλ��ͨ�Ų�����Ĭ��9600
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);
	
	/* ʹ�ܴ���1�����ж� */
//	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);         //�ر�UART1���ڽ����ж�
	USART_ClearITPendingBit(UART5,USART_IT_TC);
	USART_Cmd(UART5, ENABLE);
}

/**
  * @brief  USART2 GPIO ����,����ģʽ���á�9600 8-N-1 ����433ͨ�ŷ�ʽ
  * @param  ��
  * @retval ��
  */
void USART2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* config USART2 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	/* config GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	/* USART2 GPIO config */
	/* Configure USART2 Tx (PA.02) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	/* Configure USART2 Rx (PA.03) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* USART2 mode config */
	USART_InitStructure.USART_BaudRate = 9600;    //433ͨ�Ų�����Ĭ��9600
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	
	/* ʹ�ܴ���2�����ж� */
//	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);       //�ر�UART2���ڽ����ж�
//	USART_ClearITPendingBit(USART2,USART_IT_TC);
	USART_Cmd(USART2, ENABLE);
}

/**
  * @brief  USART3 GPIO ����,����ģʽ���á�115200 8-N-1    ����3Gģ��ͨ�ŷ�ʽ
  * @param  ��
  * @retval ��
  */
void USART3_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* config USART3 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	
	/* USART3 GPIO config */
	/* Configure USART3 Tx (PB.10) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);    
	/* Configure USART3 Rx (PB.11) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* Configure SIM5216 POWER_ON (PC.03) as output push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* USART3 mode config */
	USART_InitStructure.USART_BaudRate = 115200;         //3Gģ��ͨ�Ų�����Ĭ��115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	
	/* ʹ�ܴ���3�����ж� */
//	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
//	USART_ClearITPendingBit(USART3,USART_IT_TC);
  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);  
	USART_Cmd(USART3, ENABLE);

}

 /**
  * @brief  UART4 GPIO ����,����ģʽ���á�38400 8-N-1        ���ڴ�����ͨ�ŷ�ʽ
  * @param  ��
  * @retval ��
  */
void USART4_Config(void)     
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	

		/* config USART4 clock */

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO , ENABLE);

	/* UART4&485 GPIO config */
	/* Configure 485 DIR (PC.09) as output push-pull  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);   
	/* Configure USART4 Tx (PC.10) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);    
	/* Configure USART4 Rx (PC.11) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/* USART4 mode config */
	USART_InitStructure.USART_BaudRate = 38400;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
	
	/* ʹ�ܴ���4�����ж� */
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
  USART_ClearITPendingBit(UART4,USART_IT_TC);
	USART_Cmd(UART4, ENABLE);

}


/// ����UART�����ж�
void UART_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
 /* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the USART3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the UART4 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;	     
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;    //����ͨ���Ժ��ڶമ��ͨ��Ӧ����Ӧ����������
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

/* Enable the UART5 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
  

}

/**
  * @brief  USART1 TX DMA ���ã��ڴ浽����(USART1->DR)
  * @param  ��
  * @retval ��
  */
void USART3_DMA_Config(void)
{
		DMA_InitTypeDef DMA_InitStructure;
	
		/*����DMAʱ��*/
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	
		//NVIC_Config();	   			//����DMA�ж�
		/*����DMAԴ���������ݼĴ�����ַ*/
		DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Base;	   
		/*�ڴ��ַ(Ҫ����ı�����ָ��)*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_USART3_RecevBuff;
		/*���򣺴����赽�ڴ�*/		
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	    
//		/*���򣺴��ڴ浽����*/		
//		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;	        
//		/*�����СDMA_BufferSize=SENDBUFF_SIZE*/	
//		DMA_InitStructure.DMA_BufferSize = SENDBUFF_SIZE;
	  /*�����СDMA_BufferSize = RECEIVEBUFF_SIZE*/	
		DMA_InitStructure.DMA_BufferSize = RECEIVEBUFF_SIZE;
		/*�����ַ����*/	    
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
		/*�ڴ��ַ����*/
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	
		/*�������ݵ�λ*/	
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		/*�ڴ����ݵ�λ 8bit*/
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	 
		/*DMAģʽ������ѭ��*/
		//DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	    
		/*���ȼ�����*/	
		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;      
//		/*���ȼ����ǳ���*/	
//		DMA_InitStructure.DMA_Priority = DMA_Priority_High;       //DMA_Priority_High
		/*��ֹ�ڴ浽�ڴ�Ĵ���	*/
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		/*����DMA1��3ͨ��*/		   
		DMA_Init(DMA1_Channel3, &DMA_InitStructure); 	              //USART3����ͨ��Ϊ��DMA1_Channel3
		/*ʹ��DMA*/
		DMA_Cmd (DMA1_Channel3,ENABLE);					
		//DMA_ITConfig(DMA1_Channel3,DMA_IT_TC,ENABLE);  //����DMA������ɺ�����ж�
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
		DMA_USART3_RecevIndicator.CurrentDataStartNum =0;  //��ʼ����ǰ�������ݿ�ʼλ��
		DMA_USART3_RecevIndicator.CurrentDataEndNum =0;    //��ʼ����ǰ�������ݽ���λ��
		DMA_USART3_RecevIndicator.NextDataStartNum =0;     //��ʼ����һ�ν������ݿ�ʼλ��
		
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///**
//  * @brief  Configures the DMA.
//  * @param  None
//  * @retval None
//  */
//void DMA_Configuration(void)
//{
//  DMA_InitTypeDef DMA_InitStructure;

//  /* USART1_Tx_DMA_Channel (triggered by USART1 Tx event) Config */
//  DMA_DeInit(USART1_Tx_DMA_Channel);
//  DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
//  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)TxBufferUart1;
//  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
//  DMA_InitStructure.DMA_BufferSize = 0;
//  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
//  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//  DMA_Init(USART1_Tx_DMA_Channel, &DMA_InitStructure);
//        
//  /* USART1_Rx_DMA_Channel (triggered by USART1 Rx event) Config */
//  DMA_DeInit(USART1_Rx_DMA_Channel);
//  DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
//  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RxBufferUart1;
//  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//  DMA_InitStructure.DMA_BufferSize =64;//?????????????
//  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
//  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//  DMA_Init(USART1_Rx_DMA_Channel, &DMA_InitStructure);
//  DMA_Cmd(USART1_Rx_DMA_Channel, ENABLE);              
//        
//}




//void USART1_DMA_Config(void)  
//{  
//    DMA_InitTypeDef DMA_InitStructure;  

//        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);//??DMA??  
//        DMA_InitStructure.DMA_Channel = DMA_Channel_4;//??4   
//        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)USART1_DR_Address; //?????[#define USART1_DR_Address    (0x40011000+0x04)]  
//        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Data_Buffer; //????  
//        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//?????  
//        DMA_InitStructure.DMA_BufferSize = 65535; //????  
//        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//??????  
//        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//?????  
//        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//??????1byte  
//        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//??????1byte  
//        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//????  
//        DMA_InitStructure.DMA_Priority = DMA_Priority_High;//????  
//        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;// ??fifo  
//        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; //  
//        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//  
//        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//  
//        DMA_Init(DMA2_Stream5, &DMA_InitStructure);//???dma2?5  
//        DMA_Cmd(DMA2_Stream5, ENABLE);//??dma2?5  
//}  





/// �ض���c�⺯��printf��USART1
int fputc(int ch, FILE *f)
{
		/* ����һ���ֽ����ݵ�USART1 */
	 
		USART_SendData(USART1, (uint8_t) ch);
		/* �ȴ�������� */
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		{
        ;
    }			
		return (ch);
}


/// �ض���c�⺯��printf��UART5
//int fputc(int ch, FILE *f)
//{
//		/* ����һ���ֽ����ݵ�USART1 */
//	 
//		USART_SendData(UART5, (uint8_t) ch);
//		/* �ȴ�������� */
//		while (USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET)
//		{
//        ;
//    }			
//		return (ch);
//}


// �ض���c�⺯��scanf��USART1
int fgetc(FILE *f)
{
		/* �ȴ�����1�������� */
	  int  temp;
	  
		while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
		{
			;
    }

    temp=(int)USART_ReceiveData(USART1);
		return temp;
}

/// �ض���c�⺯��scanf��USART5
//int fgetc(FILE *f)
//{
//		/* �ȴ�����1�������� */
//	  int  temp;
//	  
//		while (USART_GetFlagStatus(UART5, USART_FLAG_RXNE) == RESET)
//		{
//			;
//    }

//    temp=(int)USART_ReceiveData(UART5);
//		return temp;
//}
/*********************************************END OF FILE**********************/
