#include "simulate_usart.h"
#include <stdio.h>
#include "timer.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif

__IO uint32_t TimingDelay;
uint8_t dataReceived[100];
uint8_t Logo[]={"Hello, C!\r\n"};
__IO uint8_t receivedFlag;
__IO uint8_t receivedNum, tmpreceivedNum;
uint8_t cnt = 0;

/*
*********************************************************************************************************
*	�� �� ��: fputc
*	����˵��: �ض���putc��������������ʹ��printf�����Ӵ���1��ӡ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{

	/* дһ���ֽڵ�USART1 */
	//USART_SendData(USART1, (uint8_t) ch);
	SendOneByte((uint8_t) ch);
	/* �ȴ����ͽ��� */
	//while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
}


void Simulate_Usart(void)
{
	GPIO_Configuration();

	/* �����ⲿ�ж� */
	EXTI_Configuration();

	/* TIM1��ʼ�������ڽ��� */
	TIM1_Configuration();

	/* TIM2��ʼ���������ж������Ƿ������� */
	TIM2_Configuration();

	/* TIM3��ʼ�������ڷ��� */
	TIM3_Configuration();
}


/*
*********************************************************************************************************
*	�� �� ��: SendOneByte
*	����˵��: ģ�⴮�ڷ���һ�ֽ�����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SendOneByte(uint8_t datatoSend)
{
	uint8_t i, tmp;

	/* Start bit */
	TXD_low();
	Delay_Ms(SendingDelay);	

	for(i = 0; i < 8; i++)
	{
		tmp	= (datatoSend >> i) & 0x01;

		if(tmp == 0)
		{
			TXD_low();
			Delay_Ms(SendingDelay);	//0		
		}
		else
		{
			TXD_high();
			Delay_Ms(SendingDelay);	//1		
		}	
	}
	
	/* Stop bit */
	TXD_high();
	Delay_Ms(SendingDelay);	
}

/*
*********************************************************************************************************
*	�� �� ��: GPIO_Configuration
*	����˵��: ����PA9ΪTXD��PA10��RXD
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

    /* ʹ�� GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	/* ��ֹ�ϵ������� */
	GPIO_SetBits(GPIOE, GPIO_Pin_2);

	/* ����PA9Ϊ������� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* ����PA10Ϊ�������� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

}

/*
*********************************************************************************************************
*	�� �� ��: EXTI_Configuration
*	����˵��: ����PA10�ϵ��½��ش����ⲿ�ж�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void EXTI_Configuration(void)
{
  	EXTI_InitTypeDef EXTI_InitStructure;
  	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the AFIO Clock */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);

    /* Configure Button EXTI line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  

    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set Button EXTI Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure); 
}

/*
*********************************************************************************************************
*	�� �� ��: EXTI15_10_IRQHandler
*	����˵��: �ⲿ�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/

void EXTI3_IRQHandler(void) 
{
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(EXTI_GetFlagStatus(EXTI_Line3) != RESET)
	{
		/* Disable the Selected IRQ Channels -------------------------------------*/
    	NVIC->ICER[EXTI3_IRQn >> 0x05] =
      		(uint32_t)0x01 << (EXTI3_IRQn  & (uint8_t)0x1F);		

		receivedFlag = 0;

		TIM_SetCounter(TIM1, 0);
		TIM_Cmd(TIM1,ENABLE);		//����TIM1

		TIM_Cmd(TIM2,DISABLE);		//�ر�TIM2
		TIM_SetCounter(TIM2, 0);
		TIM_Cmd(TIM2,ENABLE);		//����TIM2
		
		EXTI_ClearITPendingBit(EXTI_Line3);
	
		cnt++;						//������
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif	
}

/*
*********************************************************************************************************
*	�� �� ��: TIM1_UP_IRQHandler
*	����˵��: 
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/    
void TIM1_UP_IRQHandler(void)
{

	uint8_t tmp;
	static uint8_t i;	

#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif

	if(TIM_GetFlagStatus(TIM1, TIM_FLAG_Update) != RESET)
	{

		tmp = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3);
		
		if(tmp == 1)
			dataReceived[receivedNum] |= (1 << i); 
	
		i++;
				
		if(i >= 8)
		{
			i = 0;
			receivedNum++; 
	
	    	/* Enable the Selected IRQ Channels --------------------------------------*/
			EXTI_ClearITPendingBit(EXTI_Line3);
	    	NVIC->ISER[EXTI3_IRQn  >> 0x05] =
	    		(uint32_t)0x01 << (EXTI3_IRQn  & (uint8_t)0x1F);
	
			TIM_Cmd(TIM1,DISABLE);		//�ر�TIM1
		
			TIM_Cmd(TIM2,DISABLE);		//�ر�TIM2
			TIM_SetCounter(TIM2, 0);
			TIM_Cmd(TIM2,ENABLE);		//����TIM2
		}

		TIM_ClearITPendingBit(TIM1, TIM_FLAG_Update);
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif	
}


/*
*********************************************************************************************************
*	�� �� ��: TIM2_IRQHandler
*	����˵��: �ⲿ�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/    
void TIM2_IRQHandler(void)
{
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(TIM_GetFlagStatus(TIM2, TIM_FLAG_Update) != RESET)
	{
		if(receivedNum != 0)
		{
			tmpreceivedNum = receivedNum;
			receivedNum	= 0;
			receivedFlag = 1;
		}

		TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif	
}


void Clr_Usart_Data(void)
{
	memset(dataReceived, 0, tmpreceivedNum);
}

