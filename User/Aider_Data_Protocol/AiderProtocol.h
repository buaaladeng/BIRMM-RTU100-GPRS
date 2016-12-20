#ifndef _AIDERPROTOCOL_H_
#define _AIDERPROTOCOL_H_

#define MAX 20                         //   最大OID传输个数
#define MAX_COLLECTNUM  15             //最大采集数据的数量


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
 };                                          //字符数据与浮点数相互转换
 
 
 
struct liquid_set 
{
	uint16_t  CollectPeriod;         //采集间隔            
	uint16_t  SendCount;            //每次上报的上报周期    
	uint16_t  CollectStartTime;     //开始采集时间          
	u8        CollectNum;           //已完成的采集数量              上报数据的个数 = 上报周期/采集间隔
	u8        RetryNum;             //重传次数
  u8        BatteryCapacity;        //电池容量检测，例如xx%，仅为百分号前面的部分。
	uint16_t  WorkNum;                //系统已工作次数
	u8        Time_Sec;
	u8   			Time_Min;
	u8   			Time_Hour;
	u8   			Time_Mday;
	u8   			Time_Mon;
	u8   			Time_Year;

	u8        MessageSetFlag;         //用于指示当前是否通过短信修改参数配置
	
	char      AlarmPhoneNum[16];      //报警电话号码
	char      ServerIP[16];           //服务器IP
	char      ServerPort[6];          //服务器端口
	
  union     Hex_Float LowAlarmLevel;            //低报警浓度阈值
	union     Hex_Float HighAlarmLevel;           //高报警浓度阈值

};

struct SenserData 
{

	u16               CollectTime;           //每组数据对应的采集时间，以每天零点为参考，单位分钟
	union Hex_Float   Ch4Data;               //采集到的气体浓度数据     不超过15个数据
	u8                DataCount;             //采集到的数据总数

};

struct ReadDataFromFlash 
{

	u16               CollectTime[MAX_COLLECTNUM];           //每组数据对应的采集时间，以每天零点为参考，单位分钟
	union Hex_Float   Ch4Data[MAX_COLLECTNUM];               //采集到的气体浓度数据     不超过15个数据
	u8                DataCount;                            //采集到的数据总数

};


/***********************操作类型定义**************************/
typedef enum 
{
	GETREQUEST      	= 1,    //服务器请求
	GETRESPONSE 	    = 2,    //设备响应SET/GET请求
	SETREQUEST		    = 3,   //设置设备参数请求
	TRAPREQUEST 	    = 4,   //设备主动上报
	TRAPRESPONSE		  = 5,   //设备主动上报响应
	ONLINEREQUEST 	  = 6,   //设备检查链接
	ONLINERESPONSE		= 7,   //设备检查链接响应
	STARTUPREQUEST 	  = 8,   //设备开机上报信息
	STARTUPRESPONSE		= 9,   //设备开机上报信息响应
	WAKEUPREQUEST 	  = 10,   //服务器唤醒设备
	WAKEUPRESPONSE		= 11,   //设备响应服务器唤醒

} CommunicateType;                  //支持本协议的设备与服务器之间的操作类型

/***********************通信方式定义**************************/
typedef enum
{
	TYPE_433     = 1,     //表示传感器设备与服务器通过数据集中器进行通信
	TYPE_SMS     = 2,     //表示传感器设备与手机通过短信进行通信
	TYPE_GPRS    = 3,     //表示传感器设备与服务器直接通信
}SendType;                 //通信方式

/***********************OID定义**************************/

typedef enum 
{
	DEF_NR       = 0x1000000A,    //重传次数
	SYSTERM_DATA = 0x10000050,    //系统日期
	SYSTERM_TIME = 0x10000051,    //系统时间
	CLT1_ITRL1   = 0x10000105,    //一时区采集间隔

	UPLOAD_CYCLE = 0x10000062,    //数据上报周期
	DEVICE_STATE = 0x60000100,    //设备状态指示（上电）
	DEVICE_QTY   = 0x60000020,    //电池电量
  DEVICE_WAKEUP= 0x60000200,    //设备状态指示（唤醒）
	FRAM_STATE   = 0x60000300,     //数据接收状态指示
	
	BSS_IP			 = 0x10000022,     //网络服务IP号
	BSS_PORT	   = 0x10000023,     //网络服务端口号
	SMS_PHONE    = 0x10000041,     //短信告警电话号码

 	RESET_PROBER = 0x10000061,       //设备复位命令
  DATA_REQUEST = 0x20000001,       //服务器查询设备数据命令


} CommandType;                  //设备通信过程中使用到的OID集合（除了业务上报OID）

//结构体声明
// -----------------------------------------------------------------------------
// DESCRIPTION: 设备类型
// -----------------------------------------------------------------------------
typedef enum 
{
	ADL_FPL       	= 1,    //多功能渗漏
	BIRMM_NOISE1000 = 2,    //噪声记录仪
	BIRMM_RTU100		= 4,   //燃气智能监测终端
	BIRMM_LEVEL1000 = 7,   //液位监测仪
	BIRMM_RL2000		= 8,   //温度压力传感器
	BIRMM_WM1000		= 10,  //远程水表
	BIRMM_CorrR1000 = 11,  //保温层下腐蚀速率监测仪
	BIRMM_CorrE1000 = 12,  //保温层下腐蚀环境参数监测仪
	
} DeviceType;                  //支持本协议的所有设备的类型定义

//结构体声明
// -----------------------------------------------------------------------------
// DESCRIPTION: 上报数据类型
// -----------------------------------------------------------------------------
typedef enum
{
	FLOW       =1,        //流量为1
	PRESSURE   =2,        //压力为2
	LEVEL 		 =3,        //液位为3
	NOISE      =4,				//噪声为4
	GAS				 =5,        //气体为5
	H_TEMPERATURE =8,       //热力温度为8
	H_PRESSURE    =9,       //热力压力为9
	WATER					=10,      //水表为10
	CORR_RATE     =11,      //腐蚀速率
	CORR_ENV			=12,			//腐蚀环境

} ReportDataType;            //数据上传类型
//结构体声明
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
struct TagStruct                         //用于存储参数命令数据
{
   CommandType   OID_Command;            //命令OID编码
   u16           Width;                  //Value域的长度
   u8            Value[32];              //参数值，当Width为0时，Value域可以不存在，当前应用需求Value域最多占用32字节
};

//结构体声明
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
struct SpecialTagStruct                  //用于存储业务上报数据
{
	 u32           OID_Data;               //数据OID编码
   u16           Width;                  //Value域的长度
	 union                                 //声明共用体，便于后续对特定字节操作
	 {
      u8         Value[4];               //ieee754格式表示的数据
		  float      Data_F;                 //浮点数格式表示的数据
   }PerceptionData;                      //采集到的传感器数据   
	 u16           CollectTime;            //数据采集时间  单位 为 分钟
	
};

//结构体声明
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
struct CommandFrame                      //查询命令数据帧格式
{
	u8           Preamble;                 //帧前导码
	u8           Version;                  //协议版本号
	u16          Length;                   //有效数据长度，即从Length字段到CRC字段包含的实际数据长度（单位为Byte，不包含Length字段和CRC字段，OID_List长度以实际使用的数据空间为准）
	u8           DeviceID[6];              //设备ID号
	u8           RouteFlag;                //路由标志
	u16          NodeAddr;                 //路由节点地址
	u16          PDU_Type;                 //操作指示
	u8           Seq;                      //报文序号（1~255）
	CommandType  OID_List[MAX];            //OID序列，一帧数据可以包含多个OID，最多不超过20个
	u16          CrcCode;                  //16位CRC校验码，计算范围为从Preamble到CrcCode的全部数据（包括Preamble和CrcCode，CrcCode初值为0xFFFF）
	u8           OID_Count;                //OID序列计数器
};

//结构体声明
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
struct DataFrame                             //下发配置参数等帧格式
{
	u8               Preamble;                 //帧前导码
	u8               Version;                  //协议版本号
	u16              Length;                   //有效数据长度，即从Length字段到CRC字段包含的实际数据长度（单位为Byte，不包含Length字段和CRC字段，OID_List长度以实际使用的数据空间为准）
	u8               DeviceID[6];              //设备ID号
	u8               RouteFlag;                //路由标志
	u16              NodeAddr;                 //路由节点地址
	u16              PDU_Type;                 //操作指示
	u8               Seq;                      //报文序号（1~255）
	struct TagStruct TagList[MAX];             //Tag序列，一帧数据可以包含多个Tag，最多不超过20个,而且一帧数据总长度（包括9字节帧头）不能超过256
	u16              CrcCode;                  //16位CRC校验码，计算范围为从Preamble到CrcCode的全部数据（包括Preamble和CrcCode，CrcCode初值为0xFFFF）
	u8               Tag_Count;                //Tag序列计数器
	
};

//结构体声明
// -----------------------------------------------------------------------------
// DESCRIPTION:
// -----------------------------------------------------------------------------
struct  SpecialDataFrame                     //业务数据上传帧格式
{
	u8                      Preamble;                 //帧前导码
	u8                      Version;                  //协议版本号
	u16                     Length;                   //有效数据长度，即从Length字段到CRC字段包含的实际数据长度（单位为Byte，不包含Length字段和CRC字段，OID_List长度以实际使用的数据空间为准）
	u8                      DeviceID[6];              //设备ID号
	u8                      RouteFlag;                //路由标志
	u16                     NodeAddr;                 //路由节点地址
	u16                     PDU_Type;                 //操作指示
	u8                      Seq;                      //报文序号（1~255）
	struct TagStruct        SysTime;                  //系统时间，表示第一个数据的采集时间
	struct SpecialTagStruct TagList[MAX];             //Tag序列，一帧数据可以包含多个Tag，最多不超过20个,而且一帧数据总长度（包括9字节帧头）不能超过256
	struct TagStruct        BattEnergy;               //电池剩余电量，例如xx%，仅为百分号前面的部分。
	u16                     CrcCode;                  //16位CRC校验码，计算范围为从Preamble到CrcCode的全部数据（包括Preamble和CrcCode，CrcCode初值为0xFFFF）
	u8                      Tag_Count;                //Tag序列计数器
	
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
unsigned int Get_Rand_Num(void);                                 //获取随机数据
#endif

