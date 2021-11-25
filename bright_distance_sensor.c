#include "include/bright_dist_sensor.h"

int initSO6203(uint8_t chanStart, uint8_t chanEnd)
{
    uint8_t errcnt;
    for (int c = chanStart; c <= chanEnd; c++)
    {
        if (TCA9548SetChannel(HAMSTRONE_GLOBAL_I2C_PORT, c) < 0)
            errcnt++;
        if (I2CWriteRegisterSingle(HAMSTRONE_GLOBAL_I2C_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_SO6203, HAMSTRONE_CONFIG_SO6203_EN, 0b00001011) < 0)
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

int readTFmini(uint8_t chanStart, uint8_t chanEnd, uint16_t *result)
{
    uint8_t errcnt;
    uint8_t data[7];
    for (int c = chanStart; c <= chanEnd; c++)
    {
        if (TCA9548SetChannel(HAMSTRONE_GLOBAL_I2C_PORT, c) < 0)
            errcnt++;
        I2CWriteSingle(HAMSTRONE_GLOBAL_I2C_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_TFmini, 0x01);
        I2CWriteSingle(HAMSTRONE_GLOBAL_I2C_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_TFmini, 0x02);
        I2CWriteSingle(HAMSTRONE_GLOBAL_I2C_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_TFmini, 0x07);
        usleep(1000);
        if (I2CRead(HAMSTRONE_GLOBAL_I2C_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_TFmini, 7, data) < 0)
            errcnt++;
        //  data[0]=isValid? [2]=distl [3]=disth [4]=strengthl [5]=strengthh [7]=rangetype
        result[c] = (data[3] << 8) | data[2];
    }
}