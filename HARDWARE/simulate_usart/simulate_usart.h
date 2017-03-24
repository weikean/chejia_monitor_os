#ifndef __SIMULATE_USART_H
#define __SIMULATE_USART_H

#include <stm32f10x.h>

#define TXD_high()		GPIO_SetBits(GPIOE, GPIO_Pin_2)
#define TXD_low()		GPIO_ResetBits(GPIOE, GPIO_Pin_2)

#define BaudRateUsed	9600

#define SendingDelay	104

extern uint8_t dataReceived[100];

void Simulate_Usart(void);
void SendOneByte(uint8_t datatoSend);
void GPIO_Configuration(void);
void PrintfLogo(void);
void GPIO_Configuration(void);
void EXTI_Configuration(void);
void TIM1_Configuration(void);
void TIM2_Configuration(void);
void TIM3_Configuration(void);
void PrintfAnswer(void);
void Clr_Usart_Data(void);

#endif
