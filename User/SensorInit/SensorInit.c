
#include "SensorInit.h"
#include "test.h"
#include "SPI_Flash.h"

/**********************************��������Ϣ�˹�д��FLASH��*******************************/

void SensorInit(void)
{
	
     write_collect_parameter_to_flash();         //���Ժ���   �����豸�ɼ������ĳ�ʼ������
     //Set_Time();																 //���Ժ���   �����豸ʱ�������
     //Set_DS2780();
}




