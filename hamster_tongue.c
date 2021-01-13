#include "include/hamster_tongue.h"

sem_t* WriteSemaphore = NULL;

void HAMSTERTONGUE_SetWriteSemaphore(sem_t* sem) {
	WriteSemaphore = sem;
}

HAMSTERTONGUE_Message* HAMSTERTONGUE_NewMessage(uint8_t verb, uint8_t noun, uint8_t payloadLength) {
	HAMSTERTONGUE_Message* msg = malloc(sizeof(HAMSTERTONGUE_Message));
	msg->Verb = verb;
	msg->Noun = noun;
	msg->PayloadLength = payloadLength;
	msg->Payload = malloc(payloadLength);
	return msg;
}

HAMSTERTONGUE_Message* HAMSTERTONGUE_NewStringMessage(uint8_t verb, uint8_t noun, char* str) {
	int len = strlen(str);
	HAMSTERTONGUE_Message* msg = HAMSTERTONGUE_NewMessage(verb, noun, sizeof(char) * len);
	strcpy((char*)msg->Payload, str);
	return msg;
}

HAMSTERTONGUE_Message* HAMSTERTONGUE_NewFormatStringMessage(uint8_t verb, uint8_t noun, int bufSize, char* format, ...) {
	char* str = malloc(sizeof(char) * bufSize);
	va_list argptr;
    va_start(argptr, format);
    vsnprintf(str, (size_t)bufSize, format, argptr);
    va_end(argptr);
	return HAMSTERTONGUE_NewStringMessage(verb, noun, str);
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
	uint8_t* serialMsg = HAMSTERTONGUE_SerializeMessage(msg);
	if (WriteSemaphore != NULL) sem_wait(WriteSemaphore); 
	ssize_t res = pwrite(fd, serialMsg, HAMSTERTONGUE_GetMessageLength(msg), 0);
	if (WriteSemaphore != NULL) sem_post(WriteSemaphore);
	free(serialMsg);
	return res;
}

ssize_t HAMSTERTONGUE_WriteAndFreeMessage(int fd, HAMSTERTONGUE_Message* msg) {
	ssize_t res = HAMSTERTONGUE_WriteMessage(fd, msg);
	free(msg);
	return res;
}