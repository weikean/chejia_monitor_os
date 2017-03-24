#include "simulate_usart.h"
#include <stdio.h>
#include "timer.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif

__IO uint32_t TimingDelay;
uint8_t dataReceived[100];
uint8_t Logo[]={"Hello, C!\r\n"};
__IO uint8_t receivedFlag;
__IO uint8_t receivedNum, tmpreceivedNum;
uint8_t cnt = 0;

/*
*********************************************************************************************************
*	函 数 名: fputc
*	功能说明: 重定义putc函数，这样可以使用printf函数从串口1打印输出
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{

	/* 写一个字节到USART1 */
	//USART_SendData(USART1, (uint8_t) ch);
	SendOneByte((uint8_t) ch);
	/* 等待发送结束 */
	//while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
}


void Simulate_Usart(void)
{
	GPIO_Configuration();

	/* 配置外部中断 */
	EXTI_Configuration();

	/* TIM1初始化，用于接收 */
	TIM1_Configuration();

	/* TIM2初始化，用于判断数据是否接收完成 */
	TIM2_Configuration();

	/* TIM3初始化，用于发送 */
	TIM3_Configuration();
}


/*
*********************************************************************************************************
*	函 数 名: SendOneByte
*	功能说明: 模拟串口发送一字节数据
*	形    参：无
*	返 回 值: 无
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
*	函 数 名: GPIO_Configuration
*	功能说明: 配置PA9为TXD，PA10做RXD
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能 GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	/* 防止上电后的误判 */
	GPIO_SetBits(GPIOE, GPIO_Pin_2);

	/* 配置PA9为推挽输出 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* 配置PA10为浮空输入 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

}

/*
*********************************************************************************************************
*	函 数 名: EXTI_Configuration
*	功能说明: 配置PA10上的下降沿触发外部中断
*	形    参：无
*	返 回 值: 无
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
*	函 数 名: EXTI15_10_IRQHandler
*	功能说明: 外部中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

void EXTI3_IRQHandler(void) 
{
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(EXTI_GetFlagStatus(EXTI_Line3) != RESET)
	{
		/* Disable the Selected IRQ Channels -------------------------------------*/
    	NVIC->ICER[EXTI3_IRQn >> 0x05] =
      		(uint32_t)0x01 << (EXTI3_IRQn  & (uint8_t)0x1F);		

		receivedFlag = 0;

		TIM_SetCounter(TIM1, 0);
		TIM_Cmd(TIM1,ENABLE);		//开启TIM1

		TIM_Cmd(TIM2,DISABLE);		//关闭TIM2
		TIM_SetCounter(TIM2, 0);
		TIM_Cmd(TIM2,ENABLE);		//开启TIM2
		
		EXTI_ClearITPendingBit(EXTI_Line3);
	
		cnt++;						//调试用
	}
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif	
}

/*
*********************************************************************************************************
*	函 数 名: TIM1_UP_IRQHandler
*	功能说明: 
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/    
void TIM1_UP_IRQHandler(void)
{

	uint8_t tmp;
	static uint8_t i;	

#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
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
	
			TIM_Cmd(TIM1,DISABLE);		//关闭TIM1
		
			TIM_Cmd(TIM2,DISABLE);		//关闭TIM2
			TIM_SetCounter(TIM2, 0);
			TIM_Cmd(TIM2,ENABLE);		//开启TIM2
		}

		TIM_ClearITPendingBit(TIM1, TIM_FLAG_Update);
	}
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif	
}


/*
*********************************************************************************************************
*	函 数 名: TIM2_IRQHandler
*	功能说明: 外部中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/    
void TIM2_IRQHandler(void)
{
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
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
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif	
}


void Clr_Usart_Data(void)
{
	memset(dataReceived, 0, tmpreceivedNum);
}

