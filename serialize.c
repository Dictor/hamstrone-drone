#include "include/serialize.h"

void HAMSTRONE_Serialize32(uint32_t i, uint8_t* buf, int offset) {
	*(buf + offset) = i & 0xff;
	*(buf + offset + 1) = (i >> 8) & 0xff;
	*(buf + offset + 2) = (i >> 16) & 0xff;
	*(buf + offset + 3) = (i >> 24) & 0xff;
}