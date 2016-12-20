#ifndef _MODBUS_H_
#define _MODBUS_H_ 

///#include "stm32f10x_lib.h"

#include "stm32f10x.h"
#define  FILTER_ORDER    8        //���������˲���ȣ�����Һλ����ƽ������,��ӦС��4

extern vu8 Uart4_rev_comflag ;    //RS485�������־
extern u8 Senser_Address ;        //Һλ�Ƶ�ͨѶ��ַ
//extern u8 tmr0_uart35 ;         //����3.5���ֽ���ʱ
extern u8 Uart4_send_buff[100]; 
extern u8 Uart4_rev_buff[100];
extern vs8 Uart4_send_counter;
extern vu8 Uart4_rev_comflag;        //RS485�������״̬��־
extern vu8 Uart4_send_ready_flag;    //RS485���;���״̬��־
extern vu8 Crc_counter ;             //RS485У�������
extern vu8 *Uart4_send_pointer;      //RS485���ڷ���ָ��
extern u8 RS485_Receive_Complete_Flag;

//extern vu16 Mkgz_bz ;//ģ����ϱ�־ 1:�����쳣��2:��ѹ 
//extern vu16 Out_current ;//�������
//extern vu16 Out_voltage ;//�����ѹ
//extern vu16 Mkzt_bz ;//ģ��״̬��־ 0:0����,1���䣻2:0����,1�ػ�
//extern vu16 OutX_current ;//�������
//extern vu16 Jc_voltage ;//�����ѹ
//extern vu16 Fc_voltage ;//�����ѹ
//extern vu16 user_day ;//ʹ������

extern void RS485_Communication(void);
extern void mov_data(uint8_t *a, uint8_t *b, uint8_t c);
unsigned short getCRC16(unsigned char *ptr,unsigned char len); 
#endif
