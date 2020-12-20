#include "include/hamster_tongue.h"

HAMSTERTONGUE_Message* HAMSTERTONGUE_NewMessage(uint8_t verb, uint8_t noun, uint8_t payloadLength) {
	HAMSTERTONGUE_Message* msg = malloc(sizeof(HAMSTERTONGUE_Message));
	msg->Verb = verb;
	msg->Noun = noun;
	msg->PayloadLength = payloadLength;
	msg->Payload = malloc(payloadLength);
}
uint16_t HAMSTERTONGUE_GetMessageLength(HAMSTERTONGUE_Message* msg) {
	return msg->PayloadLength + 5;
}

uint8_t* HAMSTERTONGUE_SerializeMessage(HAMSTERTONGUE_Message* msg) {
	uint8_t* res = malloc(HAMSTERTONGUE_GetMessageLength(msg));
	res[0] = HAMSTERTONGUE_MESSAGE_MARKER;
	res[1] = HAMSTERTONGUE_GetMessageLength(msg) - 2;
	res[2] = msg->Verb;
	res[3] = msg->Noun;
	memcpy(res + 4, msg->Payload, msg->PayloadLength);
	res[4 + msg->PayloadLength] = 0; //CRC
	return res;
}

ssize_t HAMSTERTONGUE_WriteMessage(int fd, HAMSTERTONGUE_Message* msg) {
	return pwrite(fd, HAMSTERTONGUE_SerializeMessage(msg), HAMSTERTONGUE_GetMessageLength(msg), 0);
}