#ifndef __GPRS_SH_H
#define __GPRS_SH_H

#include "stm32f10x.h"
#include "gprs_jiankong.h"
#include "timer.h"


//#define TEST_VIN "\x41\x42\x43\x44\x45\x46\x47\x48\x49\x50\x51\x52\x53\x54\x55\x56\x57"
#define TEST_VIN_LEN (17)
#define TEST_TIME "\x09\x08\x07\x06\x05\x10"
#define TEST_TIME_LEN (6)
#define TEST_VID (0x00019136)
#define TEST_GPS "\x41\x1f\x12\x43\x56\x4e\x79\x34\x52\x67\x45\x22\x99\x00"
#define TEST_GPS_LEN (14)
#define TEST_CMD "\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55"
#define TEST_CMD_LEN (17)
#define REG_FRAME_LEN (32)
#define HEART_FRAME_LEN (20)
#define DATA_FRAME_LEN (47)

#define REG_TIMES (10)
#define REG_WAIT_MIAO (900)

static inline void uint32_to_le32_bytes(uint32_t x, uint8_t *p)
{
	p[0] = (x) & 0xFF;
	p[1] = (x >> 8) & 0xFF;
	p[2] = (x >> 16) & 0xFF;
	p[3] = (x >> 24) & 0xFF;
}

struct _data_frame
{
	uint16_t frame_mark;
	uint16_t frame_len;
	uint8_t  vid[4];
	uint8_t  sn;
	uint16_t frame_cmd;
	uint8_t   speed;
	uint8_t  mileage[3];
	uint8_t  DOCT1[2];
	uint8_t  CDPFT2[2];
	uint8_t  CDPFT3[2];
	uint8_t   DCOP1;
	uint8_t   CDPFP2;
	uint8_t   CDPFP3;
	uint8_t   gps[14];
	uint8_t  time[6];
	uint8_t  pwd_type;
	uint8_t  crc;
	uint8_t  frame_tail;
};


typedef struct MONITOR_UPLOAD_INFO
{
	monitor_info m_info;
	
	
}upload_info;


void conn_to_sh_platform(uint8_t data_signal);
uint8_t* make_reg_frame(void);
void reg_server(void);
void reg_response(void);
uint8_t* send_heartbeat(void);
void sh_task(void);
void sh_conn(void);
void sh_close(void);
void heartbeat(void);
uint8_t* send_data_frame(void);
int time_to_hextime(char* time,char* hextime);
int sh_time_convert(char * utctime,char *sh_time,int way);
void data_manage(void);
void sd_encode(void);
void sd_decode(uint8_t* data);
void plus_one_second(char *utctime);
int findStr(char *str, char *substr);
uint8_t* make_frame_right(uint8_t *src);
uint8_t* frame_sn_plus(uint8_t *src);
uint8_t* frame_time_plus(uint8_t *src);
int cal_rec_len(void);
void all_data_offline(void);
void left_data_offline(int flag);
int offline_decode(uint8_t *data_frame_off);
int wait_to_success(void);
void single_data_offline(int flag);
#endif
