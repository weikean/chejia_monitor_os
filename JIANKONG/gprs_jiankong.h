#ifndef __GPRS_JIANKONG
#define __GPRS_JIANKONG

#include <stdbool.h>
#include <string.h>
#include <stdlib.h> 
#include <ctype.h> 
#include <stdio.h>
#include "stm32f10x.h"
#include "user_config.h"
#include "usart.h"
#include "delay.h"
#include "math.h"
#include "simulate_usart.h"
#include "timer.h"
#include "can.h"
#include "usart2.h"
#include "w25qxx.h"
#include "ILI93xx.h"
#include "mc8618.h"
#include "sdio_sdcard.h"
#include "gps.h"
#include "usart.h"
#include "timer.h"
#include "exfuns.h"
#include "ff.h" 
#include "led.h"
#include "malloc.h"
#include "gprs_sh.h"
#ifdef STM32F10X_HD

#define FLASH_SAVE_CJET_ADDR					0x0807FF70
#define FLASH_SAVE_DESTINATION_ADDR		0x0807FFA0
#define FLASH_SAVE_ID									0x0807FFD0
#define FLASH_SAVE_PHONE 							0x0807FFE0
#define FLASH_SAVE_FAC 								0x0807FFF0

#endif

void bsp_init(void);
void Is_Enter_facotry(void);
bool isValidIP(const char *p);
int pick_up_ip(const char *ipinfo, char *ip_port);
bool isValidID(const char *id);
int pick_up_id(const char *idinfo, char *id);
bool isValidPhone(const char *phone);
int pick_up_phone(const char *phoneinfo, char *phone);
int pick_up_fac(const char *facinfo, char *fac);
bool isValidFac(const char *fac);
u8 monitor_factory(void);
int idcheck(const char *p, int num);
void get_Sensor(void);
void flash_fatfs_init(void);
void sd_fatfs_init(void);
void get_device_id(void);

typedef struct MONITOR_INFO
{
 char ip_info1[30];
 char ip_info2[30];
 char id[14];
 char phone[12];
 char fac_num[5];

} monitor_info;

typedef struct Sensor
{
	int front_temperature;
	int middle_temperature;
	int back_temperatue;
	int front_pressure;
	int middle_pressure;
	int back_pressure;
} monitor_sensor;

extern monitor_sensor m_sensor;
extern monitor_info m_info;
extern u8 gps_flag;
#endif
