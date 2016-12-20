 /**
  ******************************************************************************
  * @file    bsp_xxx.c
  * @author  STMicroelectronics
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   spi flash �ײ�Ӧ�ú���bsp 
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� ISO-MINI STM32 ������ 
  * ��̳    :http://www.chuxue123.com
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "SPI_Flash.h"
#include "bsp_SysTick.h"
#include "bsp_usart.h"

/* Private typedef -----------------------------------------------------------*/
//#define SPI_FLASH_PageSize         4096
#define SPI_FLASH_PageSize           256
#define SPI_FLASH_PerWritePageSize   256

/* Private define ------------------------------------------------------------*/
#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg		    0x05 
#define W25X_WriteStatusReg		    0x01 
#define W25X_ReadData			        0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02 
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase		      0x20 
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	    0xAB 
#define W25X_DeviceID			        0xAB             //ԭʼ���ݣ�
#define W25X_ManufactDeviceID   	0x17             //ԭʼ���ݣ�0x90  
#define W25X_JedecDeviceID		    0x9F 

#define WIP_Flag                  0x01  /* Write In Progress (WIP) flag */
#define Dummy_Byte                0xFF


//***********************************************************************
typedef enum { FAILED = 0, PASSED = !FAILED} TestStatus;
/* ��ȡ�������ĳ��� */
//#define TxBufferSize1   (countof(TxBuffer1) - 1)
//#define RxBufferSize1   (countof(TxBuffer1) - 1)
#define countof(a)      (sizeof(a) / sizeof(*(a)))
#define BufferSize      (countof(Tx_Buffer))

#define  FLASH_WriteAddress     0x00000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress
//#define  sFLASH_ID                0xEF4017	 //W25Q64
#define  sFLASH_ID                  0xEF4018	 //W25Q128BV
     
/* ���ͻ�������ʼ�� */
uint8_t Tx_Buffer[10] = {0x00};
/* ���ջ�������ʼ�� */
uint8_t Rx_Buffer[BufferSize];

__IO uint32_t DeviceID = 0;
__IO uint32_t FlashID = 0;
__IO TestStatus TransferStatus1 = FAILED;

// ����ԭ������
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
//******************************************************************************
/*******************************************************************************
* Function Name  : AddrMapping
* Description    : ��ȡ������Flash�д洢�ĵ�ַ��Ϣ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u32 AddrMapping(u32 block,u32 sector,u32 page)
{
  u32 result;
	
	if(block>0xFF)                                   //Flash���������������256��block
	{
      printf("block max :256!!");
			block = 0xFF;
  }
  if(sector>0x0F)                                  //Flash��������һ��block�������16��sector
  {
      printf("sector max :16!!");
			sector = 0x0F;
  }
  if(page>0x0F)                                    //Flash��������һ��sector�������16��page
  {
      printf("page max :16!!");
			page = 0x0F;
  }
  result = (block<<16) + (sector<<12) + (page<<8); //���������������ţ������ǰ�洢���ĵ�ַ
	return result;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable SPI1 and GPIO clocks */
  /*!< SPI_FLASH_SPI_CS_GPIO, SPI_FLASH_SPI_MOSI_GPIO, 
       SPI_FLASH_SPI_MISO_GPIO, SPI_FLASH_SPI_DETECT_GPIO 
       and SPI_FLASH_SPI_SCK_GPIO Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);

  /*!< SPI_FLASH_SPI Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
 
  
  /*!< Configure SPI_FLASH_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure SPI_FLASH_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure SPI_FLASH_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure SPI_FLASH_SPI_CS_PIN pin��PB12��: SPI_FLASH Card CS pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  /* SPI1 configuration */
  // W25X16: data input on the DIO pin is sampled on the rising edge of the CLK. 
  // Data on the DO and DIO pins are clocked out on the falling edge of CLK.
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure);

  /* Enable SPI2  */
  SPI_Cmd(SPI2, ENABLE);
}
/*******************************************************************************
* Function Name  : SPI_FLASH_SectorErase
* Description    : Erases the specified FLASH sector.
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_SectorErase(u32 SectorAddr)
{
  /* Send write enable instruction */
  SPI_FLASH_WriteEnable();
  SPI_FLASH_WaitForWriteEnd();
  /* Sector Erase */
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();
  /* Send Sector Erase instruction */
  SPI_FLASH_SendByte(W25X_SectorErase);
  /* Send SectorAddr high nibble address byte */
  SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /* Send SectorAddr medium nibble address byte */
  SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /* Send SectorAddr low nibble address byte */
  SPI_FLASH_SendByte(SectorAddr & 0xFF);
  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BulkErase
* Description    : Erases the entire FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BulkErase(void)
{
  /* Send write enable instruction */
  SPI_FLASH_WriteEnable();

  /* Bulk Erase */
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();
  /* Send Bulk Erase instruction  */
  SPI_FLASH_SendByte(W25X_ChipErase);
  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_PageWrite
* Description    : Writes more than one byte to the FLASH with a single WRITE
*                  cycle(Page WRITE sequence). The number of byte can't exceed
*                  the FLASH page size.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH,
*                    must be equal or less than "SPI_FLASH_PageSize" value.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  /* Enable the write access to the FLASH */
  SPI_FLASH_WriteEnable();

  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();
  /* Send "Write to Memory " instruction */
  SPI_FLASH_SendByte(W25X_PageProgram);
  /* Send WriteAddr high nibble address byte to write to */
  SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /* Send WriteAddr low nibble address byte to write to */
  SPI_FLASH_SendByte(WriteAddr & 0xFF);

  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
     //printf("\n\r Err: SPI_FLASH_PageWrite too large!");
  }

  /* while there is data to be written on the FLASH */
  while (NumByteToWrite--)
  {
    /* Send the current byte */
    SPI_FLASH_SendByte(*pBuffer);
    /* Point on the next byte to be written */
    pBuffer++;
  }

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferWrite
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % SPI_FLASH_PageSize;
  count = SPI_FLASH_PageSize - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

  if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
      {
        temp = NumOfSingle - count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      if (NumOfSingle != 0)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferRead
* Description    : Reads a block of data from the FLASH.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(W25X_ReadData);

  /* Send ReadAddr high nibble address byte to read from */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);

  while (NumByteToRead--) /* while there is data to be read */
  {
    /* Read a byte from the FLASH */
    *pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
u32 SPI_FLASH_ReadID(void)
{
  u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "RDID " instruction */
  SPI_FLASH_SendByte(W25X_JedecDeviceID);

  /* Read a byte from the FLASH */
  Temp0 = SPI_FLASH_SendByte(Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp1 = SPI_FLASH_SendByte(Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp2 = SPI_FLASH_SendByte(Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

  return Temp;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
u32 SPI_FLASH_ReadDeviceID(void)
{
  u32 Temp = 0;

  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "RDID " instruction */
  SPI_FLASH_SendByte(W25X_DeviceID);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  
  /* Read a byte from the FLASH */
  Temp = SPI_FLASH_SendByte(Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  return Temp;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_StartReadSequence
* Description    : Initiates a read data byte (READ) sequence from the Flash.
*                  This is done by driving the /CS line low to select the device,
*                  then the READ instruction is transmitted followed by 3 bytes
*                  address. This function exit and keep the /CS line low, so the
*                  Flash still being selected. With this technique the whole
*                  content of the Flash is read with a single READ instruction.
* Input          : - ReadAddr : FLASH's internal address to read from.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_StartReadSequence(u32 ReadAddr)
{
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(W25X_ReadData);

  /* Send the 24-bit address of the address to read from -----------------------*/
  /* Send ReadAddr high nibble address byte */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadByte
* Description    : Reads a byte from the SPI Flash.
*                  This function must be used only if the Start_Read_Sequence
*                  function has been previously called.
* Input          : None
* Output         : None
* Return         : Byte Read from the SPI Flash.
*******************************************************************************/
u8 SPI_FLASH_ReadByte(void)
{
  return (SPI_FLASH_SendByte(Dummy_Byte));
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
u8 SPI_FLASH_SendByte(u8 byte)
{
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  /* Send byte through the SPI2 peripheral */
  SPI_I2S_SendData(SPI2, byte);

  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI2);
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendHalfWord
* Description    : Sends a Half Word through the SPI interface and return the
*                  Half Word received from the SPI bus.
* Input          : Half Word : Half Word to send.
* Output         : None
* Return         : The value of the received Half Word.
*******************************************************************************/
u16 SPI_FLASH_SendHalfWord(u16 HalfWord)
{
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  /* Send Half Word through the SPI2 peripheral */
  SPI_I2S_SendData(SPI2, HalfWord);

  /* Wait to receive a Half Word */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the Half Word read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI2);
}

/*******************************************************************************
* Function Name  : SPI_FLASH_WriteEnable
* Description    : Enables the write access to the FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WriteEnable(void)
{
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Write Enable" instruction */
  SPI_FLASH_SendByte(W25X_WriteEnable);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_WaitForWriteEnd
* Description    : Polls the status of the Write In Progress (WIP) flag in the
*                  FLASH's status  register  and  loop  until write  opertaion
*                  has completed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WaitForWriteEnd(void)
{
  u8 FLASH_Status = 0;

  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Read Status Register" instruction */
  SPI_FLASH_SendByte(W25X_ReadStatusReg);

  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);	 
  }
  while ((FLASH_Status & WIP_Flag) == SET); /* Write in progress */

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
}


//�������ģʽ
void SPI_Flash_PowerDown(void)   
{ 
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Power Down" instruction */
  SPI_FLASH_SendByte(W25X_PowerDown);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
}   

//����
void SPI_Flash_WAKEUP(void)   
{
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Power Down" instruction */
  SPI_FLASH_SendByte(W25X_ReleasePowerDown);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();                   //�ȴ�TRES1
}   
   
/*
 * ��������Buffercmp
 * ����  ���Ƚ������������е������Ƿ����
 * ����  ��-pBuffer1     src������ָ��
 *         -pBuffer2     dst������ָ��
 *         -BufferLength ����������
 * ���  ����
 * ����  ��-PASSED pBuffer1 ����   pBuffer2
 *         -FAILED pBuffer1 ��ͬ�� pBuffer2
 */
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while(BufferLength--)
  {
    if(*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }
  return PASSED;
}
///*
// * ��������FlashTest
// * ����  ��Flash���Ժ���
// * ����  ����
// * ���  ����
// */
//void FlashTest(void)
//{ 	

//	u8 i=0;
//	/* 16M����flash W25Q128��ʼ�� */
//	SPI_FLASH_Init();
//	/* Get SPI Flash Device ID */
//	DeviceID = SPI_FLASH_ReadDeviceID();
//  Delay_us(800);                        //�д�ȷ��
//	/* Get SPI Flash ID */
//	FlashID = SPI_FLASH_ReadID();
//	printf("\r\n FlashID is 0x%x,  Manufacturer Device ID is 0x%x\r\n", FlashID, DeviceID);
//	
//	/* Check the SPI Flash ID */
//	if (FlashID == sFLASH_ID)  /* #define  sFLASH_ID  0xEF4017 */
//	{	
//		printf("\r\n ��⵽�����flash W25Q64 !\r\n");
//		/* Erase SPI FLASH Sector to write on */
//		SPI_FLASH_SectorErase(FLASH_SectorToErase);	 
//    /* �����ͻ�����������д��flash�� */
//		SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress, BufferSize);		
////		SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, 1);//����ʹ��
////		if(Rx_Buffer[0]!=255)
////		{
////			/* �����ͻ�����������д��flash�� */
////			SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress+BufferSize*2, BufferSize);
////    }
////		
////		
//////		printf("\r\n д�������Ϊ��");             //����ʹ��
//////		for(i=0;i<BufferSize;i++)               
//////		{
//////		  printf("%d  ", Tx_Buffer[i]);           //����ʹ��
//////		}                                     
////		
//    /* ���ո�д������ݶ������ŵ����ջ������� */
//		SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);
//		printf("\r\n ����������Ϊ:");
//		for(i=0;i<BufferSize;i++)
//		{
//			printf("%d  ", Rx_Buffer[i]);
//		}
//		/* ���д�������������������Ƿ���� */
//		TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);
//		
//		if( PASSED == TransferStatus1 )
//		{    
//			printf("\r\n 16M����flash(W25Q128BV)���Գɹ�!\n\r");
//		}
//		else
//		{        
//			printf("\r\n 16M����flash(W25Q128BV)����ʧ��!\n\r");
//		}
//	}// if (FlashID == sFLASH_ID)
//	else
//	{    
//		printf("\r\n ��ȡ���� W25Q128BV ID!\n\r");
//	}
//	SPI_Flash_PowerDown();  

//}
/******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DataRead_From_Flash(u32 BlockIndex, u32 SectorIndex, u32 PageIndex, u8* pBuffer, u16 NumByteToRead)
{
		u32 Addr;   //Flash�д洢ָ�����ݵĵ�ַ
		 
	  Addr = AddrMapping(BlockIndex, SectorIndex, PageIndex);  //����Flash������������ȡFlash���ݴ洢��ַ
		SPI_FLASH_Init();
		/* Get SPI Flash Device ID */
		DeviceID = SPI_FLASH_ReadDeviceID();
		Delay_us(800);                          //�д�ȷ��
		/* Get SPI Flash ID */
		FlashID = SPI_FLASH_ReadID();
		printf("\r\n FlashID is 0x%x,  Manufacturer Device ID is 0x%x\r\n", FlashID, DeviceID);  //����ʹ��
		
		/* Check the SPI Flash ID */
		if (FlashID == sFLASH_ID)  /* #define  sFLASH_ID  0xEF4018 */
		{	
			printf("\r\nSPI Flash W25Q128BV is ready to read!\r\n");   //����ʹ��
			SPI_FLASH_BufferRead(pBuffer, Addr, NumByteToRead);
		}
		SPI_Flash_PowerDown();  
}
/******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DataWrite_To_Flash(u32 BlockIndex, u32 SectorIndex, u32 PageIndex, u8* pBuffer, u16 NumByteToWrite)
{
		u32 Addr;   //Flash�д洢ָ�����ݵĵ�ַ
		 
	  Addr = AddrMapping(BlockIndex, SectorIndex, PageIndex);  //����Flash������������ȡFlash���ݴ洢��ַ
		SPI_FLASH_Init();
		/* Get SPI Flash Device ID */
		DeviceID = SPI_FLASH_ReadDeviceID();
		Delay_us(800);                          //�д�ȷ��
		/* Get SPI Flash ID */
		FlashID = SPI_FLASH_ReadID();
		printf("\r\n FlashID is 0x%x,  Manufacturer Device ID is 0x%x\r\n", FlashID, DeviceID);  //����ʹ��
		/* Check the SPI Flash ID */
		if (FlashID == sFLASH_ID)  /* #define  sFLASH_ID  0xEF4018 */
		{	
			printf("\r\nSPI Flash W25Q128BV is ready to write!\r\n");   //����ʹ��
			SPI_FLASH_SectorErase( Addr );    
			/* �����ͻ�����������д��flash�� */
			SPI_FLASH_BufferWrite(pBuffer, Addr, NumByteToWrite);	
			Delay_us(200);
		}
		SPI_Flash_PowerDown();  
}
	
	
	





























///*******************************************************************************
//* Function Name  : XX
//* Description    : XX
//* Input          : ���ݴ洢��Flash�е�ƫ�Ƶ�ַ
//* Output         : None
//* Return         : �������ϵ�������磺���ڵ��Ϊ19Ah���տ�ʼ������Ϊ19*N������ֵ��ΪN-1��N���ŵ�ص��������ģ��𽥼�С��ֱ��0
//*******************************************************************************/
//uint8_t BatteryCapacityRead_From_Flash((u32 BlockIndex,u32 SectorIndex,u32 PageIndex)
//{
//  uint8_t  BatteryCapacityFactor =0x00; //�������ϵ��
//   
//	
//	SPI_FLASH_Init();
//	/* Get SPI Flash Device ID */
//	DeviceID = SPI_FLASH_ReadDeviceID();
//  Delay_us(800);                          //�д�ȷ��
//	/* Get SPI Flash ID */
//	FlashID = SPI_FLASH_ReadID();
//	printf("\r\n FlashID is 0x%x,  Manufacturer Device ID is 0x%x\r\n", FlashID, DeviceID);  //����ʹ��
//	
//	/* Check the SPI Flash ID */
//	if (FlashID == sFLASH_ID)  /* #define  sFLASH_ID  0xEF4017 */
//	{	
//		printf("\r\n ��⵽�����flash W25Q64 !\r\n");   //����ʹ��
//		
//		SPI_FLASH_BufferRead(Rx_Buffer, (FLASH_ReadAddress + ExcursionAddress), 2);
//	}
//	SPI_Flash_PowerDown(); 
//  BatteryCapacityFactor	= Rx_Buffer[0]*256 +  Rx_Buffer[1];
//	printf("\r\n BatteryCapacityFactor:%x!\r\n", BatteryCapacityFactor);     //����ʹ��
//	
//	return  BatteryCapacityFactor;
//}



///*******************************************************************************
//* Function Name  : XX
//* Description    : XX
//* Input          :  ���ݴ洢��Flash�е�ƫ�Ƶ�ַ, �������ϵ��
//* Output         : None
//* Return         : None
//*******************************************************************************/
//void BatteryCapacityWrite_to_Flash(uint16_t ExcursionAddress, uint16_t BT_CapacityFactor)
//{
//  SPI_FLASH_Init();
//	/* Get SPI Flash Device ID */
//	DeviceID = SPI_FLASH_ReadDeviceID();
//  Delay_us(800);                          //�д�ȷ��
//	/* Get SPI Flash ID */
//	FlashID = SPI_FLASH_ReadID();
//	printf("\r\n FlashID is 0x%x,  Manufacturer Device ID is 0x%x\r\n", FlashID, DeviceID);
//	
//	/* Check the SPI Flash ID */
//	if (FlashID == sFLASH_ID)  /* #define  sFLASH_ID  0xEF4017 */
//	{	
//		printf("\r\n ��⵽�����flash W25Q64 !\r\n");   //����ʹ��
//		SPI_FLASH_SectorErase(FLASH_SectorToErase);	      //�д���һ������
//    /* �����ͻ�����������д��flash�� */
//		Tx_Buffer[0] = BT_CapacityFactor>>8;
//		Tx_Buffer[1] = BT_CapacityFactor &0xFF;
//	  SPI_FLASH_BufferWrite(Tx_Buffer, (FLASH_WriteAddress+ExcursionAddress), 2);		
//		Delay_us(200); 
////		SPI_FLASH_BufferRead(Rx_Buffer, (FLASH_ReadAddress + ExcursionAddress), 2); //����ʹ��	
//	}
//	SPI_Flash_PowerDown(); 
// 
//}




//void BatteryCapacityWrite_to_Flash(uint16_t BT_CapacityFactor)
//{
//   u32 Addr;
//	Addr = AddrMapping(0,3,0);
//	SPI_FLASH_Init();
//	/* Get SPI Flash Device ID */
//	DeviceID = SPI_FLASH_ReadDeviceID();
//  Delay_us(800);                          //�д�ȷ��
//	/* Get SPI Flash ID */
//	FlashID = SPI_FLASH_ReadID();
//	printf("\r\n FlashID is 0x%x,  Manufacturer Device ID is 0x%x\r\n", FlashID, DeviceID);
//	
//	/* Check the SPI Flash ID */
//	if (FlashID == sFLASH_ID)  /* #define  sFLASH_ID  0xEF4017 */
//	{	
//		printf("\r\n Write !\r\n");   //����ʹ��
//		SPI_FLASH_SectorErase(FLASH_SectorToErase);	     
//    /* �����ͻ�����������д��flash�� */
//		Tx_Buffer[0] = BT_CapacityFactor>>8;
//		Tx_Buffer[1] = BT_CapacityFactor &0xFF;
//	  SPI_FLASH_BufferWrite(Tx_Buffer,Addr,2);		
//		Delay_us(200); 
////		SPI_FLASH_BufferRead(Rx_Buffer, (FLASH_ReadAddress + ExcursionAddress), 2); //����ʹ��	
//	}
//	SPI_Flash_PowerDown(); 
// 
//}


//uint16_t BatteryCapacityRead_From_Flash(void)
//{
//  uint16_t  BatteryCapacityFactor =0x0000; //�������ϵ��
//	u32 Addr;
//  Addr = AddrMapping(0,3,0);
//	SPI_FLASH_Init();
//	
//	/* Get SPI Flash Device ID */
//	DeviceID = SPI_FLASH_ReadDeviceID();
//  Delay_us(800);                          //�д�ȷ��
//	/* Get SPI Flash ID */
//	FlashID = SPI_FLASH_ReadID();
//	printf("\r\n FlashID is 0x%x,  Manufacturer Device ID is 0x%x\r\n", FlashID, DeviceID);
//	
//	/* Check the SPI Flash ID */
//	if (FlashID == sFLASH_ID)  /* #define  sFLASH_ID  0xEF4017 */
//	{	
//		printf("\r\n Read !\r\n");   //����ʹ��
//		
//		SPI_FLASH_BufferRead(Rx_Buffer,Addr, 2);
//	}
//	SPI_Flash_PowerDown(); 
//  BatteryCapacityFactor	= Rx_Buffer[0]*256 +  Rx_Buffer[1];
//	printf("\r\n BatteryCapacityFactor:%x!\r\n", BatteryCapacityFactor);     //����ʹ��
//	
//	return  BatteryCapacityFactor;
//}
///*******************************************************************************
//* Function Name  : XX
//* Description    : XX
//* Input          : ���ݴ洢��Flash�е�ƫ�Ƶ�ַ
//* Output         : None
//* Return         : ѭ������
//*******************************************************************************/
//void TempNoiseDataBkp(void)
//{
//  //uint16_t  BatteryCapacityFactor =0x0000; //�������ϵ��
//	u32 Addr;
//  Addr = AddrMapping(0,2,0);
//	SPI_FLASH_Init();
//	
//	/* Get SPI Flash Device ID */
//	DeviceID = SPI_FLASH_ReadDeviceID();
//  Delay_us(800);                          //�д�ȷ��
//	/* Get SPI Flash ID */
//	FlashID = SPI_FLASH_ReadID();
//	printf("\r\n FlashID is 0x%x,  Manufacturer Device ID is 0x%x\r\n", FlashID, DeviceID);
//	
//	/* Check the SPI Flash ID */
//	if (FlashID == sFLASH_ID)  /* #define  sFLASH_ID  0xEF4017 */
//	{	
//		printf("\r\n Write !\r\n");   //����ʹ��
//		SPI_FLASH_SectorErase(Addr);	
//		SPI_FLASH_BufferWrite(Dense_Data,Addr,Noise_Count*2);
//		Delay_us(200); 
//	}
//	SPI_Flash_PowerDown(); 
//  //BatteryCapacityFactor	= Rx_Buffer[0]*256 +  Rx_Buffer[1];
//	//printf("\r\n BatteryCapacityFactor:%x!\r\n", BatteryCapacityFactor);     //����ʹ��
//	
//	//return  BatteryCapacityFactor;
//}

///*******************************************************************************
//* Function Name  : XX
//* Description    : XX
//* Input          : ���ݴ洢��Flash�е�ƫ�Ƶ�ַ
//* Output         : None
//* Return         : ѭ������
//*******************************************************************************/
//void TempNoiseDataRead(void)
//{
//  //uint16_t  BatteryCapacityFactor =0x0000; //�������ϵ��
//	u32 Addr;
//  Addr = AddrMapping(0,2,0);
//	SPI_FLASH_Init();
//	
//	/* Get SPI Flash Device ID */
//	DeviceID = SPI_FLASH_ReadDeviceID();
//  Delay_us(800);                          //�д�ȷ��
//	/* Get SPI Flash ID */
//	FlashID = SPI_FLASH_ReadID();
//	printf("\r\n FlashID is 0x%x,  Manufacturer Device ID is 0x%x\r\n", FlashID, DeviceID);
//	
//	/* Check the SPI Flash ID */
//	if (FlashID == sFLASH_ID)  /* #define  sFLASH_ID  0xEF4017 */
//	{	
//		printf("\r\n Read !\r\n");   //����ʹ��
//		//SPI_FLASH_SectorErase(Addr);	
//		SPI_FLASH_BufferRead(Dense_Data,Addr,Noise_Count*2);
//		Delay_us(200); 
//	}
//	SPI_Flash_PowerDown(); 
//  //BatteryCapacityFactor	= Rx_Buffer[0]*256 +  Rx_Buffer[1];
//	//printf("\r\n BatteryCapacityFactor:%x!\r\n", BatteryCapacityFactor);     //����ʹ��
//	
//	//return  BatteryCapacityFactor;
//}



/*********************************************END OF FILE**********************/
