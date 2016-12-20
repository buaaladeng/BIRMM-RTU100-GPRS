#ifndef _GPRS_H_
#define _GPRS_H_
#include "stm32f10x.h"
#include "AiderProtocol.h"

//#define  NETERRORCOUNT    15
#define  NETLOGIN         1     //设备注册基站
#define  TCPCONNECT       2     //设备建立TCP连接
#define  DATARECEV        3     //设备接收服务器数据


///////////////////////////////////////////////////////////////////////////////
//声明结构体
struct SMS_Config_RegPara 
{
	
	char     CurrentPhoneNum[16];      //当前通信手机号码，字符串格式
	char     AlarmPhoneNum[16];   //短信配置液位报警号码，字符串格式
	char     ServerIP[16];     //短信配置服务器IP，字符串格式
	char     ServerPort[6];    //短信配置服务器端口号，字符串格式
  
	union   Hex_Float LowAlarmLevel;
	union   Hex_Float HighAlarmLevel;
		
	
	uint8_t  CollectPeriod_Byte[2];    //数据采集间隔，使用数组存储，以方便Flash读写
	uint8_t  SendCount_Byte[2];        //一天上传液位数据次数，使用数组存储，以方便Flash读写  上传周期
	uint8_t  CollectStartTime_Byte[2];     //开始采集时间
	u8       CollectNum;                  //已经完成的采集数量
	uint8_t  RetryNum;                    //重传次数
	uint8_t  WorkNum_Byte[2];             //工作次数
	uint8_t  SensorDataInquireFlag;    //短信查询当前传感器数据标志变量
	uint8_t  DeviceSetInquireFlag;     //短信查询当前 传感器配置信息标志变量
	
};

//声明函数
void  GPRS_Init(void);          //433通信方式，该函数暂时无用，使用3G通信时才打开
void  GPRS_Config(void);        //配置3G模块相关参数
void  TCP_Connect(void);        //建立TCP连接
void  TCP_Disconnect(void);     //断开TCP连接
unsigned char  TCP_StatusQuery(void);    //查询TCP连接状态  返回TCP状态
void  mput(char *str);
void  mput_mix(char *str,int length);
char* Find_String(char *Source, char *Object);
char* Find_SpecialString(char* Source, char* Object, short Length_Source, short Length_Object);
//void  SIM5216_PowerOn(void);   //433通信方式，该函数暂时无用，使用3G通信时才打开
//void  SIM5216_PowerOff(void);  //433通信方式，该函数暂时无用，使用3G通信时才打开
void  Sms_Send(char*  pSend);    //短信发送函数
void  AlarmSMS_Send(char* pSend);//发送报警短信
void  Sms_Analysis(char* pBuff); //短信接收解析函数
//void  Sms_Consult(void);         //查阅未读短信
void Sms_Consult(u8* pDeviceID, u16 sNodeAddress);

void  USART_DataBlock_Send(USART_TypeDef *USART_PORT,char *SendUartBuf,u16 SendLength);   //批量向串口发送数据

void  SetRequest_GPRS(void);      //通过3G模块请求服务器下发配置信息
void  ConfigData_Init(struct liquid_set* Para);
unsigned char GPRS_Receive_NetLogin(void);  //模块注册网络接收信息解析
unsigned char GPRS_Receive_TcpConnect(void);//建立TCP连接接收信息解析
unsigned char Receive_Deal_GPRS(void);    //3G模块接收数据解析，主要用于处理MCU与3G模块之间的交互数据

unsigned char GPRS_Receive_DataAnalysis(u8* pDeviceID, u16 sNodeAddress);        //3G模块接收服务器数据解析，主要处理服务器下发的基于埃德尔协议的数据
void ClientRequest_GPRS(u8* pSendBuff, u8* pDeviceID, u16 NodeAddress);          //传感器终端主动申请服务器下发配置
void Treaty_Data_Analysis(u8* pTreatyBuff, u16* pFlag, u8* pDeviceID, u16 NodeAddress);	


uint16_t char_to_int(char* pSetPara);               //处理短信下发的配置，主要将字符转化为整数
float    char_to_float(char* pSetPara);               //处理短信下发的配置，主要将字符转化为小数

void SensorSetMessage(void);                       //短信获取设备配置参数                

#endif

