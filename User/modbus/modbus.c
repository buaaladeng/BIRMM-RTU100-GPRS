/*****************************************************
          modbus-rtu ͨѶ��Լ

ͨѶ��ʽ��rs-232 ��˫��
У�鷽ʽ��crc16
ֹͣλ��2λ
��д��������
��д���ڣ�2011��3��26��
�汾��v1.0(���ŵ�ͨѶ)
���˼·:
    1.�����ж������Զ����������ϵ���Ϣ,�����յ�
    �ֽں󳬹�3.5���ֽ�ʱ��û���µ��ֽ���Ϊ����
    �������,������ɱ�־��1;���������ɱ�־��
    ����1�������ݽ����������������ݡ�
    2.���ڽ������ݵĴ���, ��������ɱ�־��1����
    �������ݴ���, (1)�����жϽ��յĵ�һλ������
    ������ַ�Ƿ���ͬ,�������ͬ��ս��ջ��治��
    ���κ���Ϣ; (2)���յĵ�һλ�����뱾����ַ��
    ͬ,��Խ��ջ����е����ݽ���crc16У��,�����
    �յ�У��λ�뱾У��������ͬ��ս��ջ��治��
    ���κ���Ϣ��
    (3)���crc16У����ȷ��������ݴ��е��������
    ����Ӧ�Ĵ���
******************************************************/
#include "modbus.h"
#include "bsp_usart.h"
#include "math.h"
#include "bsp_SysTick.h"
#include "string.h"

unsigned char Senser_Address = 0x01;             //����Һλ���豸ͨ�ŵ�ַ     
unsigned char Uart4_rev_buff[100]={'\0'};        //RS485���ڽ��ջ���
unsigned char Uart4_send_buff[100]={'\0'};       //RS485���ڷ��ͻ���
volatile unsigned char Uart4_rev_count=0;        //RS485���ڽ��ռ�����
vs8 Uart4_send_counter = 0;                      //RS485���ڷ��ͼ�����
volatile unsigned char Uart4_rev_comflag=0;      //�������״̬��־
volatile unsigned char Uart4_send_ready_flag=0;  //���;���״̬��־
volatile unsigned char Crc_counter = 0;          //RS485У�������
volatile unsigned char *Uart4_send_pointer = Uart4_send_buff;//RS485���ڷ���ָ��

extern  char  Usart1_recev_buff[300];
//extern  char  Usart1_send_buff[];
extern  uint16_t  Usart1_recev_count;
//extern  uint8_t  Usart1_send_count;
extern u8  DataCollectCache[13][4];   //Һλ���ݲɼ����棬���13�����ݣ�������HEX��ʽ�洢�����ֽ���ǰ�����ֽ��ں�
extern u8  DataCollectCount;          //���ݲɼ�������
extern  float LevelData_Float[FILTER_ORDER];       //�ɼ�������ʱҺλ���ݣ�������
uint8_t  LevelDataCount = FILTER_ORDER;       //Һλ���ݼ���������ʾ��ǰ�ɼ�����������
//float  Data_Liquid_Level=0.0;

extern void Delay(vu32 nCount);
//unsigned short getCRC16(volatile unsigned char *ptr,unsigned char len) ; 
void mov_data(uint8_t *a, uint8_t *b, uint8_t c);
float Bin754_to_float(uint32_t   data_bin754);
void Modbus_Function_3(void); 
//void Modbus_Function_10(void);       
/***************************************
�������ƣ�crc16У��
�������ܣ�crc16У��
�������룺�ֽ�ָ��*ptr�����ݳ���len
�������أ�˫�ֽ�crc
������д�����
��д���ڣ�2008��6��9��
�����汾��v0.2
****************************************/
unsigned short getCRC16(unsigned char *ptr,unsigned char len) 
{ 
    unsigned char i; 
    unsigned short crc = 0xFFFF; 
	
    if(len==0)          //Ϊʲô��ô����
    {
        len = 1;
    } 
    while(len--)  
    {   
        crc ^= *ptr; 
				for(i=0; i<8; i++)  
				{ 
							if(crc&1) 
							{ 
										crc >>= 1;  
										crc ^= 0xA001; 
							}  
							else 
							{
										crc >>= 1;
							} 
				}         
				ptr++; 
    } 
    return(crc); 
} 

/*******************************************************************************
    �����ݸ������ݺ���
���ܣ�������a��c�����ݸ��Ƶ�����b��
���룺ָ��a,ָ��b,���ݸ���c
���أ���
��д�����
��д���ڣ�2008��3��28��
�汾��v0.1
********************************************************************************/
void mov_data(uint8_t *a,uint8_t *b,uint8_t c)
{
    uint8_t i;
	  if((c>0)&&(c<=100))
		{	
			for(i=0; i<c; i++)
			{
					a[i] = b[i];
			}
		}
}
/*******************************************************************************
    32λieee754��ʽ��������ת��Ϊʮ���Ƹ�����
���ܣ�32λieee754��ʽ��������ת��Ϊʮ���Ƹ�����
���룺32λieee754��ʽ��������
���أ�ת����ĸ�����
��д��
��д���ڣ�XX��XX��XX��
�汾��v0.1
********************************************************************************/
float Bin754_to_float(uint32_t   data_bin754)
{
		int   i;
		uint8_t   stand[32];
		uint32_t  temp;
	  float     temp_f=0;
		int       jiema=0;       //��������������ʼ����ֹ���
	  float     jishu=0;       //���������������ʼ����ֹ���
	   
	  temp=data_bin754;
		for(i=31;i>=0;i--)
		{
			stand[i]=temp & 0x01;
			temp=temp>>1;
		}
		for(i=1;i<=8;i++)
		{
       jiema += stand[i]*pow(2,8-i);
    }
		if(jiema>=127)
		{  jiema=jiema-127;        //�������
		}		
    else	                     //��ֹ���
		{
			 jiema=-1*(127-jiema);
    }			               
		for(i=9;i<=31;i++)
		{
       jishu += stand[i]*pow(2,8-i);
    }
		jishu=jishu+1;                        //�������
		
//		printf("%d\r\n",jiema);        //����ʹ��
//		printf("%f\r\n",jishu);        //����ʹ��
		temp_f=jishu*pow(2,jiema);            //����������
		if(stand[0]==1)   temp_f = -temp_f;   //���ӷ���
		
		return  temp_f;
}
/*******************************************************************************
���ܣ����հ��¶�Э������ݸ�ʽ����������ת��Ϊ16���Ʊ�ʾ
���룺�����ʽ����
���أ���
��д��
��д���ڣ�XX��XX��XX��
�汾��v0.1
********************************************************************************/
void  Float2Hex_Aider( float DataSmooth )
{
   uint8_t   i=0;
	 uint8_t   DataArry[4]={0x00};
	 float*  pData = &DataSmooth;
	 uint8_t* pData_Hex = NULL;
	
	 pData_Hex = (uint8_t*)pData;
   for(i=4;i>0;i--)
   {
     DataArry[i-1] = *pData_Hex;
		 pData_Hex++;
   }
	
   for(i=0;i<4;i++)
	 {
			if((i%2)==0)
			{
//        DataCollectCache[DataCollectCount-1][i+1] = DataArry[i];
      }
			else
			{
			//	DataCollectCache[DataCollectCount-1][i-1] = DataArry[i]; //��Һλ�Ʒ��ص�����hex��ʽҺλ���ݽ��С������򡱱任�����洢�����ݻ�����
			}
   }	
 	 DataCollectCount--;
}
/**************************************************������������**************************************************/
/****************************************************************************************************************/
/*-----��������--------�Ĵ�����ַ------��������--------ռ�ÿռ�-------��дȨ��-------------��ע-----------------*/                                                
/*-----LED��ʾֵ---------0x0000----------������------16λ�Ĵ���*2------ֻ��-------Һλ��̽ͷǰ����Һ�洹ֱ����--*/
/*-----�������¶�--------0x0002----------������-------4Byte(ͬ��)------ֻ��------------��һ��ʽҺλ����Ч-------*/
/*-----�������ֵ--------0x0004----------������-------4Byte(ͬ��)------ֻ��---------------�Ա��ͺ���Ч----------*/
/*-----����ֵ------------0x0006----------������-------4Byte(ͬ��)------ֻ��-----------------���޽���------------*/
/*-----ԭʼֵ------------0x0008----------������-------4Byte(ͬ��)------ֻ��-----------------���޽���------------*/
/*-----�ٷֱ�------------0x00010---------������-------4Byte(ͬ��)------ֻ��------��ǰ����ֵռ����(10m)�İٷֱ�--*/
/****************************************************************************************************************/ 
// void Modbus_Function_3(void) 
//{ 

//	  uint32_t  data_temp=0;
//	  uint32_t  data32_array[4]={0};
//	  float     data_f=0;
//	  uint8_t   i=0;
////    uint8_t*  pData =NULL;     //����ʹ��
////    float* pFloat = &data_f;     //����ʹ��

//	  for(i=0;i<4;i++)
//		{
//			data32_array[i] = Uart4_rev_buff[3+i];
////			printf("\r\ndata[%d] = %x\r\n",i,data32_array[i]);  //����ʹ��
//		} 
//	  data_temp = (data32_array[0] <<24)+(data32_array[1] << 16)+(data32_array[2] << 8)+data32_array[3];
//	  data_f = Bin754_to_float(data_temp);
//	
//		if(data_f > 0.01) 
//		{
//			LevelData_Float[LevelDataCount-1] = data_f;
//			LevelDataCount--;        //��������Һλ���ݲ�Ϊ��0��ʱ�����ݼ�������1��������Ч����
//		}

//		
////		#if DEBUG_TEST	 
//	  printf("��ǰҺλ��: %0.3f ��\r\n",data_f);  //����ʹ��
////		#endif
////		pData = (uint8_t*)pFloat;    //����ʹ��
////		for(i=0;i<4;i++)
////		{
////			printf("\r\nFdata[%d] = %x\r\n",i,*pData);  //����ʹ��
////			pData++;          //����ʹ��
////		} 
//		
//} 
/****************************����д�뺯��*************************************************/
void Modbus_Function_10(void) 
{ 
//������Ҫ������
} 
/******************************************************************************/ 
//void RS485_Communication(void) 
//{ 
//     unsigned short crcresult; 
//     unsigned char temp[2]; 
//	   int  i=0;  
//	   
//     if(Uart4_rev_comflag == 1)//������ɱ�־=1���������˳�
//		 {  

//			 if(Uart4_rev_buff[0] == Senser_Address)//��ַ��ȷ
//    	  {  		

//            crcresult = getCRC16(Uart4_rev_buff,Crc_counter-2); 
//            temp[1] = crcresult & 0xff; //��8λ
//            temp[0] = (crcresult >> 8) & 0xff; //��8λ
//            if((Uart4_rev_buff[Crc_counter-1] == temp[0])&&(Uart4_rev_buff[Crc_counter-2] == temp[1]))  //crcУ����ȷ���������� 		
//        	  { 											
//                					
//							  switch(Uart4_rev_buff[1])            //������
//                { 
//                    case 0x03:                       // ���������ݺͲ˵�����
//                    {
//                        Modbus_Function_3();         //�������Ĵ��������ݽ�����Ӧ�ı任��Ȼ��ͨ��USART1������λ����
//                        
//        			      }break;  
//										case 0x04:                       //����������
//                    {	
//                        ;                            //������
//        			      } break;
//                    case 0x10:                       //д�˵�����
//                    {	
//                        Modbus_Function_10();        //���ò���д�뺯��
//        			      } break; 
//										case 0x12:                       //�������浽EEPROM
//                    {	
//                        ;                            //������
//        			      } break;
//					          default:                         //��������Ч��Ӧ��
//        			      {
//                        ;
//					          }break; 
//                } 				 
//    		    }
//						else                                     //CRCУ���벻��ȷ�������κδ���
//						{
//						   ;
//						}
//    	  }
//        Uart4_rev_comflag = 0;	                     //������ɺ󣬽��г�ʼ��
//				memset(Uart4_rev_buff,0x00,sizeof(Uart4_rev_buff));       //���UART4����BUFF
//				Uart4_rev_count =0;                                       //��λUART4���ռ�����
////        for(i=99;i>=0;i--) 
////    	  { 
////            Uart4_rev_buff[i] = 0;
////    	  } 
//		}
//		
//		if(Uart4_send_ready_flag==1)                     //��Һλ����׼������
//    {  
//        if(Uart4_send_buff[0] == Senser_Address)     //��ַ��ȷ
//    	  {  
// 
//					  DIR485_Send();                           //485����ʹ��
//						Delay_ms(50);                            //ԭʼ3ms

//            crcresult = getCRC16(Uart4_send_buff,Crc_counter); 
//            temp[1] = crcresult & 0xff;              //��8λ
//            temp[0] = (crcresult >> 8) & 0xff;       //��8λ
//					  Uart4_send_buff[Crc_counter] =temp[1];   
//					  Uart4_send_buff[Crc_counter+1] =temp[0]; //�������ֽ�CRCУ����
//					  Uart4_send_counter=Crc_counter+2;
//					  Crc_counter=0;
//						 
//				  	Uart4_send_pointer=Uart4_send_buff;
//						for(i=0;i<Uart4_send_counter;i++)
//						{  
//							 USART_SendData(UART4, (uint8_t)*Uart4_send_pointer++);
//				       while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET){;}   //�ȴ�������� 
//						}
//								
//						Uart4_send_counter=0;          //���ͼ�������λ 
//	          memset(Uart4_send_buff,0x00,sizeof(Uart4_send_buff));       //���UART4����BUFF
//			                                     //��λUART4���ռ�����						
////						for(i=0;i<100;i++)
////						{
////							 Uart4_send_buff[i]='\0';    //��շ��ͻ���    
////            }
//				 }
//				 Uart4_send_ready_flag=0;     
//				 Delay_us(200);                  //�����ݽ�����ɺ��ӳ�һ��ʱ�����л��շ����ƿ��أ�Ĭ����ʱ200us
//				 DIR485_Receive();
//				 Delay_ms(100);                    //ԭʼ3ms
//		}
//					 
//} 






