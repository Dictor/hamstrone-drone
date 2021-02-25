#ifndef HAMSTRONE_VALUE_STORE_H
#define HAMSTRONE_VALUE_STORE_H

#include "config.h"
#include <unistd.h>
#include <stdlib.h>

void HAMSTRONE_InitValueStore(int valueCount);
HAMSTRONE_CONFIG_VALUE_TYPE HAMSTRONE_ReadValueStore(int index);
void HAMSTRONE_WriteValueStore(int index, HAMSTRONE_CONFIG_VALUE_TYPE data);
HAMSTRONE_CONFIG_VALUE_TYPE* HAMSTRONE_GetValueStorePointer();

#endif