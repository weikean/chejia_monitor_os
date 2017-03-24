#include "mc8618.h"
#include "usart.h"
#include "timer.h"
#include "delay.h"
#include <string.h>
#include "user_config.h"

bool mc8618_signal_lost(void)
{
	//+CSQ: 99, 99�������ź�
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
	}// ��鴮�ں�AT
	printf("mc8618 is READY now...\n");
	
	while (NULL == (GSM_send_cmd((u8*)"AT+ZIND=9",(u8*)"OK",3)))
	{
		printf("sim lost...\n");
		GPIO_SetBits(GPIOE,GPIO_Pin_4);//���1���������������
		delay_ms(1000);
		GPIO_ResetBits(GPIOE,GPIO_Pin_4);//���0���رշ��������
		delay_ms(1000);
	}// ��鴮�ں�AT
	
	while (mc8618_signal_lost())
	{
		printf("antenna signal has lost\n");
	}
	
	GSM_send_cmd((u8*)"ATE0",(u8*)"OK",3); //ȡ������
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
		delay_ms(1000); //�ر�Ҳ����Ҫʱ��
	}while (mc8618_ip_connected());
}

void mc8618_send_data()
{
	//�ָ�������·
	if(mc8618_ip_connected())
	{
		while ((mc8618_signal_lost()))
		{
			delay_ms(3000);
		}
			if (GSM_send_cmd((u8*)"at+zipsend=0,470",(u8*)">",3))
			{
				//ȡ����
			}
				//��������
	}
	else
	{
		mc8618_conn();
	}
}


/*******************************************************************************
* ������ : CLR_Buf2
* ����   : �������2��������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<USART_REC_LEN;k++)      //��������������
	{
		USART2_RX_BUF[k] = 0x00;
	}
    First_Int = 0;              //�����ַ�������ʼ�洢λ��
}

/*******************************************************************************
* ������ : Find
* ����   : �жϻ������Ƿ���ָ�����ַ���
* ����   : 
* ���   : 
* ����   : unsigned char:1 �ҵ�ָ���ַ���0 δ�ҵ�ָ���ַ� 
* ע��   : 
*******************************************************************************/

u8 Find(char *a)
{ 
  if(strstr((const char*)USART2_RX_BUF,a)==NULL)
	    return 1;
	else
			return 0;
}

/*******************************************************************************
* ������ : Second_AT_Command
* ����   : ����ATָ���
* ����   : �������ݵ�ָ�롢���͵ȴ�ʱ��(��λ��S)
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/

u8 GSM_send_cmd(u8 *cmd,u8 *ack,u8 wait_time)         
{
	u8 res=0;
	//u8 *c;
	//c = cmd;										//�����ַ�����ַ��c
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
