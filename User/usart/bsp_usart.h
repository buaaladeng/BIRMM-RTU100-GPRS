#ifndef __BSP_USART_H
#define	__BSP_USART_H

#include "stm32f10x.h"
#include <stdio.h>

#define USART1_DR_Base    0x40013804		//USART1���ݼĴ�������ַ�� 0x40013800 + 0x04 = 0x40013804
#define USART2_DR_Base    0x40004404		//USART2���ݼĴ�������ַ�� 0x40004400 + 0x04 = 0x40004404
#define USART3_DR_Base    0x40004804		//USART3���ݼĴ�������ַ�� 0x40004800 + 0x04 = 0x40004804
#define SENDBUFF_SIZE     500     //���65535
#define RECEIVEBUFF_SIZE  500    //���65535

///////////////////////////////////////////////////////////////////////////////
//�����ṹ��
struct DMA_USART3_RecevConfig 
{
	int  CurrentDataStartNum;     //��ǰ�������ݿ�ʼλ��
	int  CurrentDataEndNum;       //��ǰ�������ݽ���λ��
	int  NextDataStartNum;        //��һ�ν������ݿ�ʼλ��
	int  DMA_RecevCount;
};


void USART3_DMA_Config(void);
void USART1_Config(void);
void USART2_Config(void);
void USART3_Config(void);
void USART4_Config(void);
void USART5_Config(void);
void UART_NVIC_Configuration(void);
int  fputc(int ch, FILE *f);
int  fgetc(FILE *f);
#define  DIR485_Receive()  GPIO_ResetBits(GPIOC,GPIO_Pin_9)
#define  DIR485_Send()     GPIO_SetBits(GPIOC,GPIO_Pin_9)

#endif /* __USART1_H */
