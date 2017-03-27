#ifndef __USART2_H
#define __USART2_H
#include "stdio.h"	
#include "sys.h" 
//如果想串口中断接收，请不要注释以下宏定义
#define USART2_MAX_RECV_LEN		800					
#define USART2_MAX_SEND_LEN		200					
#define USART2_RX_EN 			1					

extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		
extern u16 USART2_RX_STA;   
	  	
void uart2_init(u32 bound);
void USART2_IRQHandler(void);
void UART2_SendString(char *s);
#endif
