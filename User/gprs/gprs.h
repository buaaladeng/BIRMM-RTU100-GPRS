#ifndef _GPRS_H_
#define _GPRS_H_
#include "stm32f10x.h"
#include "AiderProtocol.h"

//#define  NETERRORCOUNT    15
#define  NETLOGIN         1     //�豸ע���վ
#define  TCPCONNECT       2     //�豸����TCP����
#define  DATARECEV        3     //�豸���շ���������


///////////////////////////////////////////////////////////////////////////////
//�����ṹ��
struct SMS_Config_RegPara 
{
	
	char     CurrentPhoneNum[16];      //��ǰͨ���ֻ����룬�ַ�����ʽ
	char     AlarmPhoneNum[16];   //��������Һλ�������룬�ַ�����ʽ
	char     ServerIP[16];     //�������÷�����IP���ַ�����ʽ
	char     ServerPort[6];    //�������÷������˿ںţ��ַ�����ʽ
  
	union   Hex_Float LowAlarmLevel;
	union   Hex_Float HighAlarmLevel;
		
	
	uint8_t  CollectPeriod_Byte[2];    //���ݲɼ������ʹ������洢���Է���Flash��д
	uint8_t  SendCount_Byte[2];        //һ���ϴ�Һλ���ݴ�����ʹ������洢���Է���Flash��д  �ϴ�����
	uint8_t  CollectStartTime_Byte[2];     //��ʼ�ɼ�ʱ��
	u8       CollectNum;                  //�Ѿ���ɵĲɼ�����
	uint8_t  RetryNum;                    //�ش�����
	uint8_t  WorkNum_Byte[2];             //��������
	uint8_t  SensorDataInquireFlag;    //���Ų�ѯ��ǰ���������ݱ�־����
	uint8_t  DeviceSetInquireFlag;     //���Ų�ѯ��ǰ ������������Ϣ��־����
	
};

//��������
void  GPRS_Init(void);          //433ͨ�ŷ�ʽ���ú�����ʱ���ã�ʹ��3Gͨ��ʱ�Ŵ�
void  GPRS_Config(void);        //����3Gģ����ز���
void  TCP_Connect(void);        //����TCP����
void  TCP_Disconnect(void);     //�Ͽ�TCP����
unsigned char  TCP_StatusQuery(void);    //��ѯTCP����״̬  ����TCP״̬
void  mput(char *str);
void  mput_mix(char *str,int length);
char* Find_String(char *Source, char *Object);
char* Find_SpecialString(char* Source, char* Object, short Length_Source, short Length_Object);
//void  SIM5216_PowerOn(void);   //433ͨ�ŷ�ʽ���ú�����ʱ���ã�ʹ��3Gͨ��ʱ�Ŵ�
//void  SIM5216_PowerOff(void);  //433ͨ�ŷ�ʽ���ú�����ʱ���ã�ʹ��3Gͨ��ʱ�Ŵ�
void  Sms_Send(char*  pSend);    //���ŷ��ͺ���
void  AlarmSMS_Send(char* pSend);//���ͱ�������
void  Sms_Analysis(char* pBuff); //���Ž��ս�������
//void  Sms_Consult(void);         //����δ������
void Sms_Consult(u8* pDeviceID, u16 sNodeAddress);

void  USART_DataBlock_Send(USART_TypeDef *USART_PORT,char *SendUartBuf,u16 SendLength);   //�����򴮿ڷ�������

void  SetRequest_GPRS(void);      //ͨ��3Gģ������������·�������Ϣ
void  ConfigData_Init(struct liquid_set* Para);
unsigned char GPRS_Receive_NetLogin(void);  //ģ��ע�����������Ϣ����
unsigned char GPRS_Receive_TcpConnect(void);//����TCP���ӽ�����Ϣ����
unsigned char Receive_Deal_GPRS(void);    //3Gģ��������ݽ�������Ҫ���ڴ���MCU��3Gģ��֮��Ľ�������

unsigned char GPRS_Receive_DataAnalysis(u8* pDeviceID, u16 sNodeAddress);        //3Gģ����շ��������ݽ�������Ҫ����������·��Ļ��ڰ��¶�Э�������
void ClientRequest_GPRS(u8* pSendBuff, u8* pDeviceID, u16 NodeAddress);          //�������ն���������������·�����
void Treaty_Data_Analysis(u8* pTreatyBuff, u16* pFlag, u8* pDeviceID, u16 NodeAddress);	


uint16_t char_to_int(char* pSetPara);               //��������·������ã���Ҫ���ַ�ת��Ϊ����
float    char_to_float(char* pSetPara);               //��������·������ã���Ҫ���ַ�ת��ΪС��

void SensorSetMessage(void);                       //���Ż�ȡ�豸���ò���                

#endif

