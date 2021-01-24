#include "include/task.h"

int tskTransmitValue(int argc, char *argv[])
{
    int period = atoi(argv[1]);
    if (period <= 0)
        period = 100000;
    HAMSTERTONGUE_Message *msg = HAMSTERTONGUE_NewMessage(HAMSTERTONGUE_MESSAGE_VERB_VALUE, 0, sizeof(HAMSTRONE_CONFIG_VALUE_TYPE));

    while (1)
    {
        for (int i = 0; i < HAMSTRONE_CONFIG_VALUE_SIZE; i++)
        {
            msg->Noun = i;
            HAMSTRONE_Serialize32(HAMSTRONE_ReadValueStore(i), msg->Payload, 0);
            HAMSTERTONGUE_WriteMessage(HAMSTRONE_GLOBAL_TELEMETRY_PORT, msg);
        }
        usleep(period);
    }
}

int tskUpdateValue(int argc, char *argv[])
{
    int period = atoi(argv[1]);
    if (period <= 0)
        period = 200;

    struct timespec startTs, currentTs;
    clock_gettime(CLOCK_REALTIME, &startTs);

    uint8_t temph, templ;
    uint16_t temp;
    int ret1, ret2;
    while (1)
    {
        /* update runtime */
        clock_gettime(CLOCK_REALTIME, &currentTs);
        HAMSTRONE_WriteValueStore(0, (uint32_t)(currentTs.tv_sec - startTs.tv_sec));

        /* update itg3205 */
        ret1 = 0;
        ret2 = 0;
        ret1 = readI2CSingle(HAMSTRONE_GLOBAL_IMU_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_ITG3205, HAMSTRONE_CONFIG_ITG3205_TEMP_OUT_H, &temph);
        ret2 = readI2CSingle(HAMSTRONE_GLOBAL_IMU_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_ITG3205, HAMSTRONE_CONFIG_ITG3205_TEMP_OUT_L, &templ);
        if (ret1 < 0 || ret2 < 0)
        {
            HAMSTERTONGUE_WriteAndFreeMessage(
                HAMSTRONE_GLOBAL_TELEMETRY_PORT,
                HAMSTERTONGUE_NewFormatStringMessage(
                    HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                    HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_I2CREADFAIL,
                    24,
                    "fd=%d ret=%d,%d",
                    HAMSTRONE_GLOBAL_IMU_PORT, ret1, ret2));
        }
        temp = (temph << 8) + templ;
        HAMSTRONE_WriteValueStore(1, (uint32_t)temp);
        usleep(period);
    }
}

int readI2CSingle(int fd, uint16_t addr, uint8_t regaddr, uint8_t *buf)
{
    struct i2c_msg_s msg[2];
    struct i2c_transfer_s trans;
    uint8_t regaddrbuf[1] = {regaddr};

    msg[0].addr = addr;
    msg[0].flags = 0;
    msg[0].buffer = regaddrbuf;
    msg[0].length = 1;
    msg[0].frequency = 400000;

    msg[1].addr = addr;
    msg[1].flags = I2C_M_READ;
    msg[1].buffer = buf;
    msg[1].length = 1;
    msg[1].frequency = 400000;

    trans.msgv = (struct i2c_msg_s *)msg;
    trans.msgc = 2;

    return ioctl(fd, I2CIOC_TRANSFER, &trans);
}