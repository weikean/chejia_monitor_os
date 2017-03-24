#ifndef __MC8618_H
#define __MC8618_H

#include "sys.h"
#include <stdbool.h>

bool mc8618_signal_lost(void);
bool mc8618_ppp_open(void);
bool mc8618_ppp_close(void);
bool mc8618_ppp_connected(void);
bool mc8618_ppp_unconnected(void);
bool mc8618_ip_setup(void);
bool mc8618_ip_connected(void);
bool mc8618_ip_unconnected(void);
bool mc8618_ip_close(void);
void mc8618_power_on(void);
void mc8618_power_off(void);
void mc8618_conn(void);
void mc8618_close_conn(void);

void CLR_Buf2(void);
u8 Find(char *a);
u8 GSM_send_cmd(u8 *cmd,u8 *ack,u8 wait_time);
#endif
