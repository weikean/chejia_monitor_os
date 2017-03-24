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

/////////////////////////UCOSIIÈÎÎñÉèÖÃ///////////////////////////////////

//GPSÈÎÎñ
//ÉèÖÃÈÎÎñÓÅÏÈ¼¶
#define UBLOX_TASK_PRIO       		2 
//ÉèÖÃÈÎÎñ¶ÑÕ»´óĞ¡
#define UBLOX_STK_SIZE  				512
//ÈÎÎñ¶ÑÕ»
OS_STK UBLOX_TASK_STK[UBLOX_STK_SIZE];
//ÈÎÎñº¯Êı
void ublox_task(void *pdata);

//´«¸ĞÆ÷Êı¾İ»ñÈ¡ÈÎÎñ
//ÉèÖÃÈÎÎñÓÅÏÈ¼¶
#define SENSOR_TASK_PRIO       			3 
//ÉèÖÃÈÎÎñ¶ÑÕ»´óĞ¡
#define SENSOR_STK_SIZE  					128
//ÈÎÎñ¶ÑÕ»
OS_STK SENSOR_TASK_STK[SENSOR_STK_SIZE];
//ÈÎÎñº¯Êı
void sensor_task(void *pdata);

//logÊäÈëÉèÖÃ
//ÉèÖÃÈÎÎñÓÅÏÈ¼¶
#define LOGREV_TASK_PRIO       			8 
//ÉèÖÃÈÎÎñ¶ÑÕ»´óĞ¡
#define LOGREV_STK_SIZE  					128
//ÈÎÎñ¶ÑÕ»
OS_STK LOGREV_TASK_STK[LOGREV_STK_SIZE];
//ÈÎÎñº¯Êı
void logrev_task(void *pdata);

//LED1ÈÎÎñ
//ÉèÖÃÈÎÎñÓÅÏÈ¼¶
#define LED1_TASK_PRIO       			6 
//ÉèÖÃÈÎÎñ¶ÑÕ»´óĞ¡
#define LED1_STK_SIZE  					128
//ÈÎÎñ¶ÑÕ»
OS_STK LED1_TASK_STK[LED1_STK_SIZE];
//ÈÎÎñº¯Êı
void led1_task(void *pdata);

//LED0ÈÎÎñ
//ÉèÖÃÈÎÎñÓÅÏÈ¼¶
#define LED0_TASK_PRIO       			7 
//ÉèÖÃÈÎÎñ¶ÑÕ»´óĞ¡
#define LED0_STK_SIZE  		    		128
//ÈÎÎñ¶ÑÕ»	
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//ÈÎÎñº¯Êı
void led0_task(void *pdata);

//START ÈÎÎñ
//ÉèÖÃÈÎÎñÓÅÏÈ¼¶
#define START_TASK_PRIO      			10 //¿ªÊ¼ÈÎÎñµÄÓÅÏÈ¼¶ÉèÖÃÎª×îµÍ
//ÉèÖÃÈÎÎñ¶ÑÕ»´óĞ¡
#define START_STK_SIZE  				64
//ÈÎÎñ¶ÑÕ»	
OS_STK START_TASK_STK[START_STK_SIZE];
//ÈÎÎñº¯Êı
void start_task(void *pdata);	

int main(void)
 {
	u32 res,temp,ret;
	u32 dtsize,dfsize;
	 
	delay_init();	    //ÑÓÊ±º¯Êı³õÊ¼»¯
  NVIC_Configuration();	 
	LED_Init();		  	//³õÊ¼»¯ÓëLEDÁ¬½ÓµÄÓ²¼ş½Ó¿Ú
	TIM5_Configuration();
	uart_init(9600);
	Simulate_Usart();
	W25QXX_Init();				//³õÊ¼»¯W25Q64
	TFTLCD_Init();
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_LoopBack);
//  while (SD_Init())
//	{
//		printf("%d\n",SD_Init());
//		printf("SD Card Error!");
//		delay_ms(500);
//	}		
//	show_sdcard_info();	//´òÓ¡SD¿¨Ïà¹ØĞÅÏ¢
	exfuns_init();							//ÎªfatfsÏà¹Ø±äÁ¿ÉêÇëÄÚ´æ				 
 	res=f_mount(fs[1],"1:",1); 				//¹ÒÔØFLASH.
 	 do
	{
		temp++;
 		res=exf_getfree("1:",&dtsize,&dfsize);//µÃµ½FLASHÊ£ÓàÈİÁ¿ºÍ×ÜÈİÁ¿
		delay_ms(200);		   
	}while(res&&temp<20);//Á¬Ğø¼ì²â20´Î
	
	if(res==0X0D)////ÎÄ¼şÏµÍ³²»´æÔÚ FLASH´ÅÅÌ,FATÎÄ¼şÏµÍ³´íÎó,ÖØĞÂ¸ñÊ½»¯FLASH
	{
		printf("Flash Disk Formatting...\n");	//¸ñÊ½» ¯FLASH
		res=f_mkfs("1:",1,4096);//¸ñÊ½»¯FLASH,1,ÅÌ·û;1,²»ĞèÒªÒıµ¼Çø,8¸öÉÈÇøÎª1¸ö´Ø
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:data");	//ÉèÖÃFlash´ÅÅÌµÄÃû×ÖÎª£ºALIENTEK
			printf("Flash Disk Format Finish\n");	//¸ñÊ½»¯Íê³É
			res=exf_getfree("1:",&dtsize,&dfsize);//ÖØĞÂ»ñÈ¡ÈİÁ¿
		}
		else 
		printf("Flash Disk Format Error \n");	//¸ñÊ½»¯Ê§°Ü
	}
	if(0 == res)
	{
		printf("Flash Disk:   %d  KB", dfsize);
		temp=dtsize;
	}
	else 
		printf("Flash Fat Error!\n");	//¸ñÊ½»¯Ê§°Ü
  f_unlink("1:log.txt");
	ret = f_open(data_log,"1:log.txt",FA_CREATE_ALWAYS | FA_WRITE| FA_READ);

	Is_Enter_facotry();
	//Ublox_init();
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//´´½¨ÆğÊ¼ÈÎÎñ
	OSStart();	  	 
}
	  
//¿ªÊ¼ÈÎÎñ
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  OS_ENTER_CRITICAL();			//½øÈëÁÙ½çÇø(ÎŞ·¨±»ÖĞ¶Ï´ò¶Ï)
  OSTaskCreate(ublox_task,(void *)0,(OS_STK*)&UBLOX_TASK_STK[UBLOX_STK_SIZE-1],UBLOX_TASK_PRIO);
	OSTaskCreate(sensor_task,(void *)0,(OS_STK*)&SENSOR_TASK_STK[SENSOR_STK_SIZE-1],SENSOR_TASK_PRIO);
	OSTaskCreate(logrev_task,(void *)0,(OS_STK*)&LOGREV_TASK_STK[LOGREV_STK_SIZE-1],LOGREV_TASK_PRIO);	
 	OSTaskCreate(led0_task,(void *)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);						   
 	OSTaskCreate(led1_task,(void *)0,(OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],LED1_TASK_PRIO);	
	OSTaskSuspend(START_TASK_PRIO);	//¹ÒÆğÆğÊ¼ÈÎÎñ.
	OS_EXIT_CRITICAL();				//ÍË³öÁÙ½çÇø(¿ÉÒÔ±»ÖĞ¶Ï´ò¶Ï)
}

//UBLOXÈÎÎñ
void ublox_task(void *pdata)
{	  
	while(1)
	{
		//Ublox_Analasis();
		delay_ms(100);
	}
}

//LED0ÈÎÎñ
void sensor_task(void *pdata)
{	 	
	while(1)
	{
		//Can_test();
		delay_ms(900);
	}
}


//LED1ÈÎÎñ
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


//LED0ÈÎÎñ
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






