#include "include/task.h"

int tskTransmitValue(int argc, char * argv []) {
    int transmitPeriod = atoi(argv[0]);
    HAMSTERTONGUE_Message* msg = HAMSTERTONGUE_NewMessage(HAMSTERTONGUE_MESSAGE_VERB_VALUE, 0, sizeof(HAMSTRONE_CONFIG_VALUE_TYPE))
    
    while(1) {
        usleep(transmitPeriod);
    }
}