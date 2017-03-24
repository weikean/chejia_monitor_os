#ifndef __GPRS_HENAN_H
#define __GPRS_HENAN_H
#include "stm32f10x.h"

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
extern u8 data_ok;
extern int file_option;
extern int file_finish;
extern u8 sending;
extern int recieving;
uint8_t* send_reg_frame(void);
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
