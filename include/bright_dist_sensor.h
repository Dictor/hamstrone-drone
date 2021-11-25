#ifndef HAMSTRONE_BRDTSENSOR_H
#define HAMSTRONE_BRDTSENSOR_H

#include "config.h"
#include "sensor.h"

#define ERROR_READ_FAIL -1

int readSO6203(uint8_t chanStart, uint8_t chanEnd, uint16_t *result);
int initSO6203(uint8_t chanStart, uint8_t chanEnd);

#endif