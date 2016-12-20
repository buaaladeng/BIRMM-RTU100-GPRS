#ifndef _AIDERPROTOCOL_H_
#define _AIDERPROTOCOL_H_

#define MAX 20                         //   ���OID�������
#define MAX_COLLECTNUM  15             //���ɼ����ݵ�����


#include "stm32f10x.h"
#include "API-Platform.h"
#include "stdio.h" 
 
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#if defined ( __CC_ARM ) 

// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#pragma push

// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#pragma pack( 1 )

// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#endif  
 
union  Hex_Float
  {
   u8       Data_Hex[4];           
	 float    Data_Float;
 };                                          //�ַ������븡�����໥ת��
 
 
 
struct liquid_set 
{
	uint16_t  CollectPeriod;         //�ɼ����            
	uint16_t  SendCount;            //ÿ���ϱ����ϱ�����    
	uint16_t  CollectStartTime;     //��ʼ�ɼ�ʱ��          
	u8        CollectNum;           //����ɵĲɼ�����              �ϱ����ݵĸ��� = �ϱ�����/�ɼ����
	u8        RetryNum;             //�ش�����
  u8        BatteryCapacity;        //���������⣬����xx%����Ϊ�ٷֺ�ǰ��Ĳ��֡�
	uint16_t  WorkNum;                //ϵͳ�ѹ�������
	u8        Time_Sec;
	u8   			Time_Min;
	u8   			Time_Hour;
	u8   			Time_Mday;
	u8   			Time_Mon;
	u8   			Time_Year;

	u8        MessageSetFlag;         //����ָʾ��ǰ�Ƿ�ͨ�������޸Ĳ�������
	
	char      AlarmPhoneNum[16];      //�����绰����
	char      ServerIP[16];           //������IP
	char      ServerPort[6];          //�������˿�
	
  union     Hex_Float LowAlarmLevel;            //�ͱ���Ũ����ֵ
	union     Hex_Float HighAlarmLevel;           //�߱���Ũ����ֵ

};

struct SenserData 
{

	u16               CollectTime;           //ÿ�����ݶ�Ӧ�Ĳɼ�ʱ�䣬��ÿ�����Ϊ�ο�����λ����
	union Hex_Float   Ch4Data;               //�ɼ���������Ũ������     ������15������
	u8                DataCount;             //�ɼ�������������

};

struct ReadDataFromFlash 
{

	u16               CollectTime[MAX_COLLECTNUM];           //ÿ�����ݶ�Ӧ�Ĳɼ�ʱ�䣬��ÿ�����Ϊ�ο�����λ����
	union Hex_Float   Ch4Data[MAX_COLLECTNUM];               //�ɼ���������Ũ������     ������15������
	u8                DataCount;                            //�ɼ�������������

};


/***********************�������Ͷ���**************************/
typedef enum 
{
	GETREQUEST      	= 1,    //����������
	GETRESPONSE 	    = 2,    //�豸��ӦSET/GET����
	SETREQUEST		    = 3,   //�����豸��������
	TRAPREQUEST 	    = 4,   //�豸�����ϱ�
	TRAPRESPONSE		  = 5,   //�豸�����ϱ���Ӧ
	ONLINEREQUEST 	  = 6,   //�豸�������
	ONLINERESPONSE		= 7,   //�豸���������Ӧ
	STARTUPREQUEST 	  = 8,   //�豸�����ϱ���Ϣ
	STARTUPRESPONSE		= 9,   //�豸�����ϱ���Ϣ��Ӧ
	WAKEUPREQUEST 	  = 10,   //�����������豸
	WAKEUPRESPONSE		= 11,   //�豸��Ӧ����������

} CommunicateType;                  //֧�ֱ�Э����豸�������֮��Ĳ�������

/***********************ͨ�ŷ�ʽ����**************************/
typedef enum
{
	TYPE_433     = 1,     //��ʾ�������豸�������ͨ�����ݼ���������ͨ��
	TYPE_SMS     = 2,     //��ʾ�������豸���ֻ�ͨ�����Ž���ͨ��
	TYPE_GPRS    = 3,     //��ʾ�������豸�������ֱ��ͨ��
}SendType;                 //ͨ�ŷ�ʽ

/***********************OID����**************************/

typedef enum 
{
	DEF_NR       = 0x1000000A,    //�ش�����
	SYSTERM_DATA = 0x10000050,    //ϵͳ����
	SYSTERM_TIME = 0x10000051,    //ϵͳʱ��
	CLT1_ITRL1   = 0x10000105,    //һʱ���ɼ����

	UPLOAD_CYCLE = 0x10000062,    //�����ϱ�����
	DEVICE_STATE = 0x60000100,    //�豸״ָ̬ʾ���ϵ磩
	DEVICE_QTY   = 0x60000020,    //��ص���
  DEVICE_WAKEUP= 0x60000200,    //�豸״ָ̬ʾ�����ѣ�
	FRAM_STATE   = 0x60000300,     //���ݽ���״ָ̬ʾ
	
	BSS_IP			 = 0x10000022,     //�������IP��
	BSS_PORT	   = 0x10000023,     //�������˿ں�
	SMS_PHONE    = 0x10000041,     //���Ÿ澯�绰����

 	RESET_PROBER = 0x10000061,       //�豸��λ����
  DATA_REQUEST = 0x20000001,       //��������ѯ�豸��������


} CommandType;                  //�豸ͨ�Ź�����ʹ�õ���OID���ϣ�����ҵ���ϱ�OID��

//�ṹ������
// -----------------------------------------------------------------------------
// DESCRIPTION: �豸����
// -----------------------------------------------------------------------------
typedef enum 
{
	ADL_FPL       	= 1,    //�๦����©
	BIRMM_NOISE1000 = 2,    //������¼��
	BIRMM_RTU100		= 4,   //ȼ�����ܼ���ն�
	BIRMM_LEVEL1000 = 7,   //Һλ�����
	BIRMM_RL2000		= 8,   //�¶�ѹ��������
	BIRMM_WM1000		= 10,  //Զ��ˮ��
	BIRMM_CorrR1000 = 11,  //���²��¸�ʴ���ʼ����
	BIRMM_CorrE1000 = 12,  //���²��¸�ʴ�������������
	
} DeviceType;                  //֧�ֱ�Э��������豸�����Ͷ���

//�ṹ������
// -----------------------------------------------------------------------------
// DESCRIPTION: �ϱ���������
// -----------------------------------------------------------------------------
typedef enum
{
	FLOW       =1,        //����Ϊ1
	PRESSURE   =2,        //ѹ��Ϊ2
	LEVEL 		 =3,        //ҺλΪ3
	NOISE      =4,				//����Ϊ4
	GAS				 =5,        //����Ϊ5
	H_TEMPERATURE =8,       //�����¶�Ϊ8
	H_PRESSURE    =9,       //����ѹ��Ϊ9
	WATER					=10,      //ˮ��Ϊ10
	CORR_RATE     =11,      //��ʴ����
	CORR_ENV			=12,			//��ʴ����

} ReportDataType;            //�����ϴ�����
//�ṹ������
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
struct TagStruct                         //���ڴ洢������������
{
   CommandType   OID_Command;            //����OID����
   u16           Width;                  //Value��ĳ���
   u8            Value[32];              //����ֵ����WidthΪ0ʱ��Value����Բ����ڣ���ǰӦ������Value�����ռ��32�ֽ�
};

//�ṹ������
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
struct SpecialTagStruct                  //���ڴ洢ҵ���ϱ�����
{
	 u32           OID_Data;               //����OID����
   u16           Width;                  //Value��ĳ���
	 union                                 //���������壬���ں������ض��ֽڲ���
	 {
      u8         Value[4];               //ieee754��ʽ��ʾ������
		  float      Data_F;                 //��������ʽ��ʾ������
   }PerceptionData;                      //�ɼ����Ĵ���������   
	 u16           CollectTime;            //���ݲɼ�ʱ��  ��λ Ϊ ����
	
};

//�ṹ������
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
struct CommandFrame                      //��ѯ��������֡��ʽ
{
	u8           Preamble;                 //֡ǰ����
	u8           Version;                  //Э��汾��
	u16          Length;                   //��Ч���ݳ��ȣ�����Length�ֶε�CRC�ֶΰ�����ʵ�����ݳ��ȣ���λΪByte��������Length�ֶκ�CRC�ֶΣ�OID_List������ʵ��ʹ�õ����ݿռ�Ϊ׼��
	u8           DeviceID[6];              //�豸ID��
	u8           RouteFlag;                //·�ɱ�־
	u16          NodeAddr;                 //·�ɽڵ��ַ
	u16          PDU_Type;                 //����ָʾ
	u8           Seq;                      //������ţ�1~255��
	CommandType  OID_List[MAX];            //OID���У�һ֡���ݿ��԰������OID����಻����20��
	u16          CrcCode;                  //16λCRCУ���룬���㷶ΧΪ��Preamble��CrcCode��ȫ�����ݣ�����Preamble��CrcCode��CrcCode��ֵΪ0xFFFF��
	u8           OID_Count;                //OID���м�����
};

//�ṹ������
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
struct DataFrame                             //�·����ò�����֡��ʽ
{
	u8               Preamble;                 //֡ǰ����
	u8               Version;                  //Э��汾��
	u16              Length;                   //��Ч���ݳ��ȣ�����Length�ֶε�CRC�ֶΰ�����ʵ�����ݳ��ȣ���λΪByte��������Length�ֶκ�CRC�ֶΣ�OID_List������ʵ��ʹ�õ����ݿռ�Ϊ׼��
	u8               DeviceID[6];              //�豸ID��
	u8               RouteFlag;                //·�ɱ�־
	u16              NodeAddr;                 //·�ɽڵ��ַ
	u16              PDU_Type;                 //����ָʾ
	u8               Seq;                      //������ţ�1~255��
	struct TagStruct TagList[MAX];             //Tag���У�һ֡���ݿ��԰������Tag����಻����20��,����һ֡�����ܳ��ȣ�����9�ֽ�֡ͷ�����ܳ���256
	u16              CrcCode;                  //16λCRCУ���룬���㷶ΧΪ��Preamble��CrcCode��ȫ�����ݣ�����Preamble��CrcCode��CrcCode��ֵΪ0xFFFF��
	u8               Tag_Count;                //Tag���м�����
	
};

//�ṹ������
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
struct  SpecialDataFrame                     //ҵ�������ϴ�֡��ʽ
{
	u8                      Preamble;                 //֡ǰ����
	u8                      Version;                  //Э��汾��
	u16                     Length;                   //��Ч���ݳ��ȣ�����Length�ֶε�CRC�ֶΰ�����ʵ�����ݳ��ȣ���λΪByte��������Length�ֶκ�CRC�ֶΣ�OID_List������ʵ��ʹ�õ����ݿռ�Ϊ׼��
	u8                      DeviceID[6];              //�豸ID��
	u8                      RouteFlag;                //·�ɱ�־
	u16                     NodeAddr;                 //·�ɽڵ��ַ
	u16                     PDU_Type;                 //����ָʾ
	u8                      Seq;                      //������ţ�1~255��
	struct TagStruct        SysTime;                  //ϵͳʱ�䣬��ʾ��һ�����ݵĲɼ�ʱ��
	struct SpecialTagStruct TagList[MAX];             //Tag���У�һ֡���ݿ��԰������Tag����಻����20��,����һ֡�����ܳ��ȣ�����9�ֽ�֡ͷ�����ܳ���256
	struct TagStruct        BattEnergy;               //���ʣ�����������xx%����Ϊ�ٷֺ�ǰ��Ĳ��֡�
	u16                     CrcCode;                  //16λCRCУ���룬���㷶ΧΪ��Preamble��CrcCode��ȫ�����ݣ�����Preamble��CrcCode��CrcCode��ֵΪ0xFFFF��
	u8                      Tag_Count;                //Tag���м�����
	
};



// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#if defined ( __CC_ARM ) 

// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#pragma pop

// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
#endif

//void ConfigData_Init(struct liquid_set* Para);
void LSLIQUSET_Handle(char* pLiqudSet, struct liquid_set* Parameter);
void LSLIQUSET_Response(struct liquid_set* Parameter);
void LSTIMESET_Handle(char* pLiqudSet, struct liquid_set* Parameter);
void LSDataUpload_Finish(struct liquid_set* Parameter);
void DataUpload_TALK_OVER(struct liquid_set* Parameter);
void Float2Hex_Aider( float DataSmooth );

void LSLIQUID_DataUpload(struct liquid_set* Para);
void Section_Request(void);
void Section_Handle(void);

void  AlarmReport(u8* pSendBuff, u8* pDeviceID, u16 NodeAddress, struct SenserData* pObtainData);
void  TrapRequest(u8* pSendBuff, u8* pDeviceID, u16 NodeAddress, struct SenserData* pObtainData);
void GetResponse(struct CommandFrame RequestPara, u8* pSendBuff, u8* pDeviceID, u16 NodeAddress);
uint8_t  UploadFlash(char* pSetPara, uint8_t  InstructCode);

uint8_t SendDataToServ(CommunicateType CommType,struct TagStruct TagList[],uint8_t TagNum,u8* pDeviceID);
void TrapData(u8* pDevID);
unsigned int Get_Rand_Num(void);                                 //��ȡ�������
#endif

