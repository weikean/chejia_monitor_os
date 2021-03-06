/********************************************
*监控需要的功能
* 
*******************************************/
#include "gprs_jiankong.h"
#include "flash.h"
extern __IO uint8_t receivedFlag;

monitor_sensor m_sensor;
monitor_info m_info;

void bsp_init(void)
{
	u8 ret;
	delay_init();	    //延时函数初始�
	NVIC_Configuration();	 
	LED_Init();		  	//初始化与LED连接的硬件接口
	TIM5_Configuration();
	//mem_init(SRAMIN);		//初始化内部内存池
	uart_init(9600);
	uart2_init(9600);
	Simulate_Usart();
	W25QXX_Init();				//初始化W25Q64
	TFTLCD_Init();
	mc8618_power_on();
	Ublox_init();
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_LoopBack);
	flash_fatfs_init();
	//sd_fatfs_init();
  f_unlink("1:log.txt");
	ret = f_open(data_log,"1:log.txt",FA_CREATE_ALWAYS | FA_WRITE| FA_READ);

	Is_Enter_facotry();
	
}

void flash_fatfs_init(void)
{
	u32 res,temp;
	u32 dtsize,dfsize;
	
 	 do
	{
		temp++;
 		res=exf_getfree("1:",&dtsize,&dfsize);//得到FLASH剩余容量和总容量
		delay_ms(200);		   
	}while(res&&temp<20);//连续检测20次
	
	if(res==0X0D)////文件系统不存在 FLASH磁盘,FAT文件系统错误,重新格式化FLASH
	{
		printf("Flash Disk Formatting...\n");	//格式� 疐LASH
		res=f_mkfs("1:",1,4096);//格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:data");	//设置Flash磁盘的名字为：ALIENTEK
			printf("Flash Disk Format Finish\n");	//格式化完成
			res=exf_getfree("1:",&dtsize,&dfsize);//重新获取容量
		}
		else 
		printf("Flash Disk Format Error \n");	//格式化失败
	}
	if(0 == res)
	{
		printf("Flash Disk:   %d  KB", dfsize);
		temp=dtsize;
	}
	else 
		printf("Flash Fat Error!\n");	//格式化失败
	
	exfuns_init();							//为fatfs相关变量申请内存				 
 	res=f_mount(fs[1],"1:",1); 				//挂载FLASH.
}

void sd_fatfs_init(void)
{	
	while (SD_Init())
	{
		printf("%d\n",SD_Init());
		printf("SD Card Error!");
		delay_ms(500);
	}		
	show_sdcard_info();	//打印SD卡相关信息
	exfuns_init();							//为fatfs相关变量申请内存				 
  f_mount(fs[0],"0:",1); 					//挂载SD卡 

}

void Is_Enter_facotry(void)
{
	Time_fac = 1;
	printf("您有5s钟进入工厂模式\n");
	while(Time_fac <= 6 )
	{
		if(strstr((const char * )dataReceived, "工厂"))
		{
		Clr_Usart_Data();
		monitor_factory();
		}
	}
	 Time_fac = 0;
}

void get_device_id(void)
{
	STMFLASH_Read(FLASH_SAVE_ID, (u16 * ) m_info.id, ID_LEN);
	m_info.id[13] = '\0';
}

/*******************************************************************************
* 函数名 : monitor_factory
* 描述   : 监控工厂模式,存取监控的基本信息 。

*******************************************************************************/
u8 monitor_factory(void) 
{
    //目标：完成修改ip,端口,修改设备id，退出工厂模式的功能
u8 mode = 10;
u8 exit = 0;
char cjet_ip_len = 0;
char ip_data[50] = {0};
char id_data[14] = {0};
char id_len = 0;	
char phone[12] = {0};
char  phone_len = 0;
char fac_num[5] = {0};	
char fac_len = 0;

 
printf("@@@@@@@@@@@@@@您已进入工厂模式@@@@@@@@@@@@@@@@@@@@@\r\n");
printf("车佳at+zipsetup=0,202.2.1.0,5000\r\n");
printf("河南125.89.18.79,12345回车\r\n");
printf("设备号239828378927回车\r\n");
printf("手机号18576422291回车\r\n");    
printf("厂家编码+cj01+回车\r\n");    
printf("退出工厂模式：退出+回车\r\n");  

while (exit == 0) 
{
  if (1 == receivedFlag) 
	{
		receivedFlag = 0;//复位接收标志
		if (strstr((const char * ) dataReceived, "车佳")) mode = 0; //设置ip和端口
		else if (strstr((const char * ) dataReceived, "设备号")) mode = 1; //修改设备id
		else if (strstr((const char * ) dataReceived, "手机号")) mode = 2; //修改手机号
		else if (strstr((const char * ) dataReceived, "厂家编码")) mode = 4; //修改厂家编码
		else if (strstr((const char * ) dataReceived, "政府")) mode = 0; //设置ip和端口
		else if (strstr((const char * ) dataReceived, "退出")) mode = 3; //退出工厂模式
		else 
			Clr_Usart_Data();
	}
	
  switch (mode) 
	{
  case 0:
		cjet_ip_len = pick_up_ip((const char*)dataReceived,ip_data);
		if (isValidIP(ip_data)) 
		{
			if(strstr((const char*)dataReceived, "车佳"))
			{
				 STMFLASH_Write(FLASH_SAVE_CJET_ADDR, (u16 * ) ip_data, cjet_ip_len);
				 memset(ip_data,0,strlen(ip_data));
				 //read at once
				 STMFLASH_Read(FLASH_SAVE_CJET_ADDR	, (u16 * ) ip_data, cjet_ip_len);
				 printf("cjet ip is:%s\n", ip_data);
			}
			else if (strstr((const char*)dataReceived, "政府"))
			{
				STMFLASH_Write(FLASH_SAVE_DESTINATION_ADDR, (u16 * ) ip_data, cjet_ip_len);
				memset(ip_data,0,strlen(ip_data));
				
				STMFLASH_Read(FLASH_SAVE_DESTINATION_ADDR, (u16 * ) ip_data, cjet_ip_len);
				printf("gov ip is:%s\n", ip_data);
			}
			else
			{
				printf("your ip format is error\n");
			}
	 }
		else
		{
			printf("your ip format is error\n");
		}
		Clr_Usart_Data();
		mode = 10;
  break;
	
  case 1:
		id_len = pick_up_id((const char*)dataReceived,id_data);
		if (isValidID(id_data))
		{
		  STMFLASH_Write(FLASH_SAVE_ID, (u16 * ) id_data, id_len);
			memset(id_data, 0, strlen(id_data));
			STMFLASH_Read(FLASH_SAVE_ID, (u16 * ) id_data, id_len);
			printf("id is%s\n",id_data);
		}
		else
		{
			printf("your id format is error\n");
		}
		Clr_Usart_Data();
		mode = 10;
  break;
		
  case 2:
		phone_len = pick_up_phone((const char*)dataReceived,phone);
		if (isValidPhone(phone))
		{
			printf("%s\n", phone);
		  STMFLASH_Write(FLASH_SAVE_PHONE, (u16 * ) phone, phone_len);
			memset(phone,0,strlen(phone));
			STMFLASH_Read(FLASH_SAVE_PHONE, (u16 * ) phone, phone_len);
			printf("phone number has saved \r\n");
		}
		else
		{
			printf("your phone format is error\n");
		}
		Clr_Usart_Data();
		mode = 10;		
	break;

  case 3:
    exit = 1;
  break;

  case 4:
		fac_len = pick_up_fac((const char*)dataReceived,fac_num);
		if(isValidFac(fac_num))
		{
			printf("fac is%s\n",fac_num);
		//STMFLASH_Write(FLASH_SAVE_FAC, (u16 * ) fac_num, fac_len);
		}
		else
		{
			printf("factory format is error\n");
		}
		Clr_Usart_Data();
		mode = 10;		
  break;

  case 5:
  break;
	
	default:
		break;
}
}
	return 0;
}

bool isValidIP(const char *p)
{	
	char temp[4];
	const char *split = ",";
  int count=0;
	char *ip = NULL, *port = NULL;
	char *pNext = NULL;
	char ip_t[50];
	
	memcpy(ip_t,p,strlen(p));
	strtok_r (ip_t,split,&pNext);
	ip = strtok_r(NULL, split, &pNext);
	//printf("split ip %s\n", ip);
	
	port = strtok_r(NULL, split, &pNext);
	//printf("split port %s\n", port);
	
	if (!(atoi(port) >= 0 && atoi(port) <= 65535))
		return false;
	if (ip==NULL)
    return false;
	
  while (true)
	{
   int index=0;
		while (*ip!='\0' && *ip!='.' && count<4)
		{
			temp[index++]=*ip;
			ip++;
		}
     if (index==4)
     return false;
		
     temp[index]='\0';
     int num=atoi(temp);
		 
     if (!(num>=0 && num<=255))
     return false;
		 
     count++;
     if (*ip=='\0'){
        if (count==4)
          return true;
        else
          return false;
     }else
      ip++;
    }
}

//split chinese
int pick_up_ip(const char* ipinfo , char *ip_port)
{
	memcpy(ip_port,ipinfo+4,IP_LEN);
	ip_port[strlen(ipinfo)-4] = '\0';
	//printf("ip,port is %s\n",ip_port);
	return strlen(ip_port);
}

bool isValidID(const char *id)
{	
	if (strlen(id) != 13)
		return false;
  if (strstr(id,"CJET") != NULL)
		return true;
	else
		return false;
}

//split chinese
int pick_up_id(const char* idinfo , char *id)
{
	memcpy(id,(char*)idinfo+6,ID_LEN);
	id[strlen((char*)idinfo)-6] = '\0';
	return strlen(id);
}

bool isValidPhone(const char *phone)
{
	int i;
	
	if (strlen(phone) != 11)
		return false;
	for (i = 0 ;i < 11 ;i++)
	{
		if (phone[i] >= '0' && phone[i] <= '9')
			return true;
	else
			return false;
	}
	return false;
}
//split chinese
int pick_up_phone(const char* phoneinfo , char *phone)
{
	memcpy(phone,phoneinfo+6,PHONE_LEN);
	phone[strlen(phoneinfo)-6] = '\0';
	printf("phone number is %s\n",phone);
	return strlen(phone);
}

bool isValidFac(const char *fac)
{
  if (strstr(fac,"cj") != NULL)
		return true;
	else
		return false;
}

int pick_up_fac(const char* facinfo , char *fac)
{
	memcpy(fac,facinfo+8,FAC_LEN);
	fac[strlen(facinfo)-8] = '\0';
	return strlen(fac);
}

/*
*传感器数据采集
*/
void get_Sensor(void)
{
	monitor_sensor *m_sensor_temp;
	
	Can_Receive_Msg((u8*)m_sensor_temp);
	
	m_sensor.front_temperature = m_sensor_temp->front_temperature;
	m_sensor.middle_temperature = m_sensor_temp->middle_temperature;
	m_sensor.back_temperatue = m_sensor_temp->back_temperatue;
	m_sensor.front_pressure = m_sensor_temp->front_pressure;
	m_sensor.middle_pressure = m_sensor_temp->middle_pressure;
	m_sensor.back_pressure = m_sensor_temp->back_pressure;
	
}
