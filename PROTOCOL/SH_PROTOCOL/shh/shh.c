#include "shh.h"
#include <stdio.h>
#include <string.h>

static uint32_t s_vid = SHH_DEFAULT_VID;
static bool s_enable_cksum = false;
static uint8_t s_send_count = 0;
static uint16_t s_stx = STX_RELEASE;

static void uint16_to_be16_bytes(uint16_t x, uint8_t *p)
{
	p[0] = (x >> 8) & 0xFF;
	p[1] = (x) & 0xFF;
}

static void uint16_to_le16_bytes(uint16_t x, uint8_t *p)
{
	p[0] = (x) & 0xFF;
	p[1] = (x >> 8) & 0xFF;
}

static uint16_t be16_bytes_to_uint16(uint8_t *p)
{
	uint16_t rv;
	rv = (uint16_t)p[0] << 8  |
		 (uint16_t)p[1];
	return rv;
}

static uint16_t le16_bytes_to_uint16(uint8_t *p)
{
	uint16_t rv;
	rv = (uint16_t)p[0] |
		 (uint16_t)p[1] << 8;
	return rv;
}

// static void uint32_to_be32_bytes(uint32_t x, uint8_t *p)
// {
// 	p[0] = (x >> 24) & 0xFF;
// 	p[1] = (x >> 16) & 0xFF;
// 	p[2] = (x >> 8) & 0xFF;
// 	p[3] = (x) & 0xFF;
// }

static void uint32_to_le32_bytes(uint32_t x, uint8_t *p)
{
	p[0] = (x) & 0xFF;
	p[1] = (x >> 8) & 0xFF;
	p[2] = (x >> 16) & 0xFF;
	p[3] = (x >> 24) & 0xFF;
}

// static uint32_t be32_bytes_to_uint32(uint8_t *p)
// {
// 	uint32_t rv;
// 	rv = (uint32_t)p[0] << 24 |
// 		 (uint32_t)p[1] << 16 |
// 		 (uint32_t)p[2] << 8  |
// 		 (uint32_t)p[3];
// 	return rv;
// }

static uint32_t le32_bytes_to_uint32(uint8_t *p)
{
	uint32_t rv;
	rv = (uint32_t)p[0]       |
		 (uint32_t)p[1] << 8  |
		 (uint32_t)p[2] << 16 |
		 (uint32_t)p[3] << 24;
	return rv;
}

static void uint32_to_le24_bytes(uint32_t x, uint8_t *p)
{
	p[0] = (x) & 0xFF;
	p[1] = (x >> 8) & 0xFF;
	p[2] = (x >> 16) & 0xFF;
}

static uint8_t calc_lrc(uint8_t lrc, uint8_t *buf, uint32_t buf_size)
{
	int i;
	for (i=0; i<buf_size; i++) {
		lrc ^= buf[i];
	}
	return lrc;
}

int32_t shh_reset(void)
{
	s_vid = SHH_DEFAULT_VID;
	s_send_count = 0;
	s_enable_cksum = false;
	return 0;
}

int32_t shh_set_stx(uint16_t stx)
{
	s_stx = stx;
	return 0;
}

int32_t shh_enable_cksum(bool enable)
{
	s_enable_cksum = enable;
	return 0;
}

uint32_t shh_get_vid(void)
{
	return s_vid;
}

int32_t  shh_set_vid(uint32_t vid)
{
	s_vid = vid;
	return 0;
}

int32_t shh_encode(uint8_t *buf, uint32_t buf_size, uint8_t *data, uint32_t data_size, uint8_t sn)
{
	uint16_t buf_len;
	uint16_t frame_len;
	uint8_t lrc = 0;

	frame_len = 5 + data_size + 3;
	buf_len = 9 + data_size + 3;
	// stx
	uint16_to_be16_bytes(s_stx, &buf[0]);

	// frame len
	uint16_to_le16_bytes(frame_len, &buf[2]);

	// vid
	uint32_to_le32_bytes(s_vid, &buf[4]);

	// sn
	buf[8] = sn;

	// data
	memcpy(&buf[9], data, data_size);

	// encrypt type
	buf[buf_len - 3] = SHH_PLAIN;

	// cksum
	// from vid to encrypt type
	if (s_enable_cksum) {
		lrc = calc_lrc(lrc, &buf[4], 5 + data_size + 1);
	}
	else {
		lrc = SHH_DEFAULT_LRC;
	}
	buf[buf_len - 2] = lrc;

	// etx
	buf[buf_len - 1] = SHH_ETX;
	return buf_len;
}

int32_t shh_decode(uint8_t *buf, uint32_t buf_size, uint8_t *data, uint32_t data_size, uint16_t *frame_type)
{
	uint16_t stx;
	uint16_t frame_len;
	uint32_t vid;
	uint32_t data_len;

	if (buf_size < (9+3)) {
		return -1;
	}
	// stx
	stx = be16_bytes_to_uint16(&buf[0]);
	if (stx != s_stx) {
		return -2;
	}
	// frame len
	frame_len = le16_bytes_to_uint16(&buf[2]);

	// printf("frame_len=%d\n", (int)frame_len);

	// vid
	vid  = le32_bytes_to_uint32(&buf[4]);
	if (s_vid == SHH_DEFAULT_VID) {
		s_vid = vid;
	}

	// data
	data_len = frame_len - 5 - 3;

	// frame type
	*frame_type = be16_bytes_to_uint16(&buf[9]);

	memcpy(data, &buf[9], data_len);
	return data_len;
}

int32_t shh_reg_request_encode(uint8_t *buf, uint32_t buf_size, struct shh_reg_request *req)
{
	// frame type
	uint16_to_be16_bytes(req->frame_type, &buf[0]);

	// vin
	memcpy(&buf[2], req->vin, sizeof(req->vin));

	// send count
	buf[19] = req->send_count;

	return 20;
}

int32_t shh_reg_response_decode(uint8_t *data, uint32_t data_size, struct shh_reg_response *res)
{
	// frame type
	res->frame_type = be16_bytes_to_uint16(&data[0]);

	// vin
	memcpy(res->vin, &data[2], sizeof(res->vin));

	// time
	memcpy(res->time, &data[19], sizeof(res->time));
	return 25;
}

int32_t shh_data_request_encode(uint8_t *buf, uint32_t buf_size, struct shh_data_request *req)
{
	// frame type
	uint16_to_be16_bytes(req->frame_type, &buf[0]);

	// speed
	buf[2] = req->speed;

	// mileage
	uint32_to_le24_bytes(req->mileage, &buf[3]);

	// DOCT1
	uint16_to_le16_bytes(req->DOCT1, &buf[6]);

	// CDPFT2
	uint16_to_le16_bytes(req->CDPFT2, &buf[8]);

	// CDPFT3
	uint16_to_le16_bytes(req->CDPFT3, &buf[10]);

	// DCOP1
	buf[12] = req->DCOP1;

	// CDPFP2
	buf[13] = req->CDPFP2;

	// CDPFP3
	buf[14] = req->CDPFP3;

	// gps
	memcpy(&buf[15], req->gps, sizeof(req->gps));

	// time
	memcpy(&buf[29], req->time, sizeof(req->time));
	return 35;
}

int32_t shh_data_response_decode(uint8_t *data, uint32_t data_size, struct shh_data_response *res)
{
	// frame type
	res->frame_type = be16_bytes_to_uint16(&data[0]);

	// success
	res->success = data[2];

	// time
	memcpy(res->time, &data[3], sizeof(res->time));
	return 9;
}

int32_t shh_heartbeat_request_encode(uint8_t *buf, uint32_t buf_size, struct shh_heartbeat_request *req)
{
	// frame type
	uint16_to_be16_bytes(req->frame_type, &buf[0]);

	// time
	memcpy(&buf[2], req->time, sizeof(req->time));

	return 8;
}

int32_t shh_cmd_request_decode(uint8_t *data, uint32_t data_size, uint8_t *cmd, uint32_t cmd_size)
{
	uint32_t cmd_len;
	// frame type
	// time
 	// custom command
 	cmd_len = data_size - 8;
 	memcpy(cmd, &data[8], cmd_len);
	return cmd_len;
}

int32_t shh_cmd_response_encode(uint8_t *buf, uint32_t buf_size, struct shh_cmd_response *res)
{
	// frame type
	uint16_to_be16_bytes(res->frame_type, &buf[0]);

	// success
	buf[2] = res->success;

	// time
	memcpy(&buf[3], res->time, sizeof(res->time));
	return 9;
}
