#ifndef __MC8618_H
#define __MC8618_H

#include "sys.h"
#include <stdbool.h>

#define IP_0 "at+zipsetup=0,120.25.56.199,10003\r"

void mc8618_power_on(void);
void mc8618_power_off(void);
bool mc8618_conn(void);
void mc8618_close_conn(void);
void mc8618_send_data(const char *data,const int len);
void CLR_Buf2(void);
u8 Find(const char *a);
u8 GSM_send_cmd(u8 *cmd,u8 *ack,u8 wait_time);

#endif
