#include "include/bright_dist_sensor.h"

int initSO6203(uint8_t chanStart, uint8_t chanEnd)
{
    uint8_t errcnt;
    for (int c = chanStart; c <= chanEnd; c++)
    {
        if (TCA9548SetChannel(HAMSTRONE_GLOBAL_I2C_PORT, c) < 0)
            errcnt++;
        if (I2CWriteSingle(HAMSTRONE_GLOBAL_I2C_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_SO6203, HAMSTRONE_CONFIG_SO6203_EN, 0b00001011) < 0)
            errcnt++;
    }
    if (errcnt > 0)
        return ERROR_READ_FAIL;
    return 0;
}

int readSO6203(uint8_t chanStart, uint8_t chanEnd, uint16_t *result)
{
    uint8_t valueh, valuel, errcnt;
    for (int c = chanStart; c <= chanEnd; c++)
    {

        if (TCA9548SetChannel(HAMSTRONE_GLOBAL_I2C_PORT, c) < 0)
            errcnt++;
        if (I2CReadSingle(HAMSTRONE_GLOBAL_I2C_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_SO6203, HAMSTRONE_CONFIG_SO6203_ADCW_H, &valueh) < 0)
            errcnt++;
        if (I2CReadSingle(HAMSTRONE_GLOBAL_I2C_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_SO6203, HAMSTRONE_CONFIG_SO6203_ADCW_H + 1, &valuel) < 0)
            errcnt++;
        result[c] = (valueh << 8) | valuel;
    }
    if (errcnt > 0)
        return ERROR_READ_FAIL;
    return 0;
}