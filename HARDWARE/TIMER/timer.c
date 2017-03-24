#include "timer.h"
#include "led.h"
#include "usart.h"

//TIM5

vu8 Time_fac = 0;

/*
*********************************************************************************************************
*	�� �� ��: TIM1_Configuration
*	����˵��: ����TIM1
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TIM1_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);

  	/* Time base configuration */
  	TIM_TimeBaseStructure.TIM_Period = SendingDelay -1;
  	TIM_TimeBaseStructure.TIM_Prescaler = 72-1;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);


		TIM_ClearITPendingBit(TIM1, TIM_FLAG_Update);
	
	/* ����TIM1���ж� */
    TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);     
        
}

void TIM2_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

  	/* Time base configuration */
  	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
  	TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
 	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
	
	/* ����TIM1���ж� */
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);      
        
}

void TIM3_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

  	/* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535 - 1;
  TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
 	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM3,ENABLE);    

}


void TIM5_Configuration(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);		//ʹ��Timer5ʱ��
	TIM_TimeBaseStructure.TIM_Period = 9999;					//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ(������10000Ϊ1s)
	TIM_TimeBaseStructure.TIM_Prescaler = 7199;					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ(10KHz�ļ���Ƶ��)
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//����ʱ�ӷָ�:TDTS = TIM_CKD_DIV1
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);				//����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE ); 				//ʹ��TIM5ָ�����ж�
	
	TIM_Cmd(TIM5, ENABLE);

}

void TIM5_IRQHandler(void)
{
	static u8 flag =1;

	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)  //���TIM5�����жϷ������
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
//		if(Timer0_start)
//		Times++;
//		if(Times > shijian)
//		{
//			Timer0_start = 0;
//			Times = 0;
//		}
		if(Time_fac >= 1)
			Time_fac++;
	}
}




//������ʱ��6�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��6!
void TIM6_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE); //��ʱ��6ʱ��ʹ��
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //���÷�Ƶֵ��10khz�ļ���Ƶ��
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=arr;  //�Զ���װ��ֵ ������5000Ϊ500ms
	TIM_TimeBaseInitStructure.TIM_ClockDivision=0; //ʱ�ӷָ�:TDS=Tck_Tim
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM6,TIM_IT_Update|TIM_IT_Trigger,ENABLE); //ʹ��TIM6�ĸ����ж�

	
	TIM_Cmd(TIM6,ENABLE); //��ʱ��6ʹ��
}


void TIM6_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6,TIM_IT_Update)!=RESET)
	{
		//GUI_TOUCH_Exec();
	}
	TIM_ClearITPendingBit(TIM6,TIM_IT_Update); //����жϱ�־λ
}

/*
*********************************************************************************************************
*	�� �� ��: Delay_Ms
*	����˵��: ��ʱ
*	��    �Σ�nTime,��λΪuS
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Delay_Ms(__IO uint32_t nTime)
{ 

	uint16_t tmp;

	tmp = TIM_GetCounter(TIM3);
	
	if(tmp + nTime <= 65535)
		while( (TIM_GetCounter(TIM3) - tmp) < nTime );
	else
	{
		TIM_SetCounter(TIM3, 0);
		while( TIM_GetCounter(TIM3) < nTime );
	}
			
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;	
	
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
  NVIC_Init(&NVIC_InitStructure);
	//USART2 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
  NVIC_Init(&NVIC_InitStructure);
	
	/* Enable and set Button EXTI Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;     // �����ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // �����ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;			//SDIO�ж�����
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ�0 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;					//�����ȼ�0 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);
	
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;         
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	 
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;      
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
  NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;				//TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//��ռ���ȼ�1��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//�����ȼ�1��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ʹ��IRQͨ��
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn; //TIM6�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2; //��ռ���ȼ�1��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE; //ʹ��ͨ��
	NVIC_Init(&NVIC_InitStructure);
}
