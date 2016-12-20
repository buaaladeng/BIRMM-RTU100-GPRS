#ifndef _SENSORINIT_H_
#define _SENSORINIT_H_
#include "stm32f10x.h"
#include "stdlib.h"


#define DEVICE   BIRMM_RTU100                            //�豸����Ϊȼ�����ܼ���ն�
#define REPORTDATA  GAS                                  //�ϱ�����Ϊ����Ũ��
#define TYPESEND   TYPE_433                              //��������ѡ��433ͨ��
#define SENSORID {0x31,0x20,0x16,0x08,0x31,0x02}         //������ID��
#define SOFTVERSION 0x20                                 //����汾��
#define HARDVERSION 0x10                                 //Ӳ���汾��
#define SETTIME {16,12,12,19,11,00}

#define CONFIG 0                                      //�Ƿ���Ҫ�������ò�����־
#define DEBUG_TEST 0   																//��ӡ���ܣ�1Ӧ���ڵ��� 0Ӧ������������



/* Private function prototypes -----------------------------------------------*/
void SensorInit(void);                                        //�豸��ʼ������
#endif             
/* end of SensorInit.h */
