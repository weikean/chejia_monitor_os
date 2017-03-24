#ifndef _HEXSTRING_H_
#define _HEXSTRING_H_
#ifdef __cplusplus
extern "C" {
#endif

int HexStringToBytes(char *inhex, int size, char *out);
int HexStringToBytes_still(char *inhex, int size, char *out);
int BytesToHexString(unsigned char *bytes, int buflen, char *outhex);


#ifdef __cplusplus
}
#endif
#endif
