#include "sys.h" 	
#include "delay.h"	
#include "led.h"
#include "includes.h"
#include "usart.h"
#include "simulate_usart.h"
#include "timer.h"
#include "sdio_sdcard.h"
#include "ILI93xx.h"
#include "gps.h"
#include "can.h"
#include "exfuns.h"
#include "ff.h"
#include "w25qxx.h"  
#include "gprs_jiankong.h"

/////////////////////////UCOSII��������///////////////////////////////////

//GPS����
//�����������ȼ�
#define UBLOX_TASK_PRIO       		2 
//���������ջ��С
#define UBLOX_STK_SIZE  				512
//�����ջ
OS_STK UBLOX_TASK_STK[UBLOX_STK_SIZE];
//������
void ublox_task(void *pdata);

//���������ݻ�ȡ����
//�����������ȼ�
#define SENSOR_TASK_PRIO       			3 
//���������ջ��С
#define SENSOR_STK_SIZE  					128
//�����ջ
OS_STK SENSOR_TASK_STK[SENSOR_STK_SIZE];
//������
void sensor_task(void *pdata);

//log��������
//�����������ȼ�
#define LOGREV_TASK_PRIO       			8 
//���������ջ��С
#define LOGREV_STK_SIZE  					128
//�����ջ
OS_STK LOGREV_TASK_STK[LOGREV_STK_SIZE];
//������
void logrev_task(void *pdata);

//LED1����
//�����������ȼ�
#define LED1_TASK_PRIO       			6 
//���������ջ��С
#define LED1_STK_SIZE  					128
//�����ջ
OS_STK LED1_TASK_STK[LED1_STK_SIZE];
//������
void led1_task(void *pdata);

//LED0����
//�����������ȼ�
#define LED0_TASK_PRIO       			7 
//���������ջ��С
#define LED0_STK_SIZE  		    		128
//�����ջ	
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//������
void led0_task(void *pdata);

//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	

int main(void)
 {
	u32 res,temp,ret;
	u32 dtsize,dfsize;
	 
	delay_init();	    //��ʱ������ʼ��
  NVIC_Configuration();	 
	LED_Init();		  	//��ʼ����LED���ӵ�Ӳ���ӿ�
	TIM5_Configuration();
	uart_init(9600);
	Simulate_Usart();
	W25QXX_Init();				//��ʼ��W25Q64
	TFTLCD_Init();
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_LoopBack);
//  while (SD_Init())
//	{
//		printf("%d\n",SD_Init());
//		printf("SD Card Error!");
//		delay_ms(500);
//	}		
//	show_sdcard_info();	//��ӡSD�������Ϣ
	exfuns_init();							//Ϊfatfs��ر��������ڴ�				 
 	res=f_mount(fs[1],"1:",1); 				//����FLASH.
 	 do
	{
		temp++;
 		res=exf_getfree("1:",&dtsize,&dfsize);//�õ�FLASHʣ��������������
		delay_ms(200);		   
	}while(res&&temp<20);//�������20��
	
	if(res==0X0D)////�ļ�ϵͳ������ FLASH����,FAT�ļ�ϵͳ����,���¸�ʽ��FLASH
	{
		printf("Flash Disk Formatting...\n");	//��ʽ� �FLASH
		res=f_mkfs("1:",1,4096);//��ʽ��FLASH,1,�̷�;1,����Ҫ������,8������Ϊ1����
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:data");	//����Flash���̵�����Ϊ��ALIENTEK
			printf("Flash Disk Format Finish\n");	//��ʽ�����
			res=exf_getfree("1:",&dtsize,&dfsize);//���»�ȡ����
		}
		else 
		printf("Flash Disk Format Error \n");	//��ʽ��ʧ��
	}
	if(0 == res)
	{
		printf("Flash Disk:   %d  KB", dfsize);
		temp=dtsize;
	}
	else 
		printf("Flash Fat Error!\n");	//��ʽ��ʧ��
  f_unlink("1:log.txt");
	ret = f_open(data_log,"1:log.txt",FA_CREATE_ALWAYS | FA_WRITE| FA_READ);

	Is_Enter_facotry();
	//Ublox_init();
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	  	 
}
	  
//��ʼ����
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)
  OSTaskCreate(ublox_task,(void *)0,(OS_STK*)&UBLOX_TASK_STK[UBLOX_STK_SIZE-1],UBLOX_TASK_PRIO);
	OSTaskCreate(sensor_task,(void *)0,(OS_STK*)&SENSOR_TASK_STK[SENSOR_STK_SIZE-1],SENSOR_TASK_PRIO);
	OSTaskCreate(logrev_task,(void *)0,(OS_STK*)&LOGREV_TASK_STK[LOGREV_STK_SIZE-1],LOGREV_TASK_PRIO);	
 	OSTaskCreate(led0_task,(void *)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);						   
 	OSTaskCreate(led1_task,(void *)0,(OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],LED1_TASK_PRIO);	
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}

//UBLOX����
void ublox_task(void *pdata)
{	  
	while(1)
	{
		//Ublox_Analasis();
		delay_ms(100);
	}
}

//LED0����
void sensor_task(void *pdata)
{	 	
	while(1)
	{
		//Can_test();
		delay_ms(900);
	}
}


//LED1����
void led1_task(void *pdata)
{	  
	while(1)
	{
	printf("led1\n");
		LED1=0;
		delay_ms(300);
		LED1=1;
		delay_ms(300);
	};
}


//LED0����
void led0_task(void *pdata)
{	 	
	while(1)
	{
		printf("led0\n");
		LED0=0;
		delay_ms(80);
		LED0=1;
		delay_ms(920);
	};
}

void logrev_task(void *pdata)
{	 	
	while(1)
	{
		//printf("led0\n");
		LED0=0;
		delay_ms(80);
		LED0=1;
		delay_ms(920);
	};
}






