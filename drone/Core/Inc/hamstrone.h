#ifndef HAMSTRONE_H
#define HAMSTRONE_H

#include "stm32l4xx_hal.h"

#define HAMSTRONE_MESSAGE_MARKER 0xFF

typedef struct {
    uint8_t Verb;
    uint8_t Noun;
    uint8_t* Payload;
    uint8_t PayloadLength;
} HAMSTRONE_Message;

HAL_StatusTypeDef HAMSTRONE_MessageTransmit(UART_HandleTypeDef* huart, HAMSTRONE_Message* msg, uint32_t timeout);

#endif
