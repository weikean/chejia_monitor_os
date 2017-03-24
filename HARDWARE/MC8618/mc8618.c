#include "mc8618.h"
#include "usart.h"
#include "timer.h"
#include "delay.h"
#include <string.h>
#include "user_config.h"

bool mc8618_signal_lost(void)
{
	//+CSQ: 99, 99无天线信号
	if (GSM_send_cmd((u8*)"AT+CSQ",(u8*)"+CSQ: 99, 99",3))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool mc8618_ppp_open(void)
{
	if (GSM_send_cmd((u8*)"at+zpppopen",(u8*)"+ZPPPOPEN:ESTABLISHED",5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool mc8618_ppp_close(void)
{
	if (GSM_send_cmd((u8*)"at+zpppclose",(u8*)"+ZPPPCLOSE:OK",5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool mc8618_ppp_connected(void)
{
	if (GSM_send_cmd((u8*)"at+zpppstatus",(u8*)"+ZPPPSTATUS:ESTABLISHED",5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool mc8618_ppp_unconnected(void)
{
	if (GSM_send_cmd((u8*)"at+zpppstatus",(u8*)"+ZPPPSTATUS:DISCONNECTED",5))
	{
		return true;
	}
	else
	{
		return false;
	}
}



bool mc8618_ip_setup(void)
{
	char setup_ip[30] = "at+zipsetup=0,";
	strcat(setup_ip,ip1);
	if (GSM_send_cmd((u8*)setup_ip,(u8*)"+ZIPSETUP:CONNECTED",5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool mc8618_ip_connected(void)
{
	if(GSM_send_cmd((u8*)"at+zipstatus=0",(u8*)"+ZIPSTATUS:ESTABLISHED",5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool mc8618_ip_unconnected(void)
{
	if (GSM_send_cmd((u8*)"at+zipstatus=0",(u8*)"+ZIPSTATUS:DISCONNECTED",5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool mc8618_ip_close(void)
{
	if (GSM_send_cmd((u8*)"at+zipclose=0",(u8*)"+ZIPCLOSE:OK",5))
	{
		return true;
	}
	else
	{
		return false;
	}
}


void mc8618_power_on(void)
{
	while (NULL == (GSM_send_cmd((u8*)"AT+ZIND=8",(u8*)"OK",3))) 
	{
		printf("mc8618 is trying...\r");
		delay_ms(3000);
	}// 检查串口和AT
	printf("mc8618 is READY now...\n");
	
	while (NULL == (GSM_send_cmd((u8*)"AT+ZIND=9",(u8*)"OK",3)))
	{
		printf("sim lost...\n");
		GPIO_SetBits(GPIOE,GPIO_Pin_4);//输出1，开启蜂鸣器输出
		delay_ms(1000);
		GPIO_ResetBits(GPIOE,GPIO_Pin_4);//输出0，关闭蜂鸣器输出
		delay_ms(1000);
	}// 检查串口和AT
	
	while (mc8618_signal_lost())
	{
		printf("antenna signal has lost\n");
	}
	
	GSM_send_cmd((u8*)"ATE0",(u8*)"OK",3); //取消回显
}

void mc8618_power_off(void)
{
	while (NULL == (GSM_send_cmd((u8*)"AT+ZPWROFF",(u8*)"OK",3)));
}

void mc8618_conn(void)
{
	do
	{
		if ((mc8618_ppp_unconnected()))
		{
			mc8618_ppp_open();
		}
		else if (mc8618_ppp_connected())
		{
			printf("ppp has open\n");
			mc8618_ip_setup();
	    if(mc8618_ip_connected())
			{
				printf("ip ok");
			}
		}
	}while (NULL == (mc8618_ip_connected()));
}

void mc8618_close_conn(void)
{
	do
	{
		mc8618_ip_close();
		delay_ms(1000); //关闭也许需要时间
	}while (mc8618_ip_connected());
}

void mc8618_send_data()
{
	//恢复物理链路
	if(mc8618_ip_connected())
	{
		while ((mc8618_signal_lost()))
		{
			delay_ms(3000);
		}
			if (GSM_send_cmd((u8*)"at+zipsend=0,470",(u8*)">",3))
			{
				//取数据
			}
				//进入休眠
	}
	else
	{
		mc8618_conn();
	}
}


/*******************************************************************************
* 函数名 : CLR_Buf2
* 描述   : 清除串口2缓存数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<USART_REC_LEN;k++)      //将缓存内容清零
	{
		USART2_RX_BUF[k] = 0x00;
	}
    First_Int = 0;              //接收字符串的起始存储位置
}

/*******************************************************************************
* 函数名 : Find
* 描述   : 判断缓存中是否含有指定的字符串
* 输入   : 
* 输出   : 
* 返回   : unsigned char:1 找到指定字符，0 未找到指定字符 
* 注意   : 
*******************************************************************************/

u8 Find(char *a)
{ 
  if(strstr((const char*)USART2_RX_BUF,a)==NULL)
	    return 1;
	else
			return 0;
}

/*******************************************************************************
* 函数名 : Second_AT_Command
* 描述   : 发送AT指令函数
* 输入   : 发送数据的指针、发送等待时间(单位：S)
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/

u8 GSM_send_cmd(u8 *cmd,u8 *ack,u8 wait_time)         
{
	u8 res=0;
	//u8 *c;
	//c = cmd;										//保存字符串地址到c
	CLR_Buf2(); 
	for (; *cmd!='\0'; cmd++)
	{
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
	  USART_SendData(USART2,*cmd);
	}
	UART2_SendString("\r\n");	
	if(wait_time==0)return res;
	Times = 0;
	shijian = wait_time;
	Timer0_start = 1;
	while (Timer0_start&res)                    
	{
		if (strstr((const char*)USART2_RX_BUF,(char*)ack)==NULL)
			 res=0;
		else
		{
			 res=1;
		}
	}
	return res;
}
