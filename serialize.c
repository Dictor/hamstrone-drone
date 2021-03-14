#include "include/serialize.h"

void HAMSTRONE_Serialize32(uint32_t i, uint8_t* buf, int offset) {
	*(buf + offset) = i & 0xff;
	*(buf + offset + 1) = (i >> 8) & 0xff;
	*(buf + offset + 2) = (i >> 16) & 0xff;
	*(buf + offset + 3) = (i >> 24) & 0xff;
}

void HAMSTRONE_Serialize32Array(uint32_t *arr, uint8_t *buf, int arrlen, int offset) {
	uint8_t *rbuf = buf + offset;
	for (int i = 0; i < arrlen; i++) {
		HAMSTRONE_Serialize32(arr[i], rbuf, sizeof(uint32_t) * i);
	}
}