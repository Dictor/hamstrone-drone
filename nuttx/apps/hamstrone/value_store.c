#include "include/value_store.h"

HAMSTRONE_CONFIG_VALUE_TYPE* valueStore;

void HAMSTRONE_InitValueStore(int valueCount) {
    valueStore = malloc(sizeof(HAMSTRONE_CONFIG_VALUE_TYPE) * valueCount);
    for (int i = 0; i < valueCount; i++) {
        HAMSTRONE_WriteValueStore(i, 0);
    }
}

HAMSTRONE_CONFIG_VALUE_TYPE HAMSTRONE_ReadValueStore(int index) {
    return valueStore[index];
}

void HAMSTRONE_WriteValueStore(int index, HAMSTRONE_CONFIG_VALUE_TYPE data) {
    valueStore[index] = data;
}