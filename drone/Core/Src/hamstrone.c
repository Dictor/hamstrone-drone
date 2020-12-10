#include "hamstrone.h"

HAL_StatusTypeDef HAMSTRONE_MessageTransmit(UART_HandleTypeDef* huart, HAMSTRONE_Message* msg, uint32_t timeout){
	int payloadLen = sizeof(msg->Payload) / sizeof(uint8_t);
	uint8_t* payload = malloc(sizeof(uint8_t) * (4 + payloadLen));
	payload[0] = HAMSTRONE_MESSAGE_MARKER;
	payload[1] = 4 + payloadLen;
	payload[2] = msg->Verb;
	payload[3] = msg->Noun;
	for (int i = 0; i < payloadLen; i++) {
		payload[4+i] = msg->Payload[i];
	}
	payload[(4 + payloadLen)-1] = 0; //CRC
	HAL_StatusTypeDef res = HAL_UART_Transmit(huart, payload, 4 + payloadLen, timeout);
	free(payload);
	return res;
}


