#include "beep.h"
#include "delay.h"
#include "user_config.h"
  

//��ʼ��PE4Ϊ�����.��ʹ������ڵ�ʱ��		    
//��������ʼ��
void BEEP_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 //ʹ��GPIOE�˿�ʱ��
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //BEEP-->PB.4 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);	 //���ݲ�����ʼ��GPIOE.4
 
 GPIO_ResetBits(GPIOE,GPIO_Pin_4);//���0���رշ��������

}

void BEEP_test(void)
{


 GPIO_SetBits(GPIOE,GPIO_Pin_4);//���1���������������
 delay_ms(1000);
 GPIO_ResetBits(GPIOE,GPIO_Pin_4);//���0���رշ��������
 delay_ms(1000);

 
}
