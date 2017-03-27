#include "gprs_sh.h"
#include "shh.h"
#include "hexstring.h"
/**********************************************************************************
 * 文件名  ：gprs_sh.c
 * 描述    ：上海机动车检测通信协议的监控程序实现方法      
 * 实验平台： Sim808
 * 硬件连接： TXD(PA2)  -> 外部串口RXD     
 *           RXD(PA3) -> 外部串口TXD      
 *           GND	   -> 外部串口GND 
**********************************************************************************/

uint32_t vid;

void conn_to_sh_platform(uint8_t data_signal)
{
	if(mc8618_conn)
	{
		if (0 == vid)
		{
			reg_server();
		}
		else if (Time_heart > 180)
		{
			send_heartbeat();
			check_cmd_reply(10);
		}
		else if (1 == data_signal)
		{
			send_data_frame();
			check_data_reply(3);
		}
		else 
			check_platform_reply(3);
	}
}

/*************************************
* 向服务器注册的方法 
  发送10次不成功，休息15分钟再来注册
*
*************************************/

void reg_server(void)
{
	int i = 0;
	static u8 reg_conn_counter = 0;

	if(reg_conn_counter <= REG_TIMES || Time_sh_reg >= REG_WAIT_MIAO)
	{
		if(Time_sh_reg >= REG_WAIT_MIAO)
		{
			Time_sh_reg = 0;
			reg_conn_counter = 0;
		}
		if(reg_conn_counter >= REG_TIMES)
		{
			Time_sh_reg = 1;
		}
		reg_conn_counter++; 
		
		mc8618_send_data(send_reg_frame(), REG_FRAME_LEN);
	}
}

/**********************************
*根据协议生成注册帧
**********************************/
uint8_t* make_reg_frame(void)
{
// 	uint8_t expect_result[] = "\x33\x34\x1c\x00\x11\x11\x11\x11\x00\x52\x45\x41\x42\x43\x44\x45\x46\x47\x48\x49\x50\x51\x52\x53\x54\x55\x56\x57\x00\xff\xff\xee";
// 	uint32_t expect_len = 32;

	struct shh_reg_request reg_req;
	uint8_t buf_req[128];
	uint8_t buf[128];
	int32_t req_len;
  char sh_id[17];
	uint8_t *p = NULL;
	
	shh_reset();
	shh_set_stx(STX_RELEASE);
	shh_enable_cksum(false);

	reg_req.frame_type = FRAME_RE;
	
	if(strlen(m_info.id) != ID_LEN)
	get_device_id();

	strcpy(sh_id,m_info.id);
	memcpy(sh_id+strlen(m_info.id),"FFFFFFF",17-strlen(m_info.id));
	memcpy(reg_req.vin, sh_id, TEST_VIN_LEN);
	reg_req.send_count = 0;
	
	req_len = shh_reg_request_encode(buf_req, sizeof(buf_req), &reg_req);
	shh_encode(buf, sizeof(buf), buf_req, req_len, 0);
	printf("注册id=%s\n",m_info.id);
	p = buf;

	return p;
}

void reg_frame_response(void)
{
// 	uint8_t sample[] = "\x33\x34\x21\x00\x36\x91\x01\x00\x00\x41\x52\x41\x42\x43\x44\x45\x46\x47\x48\x49\x50\x51\x52\x53\x54\x55\x56\x57\x09\x08\x07\x06\x05\x10\xff\xff\xee";
 	uint32_t sample_len = 37;

  uint8_t reg_response[37];
	struct shh_reg_response reg_res;
	uint8_t buf_res[76];
	int32_t res_len;
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


/*************************************
* 每三分钟发一次心跳包，等待10秒接收服务器的指令帧，收到指令帧后
  发送指令应答帧，最后关闭连接，10秒内未收到指令帧，则直接关闭连接
*
*************************************/
void heartbeat(void)
{
	mc8618_send_data(send_heartbeat(), HEART_FRAME_LEN);
}

 /***************************
 *心跳帧格式
 *
 ****************************/
uint8_t *make_heart_frame(void)
{
	struct shh_heartbeat_request hb_req;
	uint8_t buf_req[34];
	uint8_t buf[34];
	int32_t req_len;
	char sh_time_t[6] = "";
	char hextime_t[12] = "";
	uint8_t *p = NULL;
//	uint8_t nib;
 	p = buf;
		
	shh_reset();
	shh_set_stx(STX_RELEASE);
	shh_enable_cksum(false);
	shh_set_vid(vid);
	hb_req.frame_type = FRAME_HB;
	
  printf("%s\n", gpsx.utc);
	memcpy(hb_req.time, sh_time_t, TEST_TIME_LEN);
	req_len = shh_heartbeat_request_encode(buf_req, sizeof(buf_req), &hb_req);
	shh_encode(buf, sizeof(buf), buf_req, req_len, 0);
	return p;
}


void data_manage(void)
{
	//从存储中取数据
	mc8618_send_data(send_heartbeat(), HEART_FRAME_LEN);  
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
