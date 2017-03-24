/********************************************
*监控需要的功能
* 
*******************************************/
#include "gprs_jiankong.h"
#include "flash.h"
extern __IO uint8_t receivedFlag;

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

/*******************************************************************************
* 函数名 : GSM_gprs_test
* 描述   : GPRS工厂模式

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
//printf("车佳125.89.18.79,12345回车\r\n");
//printf("河南125.89.18.79,12345回车\r\n");
//printf("设备号239828378927回车\r\n");
//printf("手机号18576422291回车\r\n");    
//printf("厂家编码+cj01+回车\r\n");    
//printf("退出工厂模式：退出+回车\r\n"); 

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

