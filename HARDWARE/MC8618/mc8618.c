#include "mc8618.h"
#include "usart2.h"
#include "timer.h"
#include "delay.h"
#include <string.h>
#include "user_config.h"
#include <stdlib.h>

//GSM_send_cmd返回0代表成功

static bool mc8618_signal_lost(void)
{	//+CSQ: 99, 99无天线信号
	return (GSM_send_cmd((u8*)"AT+CSQ",(u8*)"+CSQ: 99, 99",5))? false : true;
}

static bool mc8618_ppp_open(void)
{
	return (GSM_send_cmd((u8*)"at+zpppopen",(u8*)"+ZPPPSTATUS: OPENED",20))? false : true;
}

static bool mc8618_ppp_close(void)
{
	return (GSM_send_cmd((u8*)"at+zpppclose",(u8*)"+ZPPPSTATUS: CLOSED",5))? false : true;
}

static bool mc8618_ppp_connected(void)
{
	return (GSM_send_cmd((u8*)"at+zpppstatus",(u8*)"+ZPPPSTATUS: OPENED",5))? false : true;
}

static bool mc8618_ppp_unconnected(void)
{
	return (GSM_send_cmd((u8*)"at+zpppstatus",(u8*)"+ZPPPSTATUS: CLOSED",5))? false : true;
}

static bool mc8618_ip_setup(void)
{
	return (GSM_send_cmd((u8*)"at+zipsetup=0,120.25.56.199,10003",(u8*)"+ZTCPESTABLISHED:0",10))? false : true;
}

static bool mc8618_ip_connected(void)
{
	return (GSM_send_cmd((u8*)"at+zipstatus=0",(u8*)"+ZIPSTATUS: ESTABLISHED",5))? false : true;
}

static bool mc8618_ip_unconnected(void)
{
	return (GSM_send_cmd((u8*)"at+zipstatus=0",(u8*)"+ZIPSTATUS: CLOSED",5))? false : true;
}
static bool mc8618_ip_close(void)
{
	return (GSM_send_cmd((u8*)"at+zipclose=0",(u8*)"+ZTCPCLOSED:0",5))? false : true;
}

static bool mc8618_sleep(void)
{
	return (GSM_send_cmd((u8*)"AT+ZDORMANT",(u8*)"OK",5))? false : true;
}

static bool mc8618_active(void)
{
	return (GSM_send_cmd((u8*)"AT+ZGOACTIVE",(u8*)"OK",5))? false : true;
}

static bool mc8618_send_cmd(const char *data)
{
	return (GSM_send_cmd((u8*)data,(u8*)"+ZIPSEND:",20))? false : true;
}


void mc8618_power_on(void)
{
	while ((GSM_send_cmd((u8*)"AT",(u8*)"OK",3))) 
	{
		printf("mc8618 is trying OPEN...\n");
		//LCD warning
		delay_ms(2000);
	}// 检查串口和AT
	
	printf("mc8618 is READY now...\n");
	
	while ((GSM_send_cmd((u8*)"AT+ZIND=1",(u8*)"OK",3)))
	{
		printf("sim lost...\n");
		//LCD warning
		delay_ms(2000);
	}// 检查串口和AT
	while (mc8618_signal_lost())
	{
		printf("antenna signal has lost\n");
		delay_ms(2000);
	}	
	GSM_send_cmd((u8*)"ATE0",(u8*)"OK",3); //取消回显
}

void mc8618_power_off(void)
{
	while ((GSM_send_cmd((u8*)"AT+ZPWROFF",(u8*)"OK",3)));
}

bool mc8618_conn(void)
{
while(1)
{
	if (!(mc8618_signal_lost()))
	{
		if ((mc8618_ppp_connected()))
		{
			printf("ppp has open\n");
			mc8618_ip_setup();
			if (mc8618_ip_connected())
			{
				printf("ip ok\n");
				return true;
			}
		}
		else
			mc8618_ppp_open();
	}
	else
	{
		//signal lost
		delay_ms(1000);
	}
}
}

void mc8618_close_conn(void)
{
	do
	{
		mc8618_ip_close();
		delay_ms(1000); //关闭也许需要时间
	}while (mc8618_ip_connected());
}

void mc8618_send_data(const char *data, const int len)
{
	char pData[128];
	char ack[] = "at+zipsend=0,";
	char len_s[5];
	bool ret;
	
	strcat(pData,ack);
	sprintf(len_s,"%d",len);
	strcat(pData,len_s);
	strcat(pData,"\r");
	strcat(pData,data);
	
	if (!(mc8618_signal_lost()))
	{
	if(mc8618_ip_connected())
	{
		if(mc8618_active()) //恢复物理链路
		{
				ret =  mc8618_send_cmd(pData);
			  if(ret == true)
				{
					printf("data_send_ok\n");
				}
			
			mc8618_sleep();//进入休眠
			printf("sleep ok\n");
		}
		else
			printf("active failed\n");
	}
	else
	{
		mc8618_conn();
	}
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
	for(k=0;k<USART2_MAX_RECV_LEN	;k++)      //将缓存内容清零
	{
		USART2_RX_BUF[k] = 0x00;
	}
    //First_Int = 0;              //接收字符串的起始存储位置
}

/*******************************************************************************
* 函数名 : Find
* 描述   : 判断缓存中是否含有指定的字符串
* 输入   : 
* 输出   : 
* 返回   : unsigned char:1 找到指定字符，0 未找到指定字符 
* 注意   : 
*******************************************************************************/

u8 Find(const char *a)
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
			 res=1;
		else
		{
			 res=0;
		}
	}
	return res;
}

