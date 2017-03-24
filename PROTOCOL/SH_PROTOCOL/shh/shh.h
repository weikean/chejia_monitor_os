#ifndef _SHH_H_
#define _SHH_H_

#include <stdint.h>
#include <stdbool.h>

#define STX_RELEASE  0x3334
#define STX_DEBUG    0x3331

#define FRAME_RE    0x5245
#define FRAME_AR    0x4152
#define FRAME_DA    0x4441
#define FRAME_AD    0x4144
#define FRAME_HB    0x4842
#define FRAME_CM    0x434d
#define FRAME_AC    0x4143

#define SHH_PLAIN (0xFF)
#define SHH_ETX (0xEE)
#define SHH_DEFAULT_VID (0x11111111)
#define SHH_DEFAULT_LRC (0xFF)

#define HEADER_SIZE (9)

struct shh_reg_request
{
	uint16_t frame_type;
	uint8_t   vin[17];
	uint8_t  send_count;
};

struct shh_reg_response
{
	uint16_t frame_type;
	uint8_t   vin[17];
	uint8_t  time[6];
};

struct shh_data_request
{
	uint16_t frame_type;
	uint8_t  speed;
	uint32_t  mileage;
	uint16_t  DOCT1;
	uint16_t  CDPFT2;
	uint16_t  CDPFT3;
	uint8_t   DCOP1;
	uint8_t   CDPFP2;
	uint8_t   CDPFP3;
	uint8_t   gps[14];
	uint8_t  time[6];
};

struct shh_data_response
{
	uint16_t frame_type;
	uint8_t  success;
	uint8_t  time[6];
};

struct shh_heartbeat_request
{
	uint16_t frame_type;
	uint8_t  time[6];	
};

struct shh_cmd_response
{
	uint16_t frame_type;
	uint8_t  success;
	uint8_t  time[6];
};

int32_t shh_reset(void);
int32_t shh_set_stx(uint16_t stx);
int32_t shh_enable_cksum(bool enable);
uint32_t shh_get_vid(void);
int32_t  shh_set_vid(uint32_t vid);
int32_t shh_encode(uint8_t *buf, uint32_t buf_size, uint8_t *data, uint32_t data_size, uint8_t sn);
int32_t shh_decode(uint8_t *buf, uint32_t buf_size, uint8_t *data, uint32_t data_size, uint16_t *frame_type);

int32_t shh_reg_request_encode(uint8_t *buf, uint32_t buf_size, struct shh_reg_request *req);
int32_t shh_reg_response_decode(uint8_t *data, uint32_t data_size, struct shh_reg_response *res);

int32_t shh_data_request_encode(uint8_t *buf, uint32_t buf_size, struct shh_data_request *req);
int32_t shh_data_response_decode(uint8_t *data, uint32_t data_size, struct shh_data_response *res);

int32_t shh_heartbeat_request_encode(uint8_t *buf, uint32_t buf_size, struct shh_heartbeat_request *req);

int32_t shh_cmd_request_decode(uint8_t *data, uint32_t data_size, uint8_t *cmd, uint32_t cmd_size);
int32_t shh_cmd_response_encode(uint8_t *buf, uint32_t buf_size, struct shh_cmd_response *res);

#endif
