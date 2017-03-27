#include "sys.h" 	
#include "includes.h"
#include "gprs_sh.h"

/////////////////////////UCOSII任务设置///////////////////////////////////

//GPS任务
//设置任务优先级
#define UBLOX_TASK_PRIO       							2 
//设置任务堆栈大小
#define UBLOX_STK_SIZE  									512
//任务堆栈
OS_STK UBLOX_TASK_STK[UBLOX_STK_SIZE];
//任务函数
void ublox_task(void *pdata);

//传感器数据获取任务
//设置任务优先级
#define SENSOR_TASK_PRIO       							3 
//设置任务堆栈大小
#define SENSOR_STK_SIZE  									128
//任务堆栈
OS_STK SENSOR_TASK_STK[SENSOR_STK_SIZE];
//任务函数
void sensor_task(void *pdata);

//数据存储任务
//设置任务优先级
#define DATA_TASK_PRIO       								4 
//设置任务堆栈大小
#define DATA_STK_SIZE  		    						128
//任务堆栈	
OS_STK DATA_TASK_STK[DATA_STK_SIZE];
//任务函数
void data_save_task(void *pdata);

//对方平台 通信任务
//设置任务优先级
#define OTHER_PLATFORM_TASK_PRIO       			5 
//设置任务堆栈大小
#define OTHER_PLATFORM_STK_SIZE  					512
//任务堆栈
OS_STK OTHER_PLATFORM_TASK_STK[OTHER_PLATFORM_STK_SIZE];
//任务函数
void other_platform_task(void *pdata);

//车佳平台 通信任务
//设置任务优先级
#define CJET_PLATFORM_TASK_PRIO       			6 
//设置任务堆栈大小
#define CJET_PLATFORM_STK_SIZE  					512
//任务堆栈
OS_STK CJET_TASK_STK[CJET_PLATFORM_STK_SIZE];
//任务函数
void cjet_platform_task(void *pdata);

//log输入设置
//设置任务优先级
#define LOGREV_TASK_PRIO       							7 
//设置任务堆栈大小
#define LOGREV_STK_SIZE  									128
//任务堆栈
OS_STK LOGREV_TASK_STK[LOGREV_STK_SIZE];
//任务函数
void logrev_task(void *pdata);


//START 任务
//设置任务优先级
#define START_TASK_PRIO      								10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  										64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	

OS_EVENT *Sensor_Box;
OS_EVENT *Sensor_data;

int main(void)
{
	bsp_init();
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	  	 
}
	  
//开始任务
void start_task(void *pdata)
{
	uint8_t err;
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	Sensor_Box = OSMboxCreate((void *)0);
	Sensor_data = OSMutexCreate(0, &err);
  OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)
  OSTaskCreate(ublox_task,(void *)0,(OS_STK*)&UBLOX_TASK_STK[UBLOX_STK_SIZE-1],UBLOX_TASK_PRIO);
	OSTaskCreate(sensor_task,(void *)0,(OS_STK*)&SENSOR_TASK_STK[SENSOR_STK_SIZE-1],SENSOR_TASK_PRIO);
	OSTaskCreate(logrev_task,(void *)0,(OS_STK*)&LOGREV_TASK_STK[LOGREV_STK_SIZE-1],LOGREV_TASK_PRIO);	
 	OSTaskCreate(data_save_task,(void *)0,(OS_STK*)&DATA_TASK_STK[DATA_STK_SIZE-1],DATA_TASK_PRIO);						   
 	OSTaskCreate(other_platform_task,(void *)0,(OS_STK*)&OTHER_PLATFORM_TASK_STK[OTHER_PLATFORM_STK_SIZE-1],OTHER_PLATFORM_TASK_PRIO);	
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

//UBLOX任务
void ublox_task(void *pdata)
{
	uint8_t err;	
	while(1)
	{
		OSMutexPend(Sensor_data,0,&err);
		Ublox_Analasis();
		OSMutexPost(Sensor_data);
		delay_ms(1000);
	}
}

//传感器数据刷新任务
void sensor_task(void *pdata)
{
	uint8_t err;
	while(1)
	{
		OSMutexPend(Sensor_data,0,&err);
		get_Sensor();
		OSMutexPost(Sensor_data);
		delay_ms(900);
	}
}


//other_platform任务
void other_platform_task(void *pdata)
{	  
	while(1)
	{
		delay_ms(1000);
	}
}

//cjet_platform任务
void cjet_platform_task(void *pdata)
{
	uint8_t signal = 0;
	u8 err;
	while(1)
	{
		if(OSMboxPend(Sensor_Box,0,&err))
		{
			conn_to_sh_platform(signal);
		}
		delay_ms(1000);
	}
}

//数据存储任务
void data_save_task(void *pdata)
{
	uint8_t err;
	char finish = 0;
	
	while(1)
	{
		OSMutexPend(Sensor_data,0,&err);
	  
		//finish = 1;
		OSMboxPost(Sensor_Box,&finish);//数据存储任务,数据足够
		
		OSMutexPost(Sensor_data);
		delay_ms(1000);
	}
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






