/**********************************************************************************
 * 文件名  ：gprs_sh.c
 * 描述    ：上海机动车检测通信协议的监控程序实现方法      
 * 实验平台： Sim808
 * 硬件连接： TXD(PA2)  -> 外部串口RXD     
 *           RXD(PA3) -> 外部串口TXD      
 *           GND	   -> 外部串口GND 
 * 引用自泥人电子
**********************************************************************************/

#include "gprs_jiankong.h"
#include "string.h"
#include "usart.h"	  
#include "user_config.h"
#include "gsm.h"
#include "SysTick.h"
#include "stdlib.h"
#include <ctype.h>
#include <stdio.h>
#include "radix.h"
#include "delay.h"
#include "gprs_sh.h"
#include "shh.h"
#include "hexstring.h"
#include "ff.h"  
#include "exfuns.h" 
#include "mmc_sd.h"


//#define TEST_VIN "\x41\x42\x43\x44\x45\x46\x47\x48\x49\x50\x51\x52\x53\x54\x55\x56\x57"
#define TEST_VIN_LEN (17)
#define TEST_TIME "\x09\x08\x07\x06\x05\x10"
#define TEST_TIME_LEN (6)
#define TEST_VID (0x00019136)
#define TEST_GPS "\x41\x1f\x12\x43\x56\x4e\x79\x34\x52\x67\x45\x22\x99\x00"
#define TEST_GPS_LEN (14)
#define TEST_CMD "\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55"
#define TEST_CMD_LEN (17)


u8 conn_flag0 = 0;
u8 conn_flag1 = 0;
u8 reg_ok = 0;
u8 flag = 0;
int data_times = 0;
char hexid[20];
char sh_id[17];
u8 conn_flag = 0;
u8 reg_flag = 0;
int conn_fail = 1;//连接失败
char ip_fail = 0;//ip地址错误
uint32_t vid;
extern gps_info gps;

u8 temp[128];
u8 order_flag = 0;
int file_option = 0;
int file_finish = 0;
u8 sending = 0;
u8 data_ok = 1;
static int send_flag = 0;
static int power_on = 1;
int recieving = 0;
char gps_time_temp[13];
static char vid_buf[8];
static u8 off_exist = 0;
static void uint32_to_le32_bytes(uint32_t x, uint8_t *p)
{
	p[0] = (x) & 0xFF;
	p[1] = (x >> 8) & 0xFF;
	p[2] = (x >> 16) & 0xFF;
	p[3] = (x >> 24) & 0xFF;
}

void sh_task(void)
{  
	  char *p1 = NULL,*p2 = NULL;
	  char hextemp[128];
 	 
	  if(reg_ok == 0)
		{	
	  if((p1=(char*)strstr((const char*)Uart2_Buf,"+RECEIVE,0,")),(p1!=NULL))//寻找开始符
		{		
		 if((p2=(char*)strstr((char*)p1,"\x0d\x0a")),(p2!=NULL))//寻找结束符
		{
		 memcpy(temp,p2+2,40);
		 temp[40] = '\0';
		 BytesToHexString(temp,40,hextemp);
		 if(strstr(hextemp,"4152") && strstr(hextemp,"2100"))
		 {
			 printf("reg answer ackownledged\r\n");
			 reg_response();
			 reg_ok = 1;
			 reg_ack = 0;
			 CLR_Buf2();
			 heart_miao = 1;
			 power_on = 0;
			 sh_close();
		 }

// 		 if(strstr(hextemp,"4144") && strstr(hextemp,"ffee"))
// 		 {
// 			 CLR_Buf2(); 
// 		 }
	 }
	 }
 }
    if(power_on == 1)
   {
		 reg_server();
		 delay_ms(8000);//注册回复有时会很慢
   }	
	 else if(heart_miao >= 180)
	 {		 
		 heartbeat();		 
	 } 

 	 else
			data_manage();			
}


/*************************************
* 向服务器注册的方法 
  发送10次不成功，休息15分钟再来注册
*
*************************************/

void reg_server(void)
{
	int i = 0;
	uint8_t* reg = NULL ;
	static u8 reg_conn_counter = 0;

	if(reg_conn_counter < 50 || sh_reg_miao >= 900)
	{
		if(sh_reg_miao >= 900){sh_reg_miao = 0;reg_conn_counter = 0;}
		if(reg_conn_counter >= 9){sh_reg_miao = 1;}
		reg_conn_counter++;
	 
	if(conn_flag == 1)
	{
		if(power_on == 1 || reg_ack >= 10)
		{
		reg = send_reg_frame();
		if(GSM_send_cmd("AT+CIPSEND=0",">",8)==0)
		{
    for(i = 0;i<32;i++)
    UART2_Data(reg[i]);				
		UART2_Data(0X1A);//CTRL+Z,结束数据发送,启动一次传输
		UART1_SendString("已发送\r\n");
		reg_ack = 1;
	  }
	}
 }
 else
		sh_conn();

}
}
/*************************************
* 每三分钟发一次心跳包，等待10秒接收服务器的指令帧，收到指令帧后
  发送指令应答帧，最后关闭连接，10秒内未收到指令帧，则直接关闭连接
*
*************************************/
void heartbeat(void)
{

	u8 *heart = NULL;
	u8 i;
	static int heart_conn_counter = 0;
// 	static u8 heart_ok = 0;
// 	if(order_flag == 1)
// 	{//回复指令应答帧
// 	sh_close();
// 	heart_miao = 1;
// 	heart_ok = 0;
// 	}	
// 	else if(heart_miao >= 190)
// 	{
// 		printf("heart time out\r\n");
// 	  sh_close();
// 		heart_miao = 1;
// 		heart_ok = 0;
// 	}
  if(heart_conn_counter < 5 )
 {
	 if(conn_flag == 0)
		sh_conn();
		heart_conn_counter++;
	if(conn_flag == 1 )
  {
	heart = send_heartbeat();
	if(GSM_send_cmd("AT+CIPSEND=0",">",8)==0){
   for(i = 0;i<20;i++)
   UART2_Data(heart[i]);				
	 UART2_Data(0X1A);//CTRL+Z,结束数据发送,启动一次传输
   UART1_SendString("heart has sent\r\n");
//	 heart_ok = 1;
	 heart_miao = 1;
	 heart_conn_counter = 0;
	if(file_finish == 0 && off_exist == 0)
	{
	 delay_ms(1000);
   sh_close();
	}	
  }	
  }
}
 else
 {
		heart_conn_counter = 0;
	  heart_miao = 1;
	}
}


void data_manage(void)
{
  uint8_t data_frame[941];
	struct _data_frame data;
	static int g_flag = 0;
	static int off_flag = 1;
	static u8 gps_in_send = 0;
	static int retry = 0;
	static int send_fail = 0;
//	int ret = 0,
	  int i = 0;
//	static uint8_t last_frame[47];
	char *hexbuf;
	
	if(file_finish == 1)
	{
		
	if(Time_send <= 800 && Time_send > 0)
	{
	if(gps_in_send >= 45) gps_in_send = 0;
	gps_in_send++;
	if(gps_in_send % 4 == 0){get_GPS_data();}


	
	if(conn_flag == 1)
	{ 
	 //sending = 1;		
   hexbuf = malloc(sizeof((char)(1881)));	
	 sd_decode(data_frame);
	 BytesToHexString(data_frame, 940, hexbuf);
 
				 
	 if(findStr(hexbuf, vid_buf) == 20)
	 {
	 if(GSM_send_cmd("AT+CIPSEND=0,940",">",8)==0)
	 {
	 send_flag++;	
	 printf("%d ",send_flag);
	 for(i = 0;i<940;i++)
   UART2_Data(data_frame[i]);
		 
	 if(send_flag % 5 == 0 || send_fail >= 1)
	 {
   if(wait_to_success() == 0)
	 {
		 send_fail++;
		 single_data_offline(send_flag);
		 printf("存入离线文件\n");
	 }
   else {printf("发送成功\n"); send_fail = 0;}
   }
	 else {printf("发送成功\n");}
   CLR_Buf2();
	 
	 }
	 else
	 {
	  printf("sendcmd error\n");
		for(i = 0;i<940;i++)
    UART2_Data(data_frame[i]);
    GSM_send_cmd("AT+CIPCLOSE=0","CLOSE OK",4);	//关闭连接
 		GSM_send_cmd("AT+CIPSHUT","SHUT OK",4);		//关闭移动场景，关闭后状态为ip_initial
		GSM_send_cmd("AT+CGCLASS=\"B\"","OK",6);//设置GPRS移动台类别为B,支持包交换和数据交换
		GSM_send_cmd("AT+CGATT=1","OK",6);//附着GPRS业务
		GSM_send_cmd("AT+CIPMUX=1","OK",6);//设置为多路连接
		GSM_send_cmd("AT+CSTT=\"CMNET\"","OK",6);//设置为GPRS连接模式，将ip_initial变为ipstart
		GSM_send_cmd("AT+CIICR","OK",6);// 激活移动场景,ip_start状态下有效
		UART2_SendString("AT+CIFSR\r\n");//查模块IP，这个必须有
		printf("连接已经重置\n");
		conn_flag = 0;
	 }
   free(hexbuf)	; 
	 }
	 else  
	 {
		 send_flag++;
		 printf("data err\n");
		 //发现错误帧，如果第一帧就是错误帧
//		 if(send_flag >= 1)
//		 make_frame_right(lastframe);
   }
 }

	 
 // 一直连接不上
 	else 
	{
		sh_conn();
	}
	
	
	
		
	if(send_flag == 45) 
   {
     send_flag = 0;
     file_finish = 0;
		 Time_send = 0;
		 if(file_option == 0)
			{
				f_close(log1);
		    f_open(log1,"0:log1.txt",FA_CREATE_ALWAYS | FA_WRITE| FA_READ);
			}
		 else if(file_option == 1)
			{	
				f_close(log);
			  f_open(log,"0:log.txt",FA_CREATE_ALWAYS | FA_WRITE| FA_READ);
			}
			delay_ms(8000);
			sh_close();
// 			heart_miao = 1;
    }
 }
 
 		 //超时
   if(Time_send >= 800)
	  {
		 printf("发送超时\n");
		 left_data_offline(send_flag);
		 printf("数据写入离线\n");
		 Time_send = 0;
     file_finish = 0; 
		 if(file_option == 0)
			{
				f_close(log1);
		    f_open(log1,"0:log1.txt",FA_CREATE_ALWAYS | FA_WRITE| FA_READ);
			}
		 else if(file_option == 1)
			{	
			 	f_close(log);
			  f_open(log,"0:log.txt",FA_CREATE_ALWAYS | FA_WRITE| FA_READ);
			}
			send_flag = 0;
	  }
  }
   //离线文件有一组(20条)数据
  else if(f_size(offline) >= sizeof(data) *20)
	{
	 printf("offline length is %ld\n",f_size(offline));
	 off_exist = 1;
	 if(gps_in_send >= 45) gps_in_send = 0;
	 gps_in_send++;
	 if(gps_in_send % 4 == 0){get_GPS_data();}	
		//发送离线数据
	 else
	 {
	 if(conn_flag == 1)
	 {
	 if(off_flag >= 46) off_flag = 0;	 
	 offline_decode(data_frame);
	 hexbuf = malloc(sizeof((char)(2000)));
	 BytesToHexString(data_frame, 940, hexbuf);
	 if(findStr(hexbuf, vid_buf) == 20)
	 {
	 retry = 0;
		 
	 do
	 {
   if(retry >= 1)delay_ms(2000);		 
	 if(retry >= 5){sh_close();retry  = 0;}	 
   retry++;
	 if(GSM_send_cmd("AT+CIPSEND=0,940",">",10)==0)
	 {
   CLR_Buf2();		 
	 printf("off_flag = %d\r\n",off_flag);
	 for(i = 0;i<940;i++)
   UART2_Data(data_frame[i]);
	 }
	 
	 else
	 {
		printf("offline sendcmd error\n");
		for(i = 0;i<940;i++)
    UART2_Data(data_frame[i]);
    GSM_send_cmd("AT+CIPCLOSE=0","CLOSE OK",4);	//关闭连接
 		GSM_send_cmd("AT+CIPSHUT","SHUT OK",4);		//关闭移动场景，关闭后状态为ip_initial
		GSM_send_cmd("AT+CGCLASS=\"B\"","OK",6);//设置GPRS移动台类别为B,支持包交换和数据交换
		GSM_send_cmd("AT+CGATT=1","OK",6);//附着GPRS业务
		GSM_send_cmd("AT+CIPMUX=1","OK",6);//设置为多路连接
		GSM_send_cmd("AT+CSTT=\"CMNET\"","OK",6);//设置为GPRS连接模式，将ip_initial变为ipstart
		GSM_send_cmd("AT+CIICR","OK",6);// 激活移动场景,ip_start状态下有效
		UART2_SendString("AT+CIFSR\r\n");//查模块IP，这个必须有
		printf("连接已经重置\n");
		conn_flag = 0;	 
   }
	 }while(wait_to_success() == 0);
   off_flag++;	 
   CLR_Buf2();
	 free(hexbuf);
   }
	 else printf("offline data error\n");
	 } 
	 else {sh_conn();}
 }
 }
 //不传输文件就取定位
 else
	{	
		off_exist = 0;
		if(conn_flag == 1)
		sh_close();
		if(g_flag >= 3)
		{
			g_flag = 0;
			get_GPS_data(); 		
		}
    g_flag++;
	}
  
}


/**********************************
*根据协议生成注册帧
**********************************/
uint8_t* send_reg_frame(void)
{
// 	uint8_t expect_result[] = "\x33\x34\x1c\x00\x11\x11\x11\x11\x00\x52\x45\x41\x42\x43\x44\x45\x46\x47\x48\x49\x50\x51\x52\x53\x54\x55\x56\x57\x00\xff\xff\xee";
// 	uint32_t expect_len = 32;

	struct shh_reg_request reg_req;
	uint8_t buf_req[128];
	uint8_t buf[128];
	int32_t req_len;
//	int32_t ret;
  char sh_id[17];
	uint8_t *p = NULL;
	shh_reset();
	shh_set_stx(STX_RELEASE);
	shh_enable_cksum(false);

	reg_req.frame_type = FRAME_RE;
	strcpy(sh_id,device);
	memcpy(sh_id+strlen(device),"FFFF",17-strlen(device));
	memcpy(reg_req.vin, sh_id, TEST_VIN_LEN);
	reg_req.send_count = 0;
	req_len = shh_reg_request_encode(buf_req, sizeof(buf_req), &reg_req);
	shh_encode(buf, sizeof(buf), buf_req, req_len, 0);
	printf("注册id=%s\n",device);
	p = buf;
//	ASSERT(ret == expect_len && memcmp(buf, expect_result, expect_len) == 0, "shh_encode failed ret=%u expect_len=%u", (unsigned int)ret, (unsigned int)expect_len);
// 	if (!(ret == expect_len && memcmp(buf, expect_result, expect_len) == 0))
// 	{
// 		char hexbuf[128];
// 		BytesToHexString(buf, 32, hexbuf);
// 		printf("R:%s\n", hexbuf);
// 		
// 	}
	return p;
}
	

void reg_response(void)
{
// 	uint8_t sample[] = "\x33\x34\x21\x00\x36\x91\x01\x00\x00\x41\x52\x41\x42\x43\x44\x45\x46\x47\x48\x49\x50\x51\x52\x53\x54\x55\x56\x57\x09\x08\x07\x06\x05\x10\xff\xff\xee";
 	uint32_t sample_len = 37;
  uint8_t reg_response[37];
	struct shh_reg_response reg_res;
	uint8_t buf_res[76];
//	uint8_t buf[256];
	int32_t res_len;
//	int32_t ret;
	uint16_t frame_type;
	uint8_t vid_buf_1[8];
	shh_reset();
	shh_set_stx(STX_RELEASE);
	shh_enable_cksum(false);
  memcpy(reg_response,temp,37);
	res_len = shh_decode(reg_response, sample_len, buf_res, sizeof(buf_res), &frame_type);

	vid = shh_get_vid();
  printf("vid=0x%08x", vid);
	shh_reg_response_decode(buf_res, res_len, &reg_res);
  uint32_to_le32_bytes(vid,vid_buf_1);
	BytesToHexString(vid_buf_1, 4, vid_buf);
	printf("R:%s\n", vid_buf);
}

 
 /***************************
 *心跳帧格式
 *
 ****************************/
uint8_t *send_heartbeat(void)
{
	struct shh_heartbeat_request hb_req;
	uint8_t buf_req[34];
	uint8_t buf[34];
	int32_t req_len;
	char sh_time_t[6] = "";
	char time_t[12] = "";
	char ntp_time[12] = "";
	char hextime_t[12] = "";
	
	uint8_t *p = NULL;
//	uint8_t nib;
 	p = buf;
	
	memset(sh_time_t,0,sizeof(sh_time_t));
	memset(time_t,0,sizeof(time_t));
	memset(ntp_time,0,sizeof(ntp_time));
	shh_reset();
	shh_set_stx(STX_RELEASE);
	shh_enable_cksum(false);
	shh_set_vid(vid);
	hb_req.frame_type = FRAME_HB;

	if(strlen(gps_time_temp) == 12)
	{
	//+1s
 	sh_time_convert(gps_time_temp,time_t,0);	
	time_to_hextime(time_t,hextime_t);
  HexStringToBytes(hextime_t,12,sh_time_t);
	}

	memcpy(hb_req.time, sh_time_t, TEST_TIME_LEN);
	req_len = shh_heartbeat_request_encode(buf_req, sizeof(buf_req), &hb_req);
	shh_encode(buf, sizeof(buf), buf_req, req_len, 0);
	return p;
}
 
 //数据帧
 
uint8_t* send_data_frame(void)
{
// 	uint8_t expect_result[] = "\x33\x34\x2b\x00\x36\x91\x01\x00\x00\x44\x41\x11\x69\x01\x00\x13\x00\x23\x00\x34\x00\x33\x55\x53\x41\x1f\x12\x43\x56\x4e\x79\x34\x52\x67\x45\x22\x99\x00\x09\x08\x07\x06\x05\x10\xff\xff\xee";
// 	uint32_t expect_len = 47;

	struct shh_data_request data_req;
	uint8_t buf_req[128];
	uint8_t buf[128];
	int32_t req_len;
  static int32_t sn = 0; //帧序号
	char hexspeed[3];
	char hextemp[4];
	char hextemp1[4];
	char hextemp2[4];
	char hextemp3[4];
	
	char hexp1[2];
	char hexp2[2];
	char hexp3[2];
	
  char sh_time_t[6] = "";
	char time_t[12] = "";
//	char ntp_time[12];
	char hextime_t[12] = "";
	
	char hexstatus[3];
	char lati_1[2];
	char lati_2[2];
	char lati_3[2];
	char lati_4[2];
	char hexlati_1[2];
	char hexlati_2[2];
	char hexlati_3[2];
	char hexlati_4[2];
	char hex_direc[3];
	char long_1[3];
	char long_2[2];
	char long_3[2];
	char long_4[2];
	char hexlong_1[2];
	char hexlong_2[2];
	char hexlong_3[2];
	char hexlong_4[2];
	char hex_direc1[3];
	char course[4];
	char hexcourse_1[3];
	char hexcourse_2[3];
  char hexs[6];
	static int distance = 0;

	char gps_hex[30];
	char gps_byte[14];
	uint8_t *p = NULL;
	static int refresh = 1;
	static int flag= 0 ;
 
	

	if(sn == 255) sn = 0;
	sn++;
	memset(gps_hex,0,sizeof(gps_hex));
	shh_reset();
	shh_set_stx(STX_RELEASE);
	shh_enable_cksum(false);
	// shh_set_vid only for test
	shh_set_vid(vid);
	data_req.frame_type = FRAME_DA;

  //里程数换算
	data_req.mileage = 0x000000;
	if(atoi(gps.gps_speed) >= 0 && strlen(gps.gps_speed) <= 4)
	{
  distance = distance + atoi(gps.gps_speed)/4;
	sprintf(hexs,"%06X",(distance/1000));
	//printf("distance is %d\r\n",distance);
	//printf("hexkm = %s",hexs)	;
	HexStringToBytes_still(hexs,6,(char*)&(data_req.mileage));
	//printf("hexkm is 0x%06x\r\n",data_req.mileage);
	}
//	data_req.mileage = 0x000169;
	memset(hextemp,0,sizeof(hextemp));
	if(atoi(temp1) <= 999)
	sprintf(hextemp,"%04X",atoi(temp1));
	else 
	sprintf(hextemp,"%04X",999);
	sprintf(hextemp1,"%s%c%c",hextemp+2,*hextemp,*(hextemp+1));
	HexStringToBytes(hextemp1,4,(char*)&(data_req.DOCT1));
	
	memset(hextemp,0,sizeof(hextemp));
	if(atoi(temp2) <= 999)
	sprintf(hextemp,"%04X",atoi(temp2));
	else
	sprintf(hextemp,"%04X",999);
	sprintf(hextemp2,"%s%c%c",hextemp+2,*hextemp,*(hextemp+1));
	HexStringToBytes(hextemp2,4,(char*)&(data_req.CDPFT2));
	
	memset(hextemp,0,sizeof(hextemp));
	if(atoi(temp3) <= 999)
	sprintf(hextemp,"%04X",atoi(temp3));
	else 
	sprintf(hextemp,"%04X",999);
	sprintf(hextemp3,"%s%c%c",hextemp+2,*hextemp,*(hextemp+1));
	HexStringToBytes(hextemp3,4,(char*)&(data_req.CDPFT3));
	
	if((atoi(press1)/1000) <= 63)
	sprintf(hexp1,"%02X",atoi(press1)/1000);
	else
	sprintf(hexp1,"%02X",0);
	HexStringToBytes(hexp1,2,(char*)&(data_req.DCOP1));

  if((atoi(press2)/1000) <= 63)
	sprintf(hexp2,"%02X",atoi(press2)/1000);
	else
	sprintf(hexp2,"%02X",0);
	HexStringToBytes(hexp2,2,(char*)&(data_req.CDPFP2));
	
	if((atoi(press3)/1000) <= 63)
	sprintf(hexp3,"%02X",atoi(press3)/1000);
	else
	sprintf(hexp3,"%02X",0);
	HexStringToBytes(hexp3,2,(char*)&(data_req.CDPFP3));

  if(strlen(gps.gps_longitude) >= 10 && strlen(gps.gps_latitude) >= 9)
	{
		sprintf(hexstatus,"%02X",(char)'A');
	//	printf("lati = %s\r\n",gps.gps_latitude);
		memcpy(lati_1,gps.gps_latitude,2);
		memcpy(lati_2,gps.gps_latitude+2,2);
		memcpy(lati_3,gps.gps_latitude+4,2);
		memcpy(lati_4,gps.gps_latitude+6,2);
// 		printf("lati_1 = %s\r\n",lati_1);
// 		printf("lati_2 = %s\r\n",lati_2);
// 		printf("lati_3 = %s\r\n",lati_3);
// 		printf("lati_4 = %s\r\n",lati_4);
		
		sprintf(hexlati_1,"%02X",atoi(lati_1));
		sprintf(hexlati_2,"%02X",atoi(lati_2));
		sprintf(hexlati_3,"%02X",atoi(lati_3));
		sprintf(hexlati_4,"%02X",atoi(lati_4));
// 		printf("hexlati_1 = %s\r\n",hexlati_1);
// 		printf("hexlati_2 = %s\r\n",hexlati_2);
// 		printf("hexlati_3 = %s\r\n",hexlati_3);
// 		printf("hexlati_4 = %s\r\n",hexlati_4);
		
		sprintf(hex_direc,"%02X",(char)'N');
		hex_direc[2] = '\0';
//		printf("hex_direc = %s\r\n",hex_direc);
    memcpy(long_1,gps.gps_longitude,3);
		memcpy(long_2,gps.gps_longitude+3,2);
		memcpy(long_3,gps.gps_longitude+5,2);
		memcpy(long_4,gps.gps_longitude+7,2);
// 		printf("long_1 = %s\r\n",long_1);
// 		printf("long_2 = %s\r\n",long_2);
// 		printf("long_3 = %s\r\n",long_3);
// 		printf("long_4 = %s\r\n",long_4);
		
		sprintf(hexlong_1,"%02X",atoi(long_1));
		sprintf(hexlong_2,"%02X",atoi(long_2));
		sprintf(hexlong_3,"%02X",atoi(long_3));
		sprintf(hexlong_4,"%02X",atoi(long_4));
//		printf("hexlong_1 = %s\r\n",hexlong_1);
// 		printf("hexlong_2 = %s\r\n",hexlong_2);
// 		printf("hexlong_3 = %s\r\n",hexlong_3);
// 		printf("hexlong_4 = %s\r\n",hexlong_4);
		sprintf(course,"%04X",atoi(gps.gps_course));
 	//	printf("course = %s\r\n",gps.gps_course);
		memcpy(hexcourse_1,course+2,2);
		hexcourse_1[2] = '\0';
		memcpy(hexcourse_2,course,2);
		hexcourse_2[2] = '\0';
// 		printf("couse1 = %s\r\n",hexcourse_1);
// 		printf("couse2 = %s\r\n",hexcourse_2);
		

		
// 		printf("speed = %s\r\n",gps.gps_speed_not);
	  sprintf(hexspeed,"%02X",atoi(gps.gps_speed_not));
		hexspeed[2] = '\0';
// 	  printf("hexspeed = %s\r\n",hexspeed);
	  HexStringToBytes(hexspeed,2,(char*)&data_req.speed);
		

			
		sprintf(hex_direc1,"%02X",(char)'E');
		hex_direc1[2] = '\0';
//		printf("hex_direc1 = %s\r\n",hex_direc1);
		strcat(gps_hex,hexstatus);
		strcat(gps_hex,hexlati_1);
		strcat(gps_hex,hexlati_2);
		strcat(gps_hex,hexlati_3);
		strcat(gps_hex,hexlati_4);
		strcat(gps_hex,hex_direc);
		strcat(gps_hex,hexlong_1);
		strcat(gps_hex,hexlong_2);
		strcat(gps_hex,hexlong_3);
		strcat(gps_hex,hexlong_4);
		strcat(gps_hex,hex_direc1);
		strcat(gps_hex,hexspeed);
		strcat(gps_hex,hexcourse_1);
		strcat(gps_hex,hexcourse_2);
		
	}
	else
	{	
    strcat(gps_hex,"\x56\x00\x00\x00\x00\x4e\x00\x00\x00\x00\x45\x00\x00\x00");		
	}
  HexStringToBytes(gps_hex,28,gps_byte);
	memcpy(data_req.gps, gps_byte, TEST_GPS_LEN);
	if(flag >= 900)
 {flag = 0;
//refresh = 1;
}
	flag++;
  if(strlen(gps.gps_time) == 12 && refresh == 1)
	{ memcpy(gps_time_temp,gps.gps_time,12);memcpy(my_clock,gps.gps_time,12);gps_time_temp[12] = '\0'; refresh = 0;printf("check");}
	
	if(strlen(gps_time_temp) == 12)
	{
	//+1s
  //plus_one_second(gps_time_temp);
	//printf(my_clock);
 	sh_time_convert(my_clock,time_t,0);	
	time_to_hextime(time_t,hextime_t);
  HexStringToBytes(hextime_t,12,sh_time_t);
	}
	
	memcpy(data_req.time, sh_time_t, TEST_TIME_LEN);

	req_len = shh_data_request_encode(buf_req, sizeof(buf_req), &data_req);

	shh_encode(buf, sizeof(buf), buf_req, req_len, sn);
  p = buf;
// 	char hexbuf2[95];
// 	BytesToHexString(buf, 47, hexbuf2);
// 	printf("\n%s\n",hexbuf2);
	return p;
}


void sh_conn(void)
{
 if(conn_fail >= 10)
 {
	  conn_fail = 0;
		GSM_send_cmd("AT+CIPCLOSE=0","CLOSE OK",6);	//关闭连接
 		GSM_send_cmd("AT+CIPSHUT","SHUT OK",6);		//关闭移动场景，关闭后状态为ip_initial
		GSM_send_cmd("AT+CGCLASS=\"B\"","OK",8);//设置GPRS移动台类别为B,支持包交换和数据交换
		GSM_send_cmd("AT+CGATT=1","OK",8);//附着GPRS业务
		GSM_send_cmd("AT+CIPMUX=1","OK",10);//设置为多路连接
		GSM_send_cmd("AT+CSTT=\"CMNET\"","OK",10);//设置为GPRS连接模式，将ip_initial变为ipstart
		GSM_send_cmd("AT+CIICR","OK",10);// 激活移动场景,ip_start状态下有效
		UART2_SendString("AT+CIFSR\r\n");//查模块IP，这个必须有
	  printf("连接已经重置");

 }
 if(GSM_send_cmd("AT+CIPSTART=0,\"TCP\",\"118.178.84.185\",9000","OK",15) == 0)//发起连接
 {
  ip_fail = 0;
  UART1_SendString("正在发起连接\r\n");
	Conn_miao=1;
 while(Conn_miao <= 5)
 {
 if(strstr((const char*)Uart2_Buf,"0, CONNECT OK"))//连接成功
 {
	UART1_SendString("信号0连接成功\r\n");
	conn_flag = 1;
	conn_fail = 0;
	break;	
 }							
	else if(strstr((const char*)Uart2_Buf,"0, ALREAY CONNECT"))//连接成功
 {
	UART1_SendString("信号0已经连接\r\n");
	conn_flag = 1;
	conn_fail = 0;
	break;
 }
	else if(strstr((const char*)Uart2_Buf,"0, CONNECT FAIL"))//连接失败
{
	UART1_SendString("信号0连接失败\r\n");
	conn_fail ++;
	break;	
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
	else if(strstr((const char*)Uart2_Buf,"ERROR"))//连接出错
{
	UART1_SendString("信号0连接出错\r\n");
	conn_fail ++;
	break;	
}
	//else conn_fail ++;
}
  conn_fail ++;
}
	else
{
	if(strstr((const char*)Uart2_Buf,"0, ALREADY CONNECT"))//连接成功
	{
		GSM_send_cmd("AT+CIPCLOSE=0","CLOSE OK",4);	//关闭连接
	}
	else
	{
		UART1_SendString("ip地址无效\r\n");
		get_GPS_data();
		if(conn_fail ++ >= 5)
		{
		conn_fail = 0;
		GSM_send_cmd("AT+CIPCLOSE=0","CLOSE OK",6);	//关闭连接
 		GSM_send_cmd("AT+CIPSHUT","SHUT OK",6);		//关闭移动场景，关闭后状态为ip_initial
		GSM_send_cmd("AT+CGCLASS=\"B\"","OK",8);//设置GPRS移动台类别为B,支持包交换和数据交换
		GSM_send_cmd("AT+CGATT=1","OK",8);//附着GPRS业务
		GSM_send_cmd("AT+CIPMUX=1","OK",10);//设置为多路连接
		GSM_send_cmd("AT+CSTT=\"CMNET\"","OK",10);//设置为GPRS连接模式，将ip_initial变为ipstart
		GSM_send_cmd("AT+CIICR","OK",10);// 激活移动场景,ip_start状态下有效
		UART2_SendString("AT+CIFSR\r\n");//查模块IP，这个必须有
	  printf("连接已经重置");
		}
	}
		
}
  CLR_Buf2();
}


void sh_close(void)
{
	conn_flag = 0;
// 	sending = 1;
	GSM_send_cmd("AT+CIPCLOSE=0","0, CLOSE OK",12);	//关闭连接,关闭后状态为ip_close
// 	sending = 0;
	UART1_SendString("close sh\r\n");
}

int sh_time_convert(char * utctime,char *sh_time,int way)
{
	char UTCYear[2];
	char UTCMonth[2];
	char UTCDay[2];
	char UTCHour[2];
	char UTCMinutes[2];
	char UTCSeconds[2];
  int year = 0;
	int month = 0;
	int day = 0;
	int sec = 0;
	int min = 0;
	int hour = 0;
  memcpy(UTCYear,utctime,2);
	memcpy(UTCMonth,utctime+2,2);
	memcpy(UTCDay,utctime+4,2);
	memcpy(UTCHour,utctime+6,2);
	memcpy(UTCMinutes,utctime+8,2);
	memcpy(UTCSeconds,utctime+10,2);
	
     year = atoi(UTCYear);  
     month = atoi(UTCMonth);  
     day = atoi(UTCDay);  
     sec = atoi(UTCSeconds);  
     min = atoi(UTCMinutes); 
		 if(way == 0) 
     hour = atoi(UTCHour)+8;
     if(way == 1)
		 hour = atoi(UTCHour)+6;	
     if (hour>23)  
        {  
        hour-=24;  
        day++;  
        switch (month)  
               {  
               case 1:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                    break;  
               case 2:  
                    if ((0==year%4 && 0!=year%100) || 0==year%400)  
                       {  
                       if (day>29)  
                          {  
                          day=1;  
                          month++;  
                          }  
                       }  
                    else  
                       {  
                       if (day>28)  
                          {  
                          day=1;  
                          month++;  
                          }  
                       }  
                   break;  
               case 3:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 4:  
                    if (day>30)  
                       {  
                       day=1;  
                       month++;  
                       }  
                    break;  
               case 5:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 6:  
                    if (day>30)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 7:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 8:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                    break;  
               case 9:  
                    if (day>30)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 10:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 11:  
                    if (day>30)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 12:  
                    if (day>31)  
                       {  
                       day=1;  
                       month=1;  
                       year++;  
                       }  
                    break;  
               default:break;  
               }  
        }			
        sprintf(sh_time,"%02d",hour);
				sprintf(sh_time+2,"%02d",min);
		    sprintf(sh_time+4,"%02d",sec);
		    sprintf(sh_time+6,"%02d",day);
		    sprintf(sh_time+8,"%02d",month);
		    sprintf(sh_time+10,"%02d",year);
		
        return 1;
	 
}

int time_to_hextime(char* time,char* hextime)
{
  char hexhour[2];
	char hexmin[2];
	char hexsec[2];
	char hexdate[2];
	char hexmonth[2];
	char hexyear[2];
	
	char hour[2];
	char min[2];
	char sec[2];
	char date[2];
	char month[2];
	char year[2];
	
	memcpy(hour,time,2);
	memcpy(min,time+2,2);
	memcpy(sec,time+4,2);
	memcpy(date,time+6,2);
	memcpy(month,time+8,2);
	memcpy(year,time+10,2);
	
// 	printf("hour = %s\r\n",hour);
// 	printf("min = %s\r\n",min);
// 	printf("year = %s\r\n",year);
	
	sprintf(hexhour,"%02X",atoi(hour));
	sprintf(hexmin,"%02X",atoi(min));
	sprintf(hexsec,"%02X",atoi(sec));
	sprintf(hexdate,"%02X",atoi(date));
	sprintf(hexmonth,"%02X",atoi(month));
	sprintf(hexyear,"%02X",atoi(year));
	
// 	printf("hex_hour = %s\r\n",hexhour);
// 	printf("hex_min = %s\r\n",hexmin);
// 	printf("hex_year = %s\r\n",hexyear);
	
	strcat(hextime,hexhour);
	strcat(hextime,hexmin);
	strcat(hextime,hexsec);
	strcat(hextime,hexdate);
	strcat(hextime,hexmonth);
	strcat(hextime,hexyear);
  return 0;
}

void sd_encode(void)
{
	struct _data_frame data;
	uint8_t *p = NULL;
	u8 err = 0;
	int ret ;
	p = send_data_frame();
	data.frame_mark = 0x3334;
	data.frame_len = 0x2B00;
	memcpy(data.vid,&p[4],4);
	data.sn = p[8];
	data.frame_cmd = 0x4441;
	data.speed = p[11];
	memcpy(data.mileage,&p[12],3);
	memcpy(data.DOCT1,&p[15],2);
	memcpy(data.CDPFT2,&p[17],2);
	memcpy(data.CDPFT3,&p[19],2);
	data.DCOP1 = p[21];
	data.CDPFP2 = p[22];
	data.CDPFP3 = p[23];
	memcpy(data.gps,&p[24],14);
	memcpy(data.time,&p[38],6);
	data.pwd_type = 0xFF;
	data.crc = 0xFF;
	data.frame_tail = 0xEE;
	switch(file_option)
	{
		case 0:
		OSMutexPend(Semsd,0,&err); 	
		f_lseek(log,log->fsize);
		ret = f_write(log,&data,sizeof(data),&bw);
		if(ret != FR_OK) printf("write0_error");
		f_sync(log);
		OSMutexPost(Semsd); 
		break;
		
		case 1:
		OSMutexPend(Semsd,0,&err); 	
		f_lseek(log1,log1->fsize);
		ret = f_write(log1,&data,sizeof(data),&bw);
		if(ret != FR_OK) printf("write1_error");
		f_sync(log1);
		OSMutexPost(Semsd); 
		break;
	}
}


void all_data_offline(void)
{
	struct _data_frame data_off;
	int len = 0,i;
	u8 err = 0;
	int ret  = 0;
	printf("数据全部写入\r\n");
	switch(file_option)
	{
		case 0:
		OSMutexPend(Semsd,0,&err);
    for(i = 0;i< 900;i++)
    {
    len = sizeof(data_off) * i;			
		f_lseek(log1,len);
	  ret = f_read(log1,&data_off,sizeof(data_off),&bw);
		if(ret != FR_OK) printf("offread1_error");
		f_lseek(offline,offline->fsize);
		ret = f_write(offline,&data_off,sizeof(data_off),&bw);
		f_sync(offline);
		if(ret != FR_OK) printf("w_offline_0_error");
		}
    OSMutexPost(Semsd); 		
		break;
		
		case 1:
		OSMutexPend(Semsd,0,&err);
		for(i = 0;i< 900;i++)
    {		
		len = sizeof(data_off) * i;			
		f_lseek(log,len);
	  ret = f_read(log,&data_off,sizeof(data_off),&bw);
		if(ret != FR_OK) printf("offead0_error");
		f_lseek(offline,offline->fsize);
		ret = f_write(offline,&data_off,sizeof(data_off),&bw);
		f_sync(offline);
		if(ret != FR_OK) printf("w_offline_1_error");
		}
		OSMutexPost(Semsd); 		
	  break;
	}
	

}

void single_data_offline(int flag)
{
	struct _data_frame data_left;
	int len = 0,i;
	u8 err = 0;
	int ret  = 0;
	switch(file_option)
	{
		case 0:
		OSMutexPend(Semsd,0,&err);
    for(i = ((flag-1) * 20)+1;i<= 20;i++)
    {
    len = sizeof(data_left) * i;			
		f_lseek(log1,len);
	  ret = f_read(log1,&data_left,sizeof(data_left),&bw);
		if(ret != FR_OK) printf("offread1_error");
		f_lseek(offline,offline->fsize);
		ret = f_write(offline,&data_left,sizeof(data_left),&bw);
		f_sync(offline);
		if(ret != FR_OK) printf("w_offline_0_error");
		}
    OSMutexPost(Semsd); 		
		break;
		
		case 1:
		OSMutexPend(Semsd,0,&err);
    for(i = ((flag-1) * 20)+1;i<= 20;i++)
    {	
		len = sizeof(data_left) * i;			
		f_lseek(log,len);
	  ret = f_read(log,&data_left,sizeof(data_left),&bw);
		if(ret != FR_OK) printf("offead0_error");
		f_lseek(offline,offline->fsize);
		ret = f_write(offline,&data_left,sizeof(data_left),&bw);
		f_sync(offline);	
		if(ret != FR_OK) printf("w_offline_1_error");
		}
		OSMutexPost(Semsd);	
	  break;
	}
}




void left_data_offline(int flag)
{
	struct _data_frame data_left;
	int len = 0,i;
	u8 err = 0;
	int ret  = 0;
	int oflag = 0;
	
	oflag = (flag * 20) +1;
	switch(file_option)
	{
		case 0:
		OSMutexPend(Semsd,0,&err);
    for(i = (flag * 20)+1;i<=900;i++)
    {
		if(oflag % 100 == 0)
		printf("oflag = %d\n",oflag);
		oflag++;
    len = sizeof(data_left) * i;			
		f_lseek(log1,len);
	  ret = f_read(log1,&data_left,sizeof(data_left),&bw);
		if(ret != FR_OK) printf("offread1_error");
		f_lseek(offline,offline->fsize);
		ret = f_write(offline,&data_left,sizeof(data_left),&bw);
		f_sync(offline);
		if(ret != FR_OK) printf("w_offline_0_error");
		}
    OSMutexPost(Semsd); 		
		break;
		
		case 1:
		OSMutexPend(Semsd,0,&err);
    for(i = (flag * 20)+1;i<= 900;i++)
    {
		if(oflag % 100 == 0)
		printf("oflag = %d\n",oflag);
    oflag++;		
		len = sizeof(data_left) * i;			
		f_lseek(log,len);
	  ret = f_read(log,&data_left,sizeof(data_left),&bw);
		if(ret != FR_OK) printf("offead0_error");
		f_lseek(offline,offline->fsize);
		ret = f_write(offline,&data_left,sizeof(data_left),&bw);
		f_sync(offline);
		if(ret != FR_OK) printf("w_offline_1_error");
		}
		OSMutexPost(Semsd);	
	  break;
	}
}

int offline_decode(uint8_t *databuf)
{
	uint8_t buf[47];	
	struct _data_frame data_d;
	int i,len = 0;
	int ret;
	int fail = 0;
  u8 err = 0;
  memset(databuf,0,sizeof(databuf));
  for(i = 0;i< 20;i++)
  {
	memset(buf,NULL,sizeof(buf));
		
  len = sizeof(data_d);
	
	OSMutexPend(Semsd,0,&err);
		
  if((offline->fsize) >= len)
	{		
	f_lseek(offline,(offline->fsize)-len);
  ret = f_read(offline,&data_d,sizeof(data_d),&bw);
	if(ret != FR_OK)
	{
		printflog(ret);
		if(fail > 10)
		{
			fail = 0;
			f_unlink("0:offline.txt");
			f_open(offline,"0:offline.txt", FA_CREATE_ALWAYS| FA_WRITE| FA_READ);
			return 0;
		}
		else fail++;
		printf("offdecode_error\n");
	}
	f_lseek(offline,(offline->fsize)-len);
	ret = f_truncate(offline);
	//ret = f_read(offline,&data_d,sizeof(data_d),&bw);
	//printflog(ret);
  OSMutexPost(Semsd); 		
	memcpy(&buf[0],"\x33\x34",2);
	memcpy(&buf[2],"\x2B\x00",2);
	memcpy(&buf[4],data_d.vid,4);
	buf[8] = data_d.sn;
	memcpy(&buf[9],"\x44\x41",2);
	
	buf[11] = data_d.speed;
	
	memcpy(&buf[12],data_d.mileage,3);
	memcpy(&buf[15],data_d.DOCT1,2);
	memcpy(&buf[17],data_d.CDPFT2,2);
	memcpy(&buf[19],data_d.CDPFT3,2);
		
	buf[21] = data_d.DCOP1;
	buf[22] = data_d.CDPFP2;
	buf[23] = data_d.CDPFP3;
	memcpy(&buf[24],data_d.gps,14);
	memcpy(&buf[38],data_d.time,6);
	buf[44] = 0xFF;
	buf[45] = 0xFF;
	buf[46] = 0xEE;
	memcpy(databuf+(i*47),buf,47);
 }
}
//  	char hexbuf2[1881];
//  	BytesToHexString(databuf, 940, hexbuf2);
//   hexbuf2[1880] = '\0';
//  	printf("%s\r\n",hexbuf2);
  return 1;
}


void sd_decode(uint8_t* data) 
{
	uint8_t buf[47];
	static uint8_t *databuf;
	struct _data_frame data_d;
	int i,len = 0;
	static int times = 0; //控制一次发送的帧数

	int ret;
  u8 err = 0;
	
	databuf = data;
  memset(databuf,0,sizeof(databuf));
  for(i = 0;i< 20;i++)
{
	memset(buf,NULL,sizeof(buf));
  len = (i+times) * sizeof(data_d);
	
	switch(file_option)
	{
		case 0:
		OSMutexPend(Semsd,0,&err); 
		f_lseek(log1,len);
	  ret = f_read(log1,&data_d,sizeof(data_d),&bw);
		if(ret != FR_OK) printf("read1_error\n");
    OSMutexPost(Semsd); 		
		break;
		
		case 1:
		OSMutexPend(Semsd,0,&err); 
		f_lseek(log,len);
	  ret = f_read(log,&data_d,sizeof(data_d),&bw);
		if(ret != FR_OK) printf("read0_error\n");
		OSMutexPost(Semsd); 		
	  break;
	}

	memcpy(&buf[0],"\x33\x34",2);
	memcpy(&buf[2],"\x2B\x00",2);
	memcpy(&buf[4],data_d.vid,4);
	buf[8] = data_d.sn;
	memcpy(&buf[9],"\x44\x41",2);
	
	buf[11] = data_d.speed;
	
	memcpy(&buf[12],data_d.mileage,3);
	memcpy(&buf[15],data_d.DOCT1,2);
	memcpy(&buf[17],data_d.CDPFT2,2);
	memcpy(&buf[19],data_d.CDPFT3,2);
		
	buf[21] = data_d.DCOP1;
	buf[22] = data_d.CDPFP2;
	buf[23] = data_d.CDPFP3;
	memcpy(&buf[24],data_d.gps,14);
	memcpy(&buf[38],data_d.time,6);
	buf[44] = 0xFF;
	buf[45] = 0xFF;
	buf[46] = 0xEE;
	memcpy(databuf+(i*47),buf,47);
}	
  if(times == 880) times = 0;
  else 
  times += 20;


// 	char hexbuf2[1024];
// 	BytesToHexString(databuf, 235, hexbuf2);
// 	printf("read  hexbuf is  = %s\r\n",hexbuf2);
}



void plus_one_second(char *utctime)
{
	char UTCYear[2];
	char UTCMonth[2];
	char UTCDay[2];
	char UTCHour[2];
	char UTCMinutes[2];
	char UTCSeconds[2];
  int year = 0;
	int month = 0;
	int day = 0;
	int sec = 0;
	int min = 0;
	int hour = 0;
  memcpy(UTCYear,utctime,2);
	memcpy(UTCMonth,utctime+2,2);
	memcpy(UTCDay,utctime+4,2);
	memcpy(UTCHour,utctime+6,2);
	memcpy(UTCMinutes,utctime+8,2);
	memcpy(UTCSeconds,utctime+10,2);
	
     year = atoi(UTCYear);  
     month = atoi(UTCMonth);  
     day = atoi(UTCDay);  
     sec = atoi(UTCSeconds)+1;  
     min = atoi(UTCMinutes);  
     hour = atoi(UTCHour);
     if(sec>= 60)
		 {
       sec -= 60;
       min++;
		 }
     if(min > 60)
		 {
       hour++;
			 min -= 60;
			}			 
     if (hour>23)  
        {  
        hour-=24;  
        day++;  
        switch (month)  
               {  
               case 1:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                    break;  
               case 2:  
                    if ((0==year%4 && 0!=year%100) || 0==year%400)  
                       {  
                       if (day>29)  
                          {  
                          day=1;  
                          month++;  
                          }  
                       }  
                    else  
                       {  
                       if (day>28)  
                          {  
                          day=1;  
                          month++;  
                          }  
                       }  
                   break;  
               case 3:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 4:  
                    if (day>30)  
                       {  
                       day=1;  
                       month++;  
                       }  
                    break;  
               case 5:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 6:  
                    if (day>30)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 7:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 8:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                    break;  
               case 9:  
                    if (day>30)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 10:  
                    if (day>31)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 11:  
                    if (day>30)  
                       {  
                       day=1;  
                       month++;  
                       }  
                   break;  
               case 12:  
                    if (day>31)  
                       {  
                       day=1;  
                       month=1;  
                       year++;  
                       }  
                    break;  
               default:break;  
               }  
        }			

        sprintf(utctime,"%02d",year);
				sprintf(utctime+2,"%02d",month);
		    sprintf(utctime+4,"%02d",day);
		    sprintf(utctime+6,"%02d",hour);
		    sprintf(utctime+8,"%02d",min);
		    sprintf(utctime+10,"%02d",sec);				
}

int findStr(char *str, char *substr) //比较函数，参数是字符串指针    
{    
    int  n;    
    char  *p, *r;    
    n = 0;    
    while (*str)  //    
    {    
        p = str;    
        r = substr;    
        while (*r)    
            if (*r == *p)//从str字符串的左侧第一字母比较，如果相同继续下一个字母    
            {    
                r++;    
                p++;    
            }    
            else    
            {    
                break;    
            }    
        if (*r == '\0')//比较到substr的结尾，相同则继续下一次    
            n++;    
        str++;    
    }    
    return n;    
}

uint8_t* make_frame_right(uint8_t *src)
{
	//将帧序号加1，提取时间加一秒，做成新的帧
	uint8_t *data_frame = NULL;
	int i;
	for(i=0;i<10;i++)
	{
		strcat((char*)data_frame,(char*)frame_time_plus(frame_sn_plus(src)));
		
	}
	return data_frame;
//	BytesToHexString(data_frame, 940, hexbuf);
 
}
uint8_t* frame_sn_plus(uint8_t *src)
{
	src[8] = src[8]+1;
	return src;
}
uint8_t* frame_time_plus(uint8_t *src)
{
	char time[12];
  char time_t[12];
	char hextime_t[12];
	char sh_time_t[6];
	char hexbuf[95];
	
	BytesToHexString(src, 47, hexbuf);
	memcpy(time,hexbuf+76,12);
	printf("time is %s",time);
  plus_one_second(time);
 	sh_time_convert(time,time_t,0);	
	time_to_hextime(time_t,hextime_t);
  HexStringToBytes(hextime_t,12,sh_time_t);
	
	memcpy(src+38,sh_time_t,6);
	//printf(src);
	return src;
}

int cal_rec_len(void)
{
	char *p1,*p2,*p3;
	char *outer_ptr=NULL;
	static int len_r = 0;
	
	if((p1=(char*)strstr((char*)Uart2_Buf,"+RECEIVE,0")),(p1!=NULL))
	{
  p2 = strtok_r(p1,":",&outer_ptr);
	p3 = strtok_r(p2,",",&outer_ptr);
	p3 = strtok_r(NULL,",",&outer_ptr);
	p3 = strtok_r(NULL,",",&outer_ptr);
  len_r = len_r+atoi(p3);
  printf("reclen is %d\n",len_r);
	CLR_Buf2();	
	}
	if(len_r == 420) { len_r = 0;return 1;}
	else return 0;

}

int wait_to_success(void)
{
	
	Time_send_wait = 1;
	while(Time_send_wait <= 4)
	{
	if(strstr((char*)Uart2_Buf,"0, SEND OK"))
	{
		Time_send_wait = 0;
		return 1;	
	}
  else if(strstr((char*)Uart2_Buf,"0, SEND FAIL"))
	{
		Time_send_wait = 0;
		printf("发送失败\n");
		return 0;	
	}
	else if(strstr((char*)Uart2_Buf,"0, SEND ERROR"))
	{
		Time_send_wait = 0;
		printf("发送错误\n");	
		return 0;
    	
	}
	}
	Time_send_wait = 0;
  return 2;
}
