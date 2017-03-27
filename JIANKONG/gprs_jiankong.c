/********************************************
*¼à¿ØÐèÒªµÄ¹¦ÄÜ
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
	delay_init();	    //ÑÓÊ±º¯Êý³õÊ¼»
	NVIC_Configuration();	 
	LED_Init();		  	//³õÊ¼»¯ÓëLEDÁ¬½ÓµÄÓ²¼þ½Ó¿Ú
	TIM5_Configuration();
	//mem_init(SRAMIN);		//³õÊ¼»¯ÄÚ²¿ÄÚ´æ³Ø
	uart_init(9600);
	uart2_init(9600);
	Simulate_Usart();
	W25QXX_Init();				//³õÊ¼»¯W25Q64
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
 		res=exf_getfree("1:",&dtsize,&dfsize);//µÃµ½FLASHÊ£ÓàÈÝÁ¿ºÍ×ÜÈÝÁ¿
		delay_ms(200);		   
	}while(res&&temp<20);//Á¬Ðø¼ì²â20´Î
	
	if(res==0X0D)////ÎÄ¼þÏµÍ³²»´æÔÚ FLASH´ÅÅÌ,FATÎÄ¼þÏµÍ³´íÎó,ÖØÐÂ¸ñÊ½»¯FLASH
	{
		printf("Flash Disk Formatting...\n");	//¸ñÊ½» ¯FLASH
		res=f_mkfs("1:",1,4096);//¸ñÊ½»¯FLASH,1,ÅÌ·û;1,²»ÐèÒªÒýµ¼Çø,8¸öÉÈÇøÎª1¸ö´Ø
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:data");	//ÉèÖÃFlash´ÅÅÌµÄÃû×ÖÎª£ºALIENTEK
			printf("Flash Disk Format Finish\n");	//¸ñÊ½»¯Íê³É
			res=exf_getfree("1:",&dtsize,&dfsize);//ÖØÐÂ»ñÈ¡ÈÝÁ¿
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
	
	exfuns_init();							//ÎªfatfsÏà¹Ø±äÁ¿ÉêÇëÄÚ´æ				 
 	res=f_mount(fs[1],"1:",1); 				//¹ÒÔØFLASH.
}

void sd_fatfs_init(void)
{	
	while (SD_Init())
	{
		printf("%d\n",SD_Init());
		printf("SD Card Error!");
		delay_ms(500);
	}		
	show_sdcard_info();	//´òÓ¡SD¿¨Ïà¹ØÐÅÏ¢
	exfuns_init();							//ÎªfatfsÏà¹Ø±äÁ¿ÉêÇëÄÚ´æ				 
  f_mount(fs[0],"0:",1); 					//¹ÒÔØSD¿¨ 

}

void Is_Enter_facotry(void)
{
	Time_fac = 1;
	printf("ÄúÓÐ5sÖÓ½øÈë¹¤³§Ä£Ê½\n");
	while(Time_fac <= 6 )
	{
		if(strstr((const char * )dataReceived, "¹¤³§"))
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
* º¯ÊýÃû : monitor_factory
* ÃèÊö   : ¼à¿Ø¹¤³§Ä£Ê½,´æÈ¡¼à¿ØµÄ»ù±¾ÐÅÏ¢ ¡£

*******************************************************************************/
u8 monitor_factory(void) 
{
    //Ä¿±ê£ºÍê³ÉÐÞ¸Äip,¶Ë¿Ú,ÐÞ¸ÄÉè±¸id£¬ÍË³ö¹¤³§Ä£Ê½µÄ¹¦ÄÜ
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

 
printf("@@@@@@@@@@@@@@ÄúÒÑ½øÈë¹¤³§Ä£Ê½@@@@@@@@@@@@@@@@@@@@@\r\n");
printf("³µ¼Ñat+zipsetup=0,202.2.1.0,5000\r\n");
printf("ºÓÄÏ125.89.18.79,12345»Ø³µ\r\n");
printf("Éè±¸ºÅ239828378927»Ø³µ\r\n");
printf("ÊÖ»úºÅ18576422291»Ø³µ\r\n");    
printf("³§¼Ò±àÂë+cj01+»Ø³µ\r\n");    
printf("ÍË³ö¹¤³§Ä£Ê½£ºÍË³ö+»Ø³µ\r\n");  

while (exit == 0) 
{
  if (1 == receivedFlag) 
	{
		receivedFlag = 0;//¸´Î»½ÓÊÕ±êÖ¾
		if (strstr((const char * ) dataReceived, "³µ¼Ñ")) mode = 0; //ÉèÖÃipºÍ¶Ë¿Ú
		else if (strstr((const char * ) dataReceived, "Éè±¸ºÅ")) mode = 1; //ÐÞ¸ÄÉè±¸id
		else if (strstr((const char * ) dataReceived, "ÊÖ»úºÅ")) mode = 2; //ÐÞ¸ÄÊÖ»úºÅ
		else if (strstr((const char * ) dataReceived, "³§¼Ò±àÂë")) mode = 4; //ÐÞ¸Ä³§¼Ò±àÂë
		else if (strstr((const char * ) dataReceived, "Õþ¸®")) mode = 0; //ÉèÖÃipºÍ¶Ë¿Ú
		else if (strstr((const char * ) dataReceived, "ÍË³ö")) mode = 3; //ÍË³ö¹¤³§Ä£Ê½
		else 
			Clr_Usart_Data();
	}
	
  switch (mode) 
	{
  case 0:
		cjet_ip_len = pick_up_ip((const char*)dataReceived,ip_data);
		if (isValidIP(ip_data)) 
		{
			if(strstr((const char*)dataReceived, "³µ¼Ñ"))
			{
				 STMFLASH_Write(FLASH_SAVE_CJET_ADDR, (u16 * ) ip_data, cjet_ip_len);
				 memset(ip_data,0,strlen(ip_data));
				 //read at once
				 STMFLASH_Read(FLASH_SAVE_CJET_ADDR	, (u16 * ) ip_data, cjet_ip_len);
				 printf("cjet ip is:%s\n", ip_data);
			}
			else if (strstr((const char*)dataReceived, "Õþ¸®"))
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
*´«¸ÐÆ÷Êý¾Ý²É¼¯
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
