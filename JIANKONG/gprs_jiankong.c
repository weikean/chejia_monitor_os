/********************************************
*�����Ҫ�Ĺ���
* 
*******************************************/
#include "gprs_jiankong.h"
#include "flash.h"
extern __IO uint8_t receivedFlag;

void Is_Enter_facotry(void)
{
	Time_fac = 1;
	printf("����5s�ӽ��빤��ģʽ\n");
	while(Time_fac <= 6 )
	{
		if(strstr((const char * )dataReceived, "����"))
		{
		Clr_Usart_Data();
		monitor_factory();
		}
	}
	 Time_fac = 0;
}

/*******************************************************************************
* ������ : GSM_gprs_test
* ����   : GPRS����ģʽ

*******************************************************************************/
u8 monitor_factory(void) 
{
    //Ŀ�꣺����޸�ip,�˿�,�޸��豸id���˳�����ģʽ�Ĺ���
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

 
printf("@@@@@@@@@@@@@@���ѽ��빤��ģʽ@@@@@@@@@@@@@@@@@@@@@\r\n");
//printf("����125.89.18.79,12345�س�\r\n");
//printf("����125.89.18.79,12345�س�\r\n");
//printf("�豸��239828378927�س�\r\n");
//printf("�ֻ���18576422291�س�\r\n");    
//printf("���ұ���+cj01+�س�\r\n");    
//printf("�˳�����ģʽ���˳�+�س�\r\n"); 

printf("����at+zipsetup=0,202.2.1.0,5000\r\n");
printf("����125.89.18.79,12345�س�\r\n");
printf("�豸��239828378927�س�\r\n");
printf("�ֻ���18576422291�س�\r\n");    
printf("���ұ���+cj01+�س�\r\n");    
printf("�˳�����ģʽ���˳�+�س�\r\n");  

while (exit == 0) 
{
  if (1 == receivedFlag) 
	{
		receivedFlag = 0;//��λ���ձ�־
		if (strstr((const char * ) dataReceived, "����")) mode = 0; //����ip�Ͷ˿�
		else if (strstr((const char * ) dataReceived, "�豸��")) mode = 1; //�޸��豸id
		else if (strstr((const char * ) dataReceived, "�ֻ���")) mode = 2; //�޸��ֻ���
		else if (strstr((const char * ) dataReceived, "���ұ���")) mode = 4; //�޸ĳ��ұ���
		else if (strstr((const char * ) dataReceived, "����")) mode = 0; //����ip�Ͷ˿�
		else if (strstr((const char * ) dataReceived, "�˳�")) mode = 3; //�˳�����ģʽ
		else 
			Clr_Usart_Data();
	}
	
  switch (mode) 
	{
  case 0:
		cjet_ip_len = pick_up_ip((const char*)dataReceived,ip_data);
		if (isValidIP(ip_data)) 
		{
			if(strstr((const char*)dataReceived, "����"))
			{
				 STMFLASH_Write(FLASH_SAVE_CJET_ADDR, (u16 * ) ip_data, cjet_ip_len);
				 memset(ip_data,0,strlen(ip_data));
				 //read at once
				 STMFLASH_Read(FLASH_SAVE_CJET_ADDR	, (u16 * ) ip_data, cjet_ip_len);
				 printf("cjet ip is:%s\n", ip_data);
			}
			else if (strstr((const char*)dataReceived, "����"))
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

