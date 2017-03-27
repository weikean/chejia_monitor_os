#include "sys.h" 	
#include "includes.h"
#include "gprs_sh.h"

/////////////////////////UCOSII��������///////////////////////////////////

//GPS����
//�����������ȼ�
#define UBLOX_TASK_PRIO       							2 
//���������ջ��С
#define UBLOX_STK_SIZE  									512
//�����ջ
OS_STK UBLOX_TASK_STK[UBLOX_STK_SIZE];
//������
void ublox_task(void *pdata);

//���������ݻ�ȡ����
//�����������ȼ�
#define SENSOR_TASK_PRIO       							3 
//���������ջ��С
#define SENSOR_STK_SIZE  									128
//�����ջ
OS_STK SENSOR_TASK_STK[SENSOR_STK_SIZE];
//������
void sensor_task(void *pdata);

//���ݴ洢����
//�����������ȼ�
#define DATA_TASK_PRIO       								4 
//���������ջ��С
#define DATA_STK_SIZE  		    						128
//�����ջ	
OS_STK DATA_TASK_STK[DATA_STK_SIZE];
//������
void data_save_task(void *pdata);

//�Է�ƽ̨ ͨ������
//�����������ȼ�
#define OTHER_PLATFORM_TASK_PRIO       			5 
//���������ջ��С
#define OTHER_PLATFORM_STK_SIZE  					512
//�����ջ
OS_STK OTHER_PLATFORM_TASK_STK[OTHER_PLATFORM_STK_SIZE];
//������
void other_platform_task(void *pdata);

//����ƽ̨ ͨ������
//�����������ȼ�
#define CJET_PLATFORM_TASK_PRIO       			6 
//���������ջ��С
#define CJET_PLATFORM_STK_SIZE  					512
//�����ջ
OS_STK CJET_TASK_STK[CJET_PLATFORM_STK_SIZE];
//������
void cjet_platform_task(void *pdata);

//log��������
//�����������ȼ�
#define LOGREV_TASK_PRIO       							7 
//���������ջ��С
#define LOGREV_STK_SIZE  									128
//�����ջ
OS_STK LOGREV_TASK_STK[LOGREV_STK_SIZE];
//������
void logrev_task(void *pdata);


//START ����
//�����������ȼ�
#define START_TASK_PRIO      								10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  										64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	

OS_EVENT *Sensor_Box;
OS_EVENT *Sensor_data;

int main(void)
{
	bsp_init();
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	  	 
}
	  
//��ʼ����
void start_task(void *pdata)
{
	uint8_t err;
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	Sensor_Box = OSMboxCreate((void *)0);
	Sensor_data = OSMutexCreate(0, &err);
  OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)
  OSTaskCreate(ublox_task,(void *)0,(OS_STK*)&UBLOX_TASK_STK[UBLOX_STK_SIZE-1],UBLOX_TASK_PRIO);
	OSTaskCreate(sensor_task,(void *)0,(OS_STK*)&SENSOR_TASK_STK[SENSOR_STK_SIZE-1],SENSOR_TASK_PRIO);
	OSTaskCreate(logrev_task,(void *)0,(OS_STK*)&LOGREV_TASK_STK[LOGREV_STK_SIZE-1],LOGREV_TASK_PRIO);	
 	OSTaskCreate(data_save_task,(void *)0,(OS_STK*)&DATA_TASK_STK[DATA_STK_SIZE-1],DATA_TASK_PRIO);						   
 	OSTaskCreate(other_platform_task,(void *)0,(OS_STK*)&OTHER_PLATFORM_TASK_STK[OTHER_PLATFORM_STK_SIZE-1],OTHER_PLATFORM_TASK_PRIO);	
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}

//UBLOX����
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

//����������ˢ������
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


//other_platform����
void other_platform_task(void *pdata)
{	  
	while(1)
	{
		delay_ms(1000);
	}
}

//cjet_platform����
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

//���ݴ洢����
void data_save_task(void *pdata)
{
	uint8_t err;
	char finish = 0;
	
	while(1)
	{
		OSMutexPend(Sensor_data,0,&err);
	  
		//finish = 1;
		OSMboxPost(Sensor_Box,&finish);//���ݴ洢����,�����㹻
		
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






