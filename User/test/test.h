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

void write_collect_parameter_to_flash(void);      //���вɼ���Ϣ������
void write_3G_parameter_to_flash(void);          //����3Gģ�������
void write_433_parameter_to_flash(void);         //����433ģ���������
void Set_Time(void);                             //�����豸ʱ��ĳ�ʼ��
void SetData(u8* pSendBuff, u8* pDeviceID, u16 NodeAddress);      //ģ����λ�����в���

void test_sensor_to_flash(void);