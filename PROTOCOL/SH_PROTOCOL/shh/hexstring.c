#include "hexstring.h"


/* utility function to convert hex character representation to their nibble (4 bit) values */
static unsigned char
nibbleFromChar(char c)
{
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 255;
}

int HexStringToBytes(char *inhex, int size, char *out)
{
	int len;
	int i;
	unsigned char *p;
	len = size / 2;
	p = (unsigned char *)inhex;
	for (i=0; i<len; i++) {
		out[i] = (nibbleFromChar(*p) << 4) | nibbleFromChar(*(p+1));
		p += 2;
	}
	out[len] = 0;
	return len;
}

int HexStringToBytes_still(char *inhex, int size, char *out)
{
	int len;
	int i;
	unsigned char *p;
	len = size / 2;
	p = (unsigned char *)inhex;
	for (i=len-1; i>=0; i--) {
		out[i] = (nibbleFromChar(*p) << 4) | nibbleFromChar(*(p+1));
		p += 2;
	}
	out[len] = 0;
	return len;
}

static const char byteMap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
static const int byteMapLen = sizeof(byteMap);
/* Utility function to convert nibbles (4 bit values) into a hex character representation */
static char
nibbleToChar(unsigned char nibble)
{
	if(nibble < byteMapLen) return byteMap[nibble];
	return '*';
}

int BytesToHexString(unsigned char *bytes, int buflen, char *outhex)
{
	int i;
	int j;
	for (i=0; i<buflen; i++) {
		j = i * 2;
		outhex[j] = nibbleToChar(bytes[i] >> 4);
		outhex[j+1] = nibbleToChar(bytes[i] & 0x0f);
	}
	outhex[buflen * 2] = '\0';
	return buflen * 2;
}
