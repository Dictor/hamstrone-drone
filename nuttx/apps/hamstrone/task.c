#include "include/task.h"

int tskTransmitValue(int argc, char *argv[]) {
    int period = atoi(argv[1]);
    if (period <= 0) period = 100000;
    HAMSTERTONGUE_Message* msg = HAMSTERTONGUE_NewMessage(HAMSTERTONGUE_MESSAGE_VERB_VALUE, 0, sizeof(HAMSTRONE_CONFIG_VALUE_TYPE));
    
    while(1) {
        for (int i = 0; i < HAMSTRONE_CONFIG_VALUE_SIZE; i++) {
            msg->Noun = i;
            HAMSTRONE_Serialize32(HAMSTRONE_ReadValueStore(i), msg->Payload, 0);
            HAMSTERTONGUE_WriteMessage(HAMSTRONE_GLOBAL_TELEMERTY_PORT, msg);
        }
        usleep(period);
    }
}

int tskUpdateValue(int argc, char *argv[]) {
    int period = atoi(argv[1]);
    if (period <= 0) period = 100;

    struct timespec startTs, currentTs;
    clock_gettime(CLOCK_REALTIME, &startTs);

    while(1) {
        clock_gettime(CLOCK_REALTIME, &currentTs);
        HAMSTRONE_WriteValueStore(0, (uint32_t)(currentTs.tv_sec - startTs.tv_sec));
        usleep(period);
    }
}