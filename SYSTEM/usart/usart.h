#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART1_MAX_RECV_LEN		800					
#define USART1_MAX_SEND_LEN		200					
#define USART1_RX_EN 			1					

extern u8  USART1_RX_BUF[USART1_MAX_RECV_LEN]; 		
extern u8  USART1_TX_BUF[USART1_MAX_SEND_LEN]; 		
extern u16 USART1_RX_STA;   
	  	

//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
void TIM4_Set(u8 sta);
void TIM4_Init(u16 arr,u16 psc);
void UART_DMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar);
void UART_DMA_Enable(DMA_Channel_TypeDef*DMA_CHx,u8 len);
void u1_printf(char* fmt, ...);
#endif


