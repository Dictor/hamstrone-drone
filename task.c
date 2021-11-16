#include "include/task.h"
#define MAX_SIZE 2

double kP[MAX_SIZE] = {0.408, 0.410};
double kI[MAX_SIZE] = {1.02, 1.02};
double kD[MAX_SIZE] = {0.0408, 0.0408};

void pidControl(double AngX, double AngY, double *pidAssemble)
{
    double degree[MAX_SIZE] = {
        0.0,
    };
    degree[0] = AngX;
    degree[1] = AngY;

    static double prevInput[MAX_SIZE] = {
        0.0,
    };
    static double controlI[MAX_SIZE] = {
        0.0,
    };
    double controlP[MAX_SIZE], controlD[MAX_SIZE], dInput[MAX_SIZE], error[MAX_SIZE], desired[MAX_SIZE] = {
                                                                                          10.0,
                                                                                      };
    double time = 0.01;
    int i;

    for (i = 0; i < 2; i++)
    {
        error[i] = desired[i] - degree[i];
        dInput[i] = degree[i] - prevInput[i];
        prevInput[i] = degree[i];

        controlP[i] = kP[i] * error[i];
        controlI[i] = kI[i] * error[i] * time;
        controlD[i] = -kD[i] * dInput[i] / time;

        pidAssemble[i] = controlP[i] + controlI[i] + controlD[i];
    }
}

int tskTransmitValue(int argc, char *argv[])
{
    int period = atoi(argv[1]);
    if (period <= 0)
        period = 500000; //500ms
    HAMSTERTONGUE_Message *msg = HAMSTERTONGUE_NewMessage(HAMSTERTONGUE_MESSAGE_VERB_VALUE, 0, sizeof(HAMSTRONE_CONFIG_VALUE_TYPE) * HAMSTRONE_CONFIG_VALUE_SIZE);

    while (1)
    {
        HAMSTRONE_Serialize32Array(HAMSTRONE_GetValueStorePointer(), msg->Payload, HAMSTRONE_CONFIG_VALUE_SIZE, 0);
        HAMSTERTONGUE_WriteMessage(HAMSTRONE_GLOBAL_TELEMETRY_PORT, msg);
        usleep(period);
    }
}

int tskUpdateValue(int argc, char *argv[])
{
    int period = atoi(argv[1]);
    if (period <= 0)
        period = 1000; //2ms

    struct timespec startTs, currentTs, taskendTs;
    clock_gettime(CLOCK_MONOTONIC, &startTs);

#define SO6203_CHAN_START 0
#define SO6203_CHAN_END 0
#define SO6203_COUNT SO6203_CHAN_END - SO6203_CHAN_START + 1
/* count of register, one count means set of high and low byte register (2 bytes)   */
#define SO6203_REGISTER_CNT 1
#define MPU6050_REGISTER_CNT 6
#define SO6203_VALUE_CNT SO6203_REGISTER_CNT *SO6203_COUNT
#define VALUE_COUNT SO6203_VALUE_CNT + MPU6050_REGISTER_CNT
#define MPU6050_SPI_MODE SPIDEV_MODE3

    const uint8_t devAddr[SO6203_VALUE_CNT] = {
        HAMSTRONE_CONFIG_I2C_ADDRESS_SO6203,
    };
    const uint8_t regAddr[VALUE_COUNT] = {
        HAMSTRONE_CONFIG_SO6203_ADCW_H,
        MPUREG_ACCEL_XOUT_H,
        MPUREG_ACCEL_YOUT_H,
        MPUREG_ACCEL_ZOUT_H,
        MPUREG_GYRO_XOUT_H,
        MPUREG_GYRO_YOUT_H,
        MPUREG_GYRO_ZOUT_H};
    uint8_t valuel, valueh;
    uint16_t value[VALUE_COUNT];
    double accelX, accelY, accelZ, gyroX, gyroY, gyroZ;
    double accelXsq, accelYsq, accelZsq, accelAngX, accelAngY;
    double gyroAngX, gyroAngY, gyroAngZ;
    double filterAngX, filterAngY;
    double pidAssemble[MAX_SIZE];

    int errcnt;
    /* initialize mpu6050 */
    uint8_t whoami;
    SPIReadSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU6050_SPI_MODE, MPUREG_WHOAMI | READ_FLAG, &whoami);
    HAMSTERTONGUE_Debugf("mpu whoami: %d", whoami);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU6050_SPI_MODE, MPUREG_PWR_MGMT_1, BIT_H_RESET);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU6050_SPI_MODE, MPUREG_PWR_MGMT_1, 0x01);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU6050_SPI_MODE, MPUREG_PWR_MGMT_2, 0x00);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU6050_SPI_MODE, MPUREG_ACCEL_CONFIG, BITS_FS_2G);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU6050_SPI_MODE, MPUREG_GYRO_CONFIG, BITS_FS_250DPS);

    /* initialize SO6203 */
    /*
    for (int c = SO6203_CHAN_START; c <= SO6203_CHAN_END; c++)
    {
        TCA9548SetChannel(HAMSTRONE_GLOBAL_I2C_PORT, c);
        if (I2CWriteSingle(HAMSTRONE_GLOBAL_I2C_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_SO6203, HAMSTRONE_CONFIG_SO6203_EN, 0b00001011) < 0)
        {
            HAMSTERTONGUE_WriteAndFreeMessage(
                HAMSTRONE_GLOBAL_TELEMETRY_PORT,
                HAMSTERTONGUE_NewFormatStringMessage(
                    HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                    HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_I2CREADFAIL,
                    24,
                    "fd=%d chan=%d so6203 en",
                    HAMSTRONE_GLOBAL_I2C_PORT, c));
        }
    }
    */

    while (1)
    {
        /* update runtime */
        clock_gettime(CLOCK_MONOTONIC, &currentTs);
        HAMSTRONE_WriteValueStore(0, (uint32_t)(currentTs.tv_sec - startTs.tv_sec));

        /* update so6203 sensor value */
        /*
        for (int c = SO6203_CHAN_START; c <= SO6203_CHAN_END; c++)
        {
            for (int i = 0; i < SO6203_REGISTER_CNT; i++)
            {
                errcnt = 0;
                if (TCA9548SetChannel(HAMSTRONE_GLOBAL_I2C_PORT, c) < 0)
                    errcnt++;
                if (I2CReadSingle(HAMSTRONE_GLOBAL_I2C_PORT, devAddr[c + i], regAddr[c + i], &valueh) < 0)
                    errcnt++;
                if (I2CReadSingle(HAMSTRONE_GLOBAL_I2C_PORT, devAddr[c + i], regAddr[c + i] + 1, &valuel) < 0)
                    errcnt++;
                if (errcnt > 0)
                {
                    HAMSTERTONGUE_WriteAndFreeMessage(
                        HAMSTRONE_GLOBAL_TELEMETRY_PORT,
                        HAMSTERTONGUE_NewFormatStringMessage(
                            HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                            HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_I2CREADFAIL,
                            32,
                            "fd=%d dev=%d errcnt=%d so6203",
                            HAMSTRONE_GLOBAL_I2C_PORT, devAddr[i], errcnt));
                    continue;
                }
                value[c + i] = (valueh << 8) | valuel;
            }
        }
        */

        for (int i = SO6203_VALUE_CNT; i < SO6203_VALUE_CNT + MPU6050_REGISTER_CNT; i++)
        {
            errcnt = 0;
            if (SPIReadSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU6050_SPI_MODE, regAddr[i] | READ_FLAG, &valueh) < 0)
                errcnt++;
            if (SPIReadSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU6050_SPI_MODE, (regAddr[i] + 1) | READ_FLAG, &valuel) < 0)
                errcnt++;
            if (errcnt > 0)
            {
                HAMSTERTONGUE_WriteAndFreeMessage(
                    HAMSTRONE_GLOBAL_TELEMETRY_PORT,
                    HAMSTERTONGUE_NewFormatStringMessage(
                        HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                        HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_I2CREADFAIL,
                        32,
                        "fd=%d errcnt=%d mpu",
                        HAMSTRONE_GLOBAL_SPI_PORT, errcnt));
                continue;
            }
            value[i] = (valueh << 8) | valuel;
        }

        /* calculate gyro and accel angle*/

        accelX = (int16_t)(~value[SO6203_VALUE_CNT] + 1) / HAMSTRONE_CONFIG_MPU6050_ACCEL_COEFFICIENT;
        accelY = (int16_t)(~value[SO6203_VALUE_CNT + 1] + 1) / HAMSTRONE_CONFIG_MPU6050_ACCEL_COEFFICIENT;
        accelZ = (int16_t)(~value[SO6203_VALUE_CNT + 2] + 1) / HAMSTRONE_CONFIG_MPU6050_ACCEL_COEFFICIENT;
        gyroX = (int16_t)(~value[SO6203_VALUE_CNT + 3] + 1) / HAMSTRONE_CONFIG_MPU6050_GYRO_COEFFICIENT;
        gyroY = (int16_t)(~value[SO6203_VALUE_CNT + 4] + 1) / HAMSTRONE_CONFIG_MPU6050_GYRO_COEFFICIENT;
        gyroZ = (int16_t)(~value[SO6203_VALUE_CNT + 5] + 1) / HAMSTRONE_CONFIG_MPU6050_GYRO_COEFFICIENT;

        accelXsq = pow(accelX, 2);
        accelYsq = pow(accelY, 2);
        accelZsq = pow(accelZ, 2);
        accelAngX = atan(accelY / sqrt(accelXsq + accelZsq)) * HAMSTRONE_CONFIG_RADIAN_TO_ANGLE;
        accelAngY = atan(-1 * accelX / sqrt(accelYsq + accelZsq)) * HAMSTRONE_CONFIG_RADIAN_TO_ANGLE;
        gyroAngX += gyroX * HAMSTRONE_CONFIG_MPU6050_GYRO_TIMEDELTA;
        gyroAngY += gyroY * HAMSTRONE_CONFIG_MPU6050_GYRO_TIMEDELTA;
        gyroAngZ += gyroZ * HAMSTRONE_CONFIG_MPU6050_GYRO_TIMEDELTA;
        filterAngX = accelAngX * HAMSTRONE_CONFIG_COMPLEMENTARY_FILTER_COEFFICIENT + (1 - HAMSTRONE_CONFIG_COMPLEMENTARY_FILTER_COEFFICIENT) * gyroAngX;
        filterAngY = accelAngY * HAMSTRONE_CONFIG_COMPLEMENTARY_FILTER_COEFFICIENT + (1 - HAMSTRONE_CONFIG_COMPLEMENTARY_FILTER_COEFFICIENT) * gyroAngY;

        if (filterAngX >= 10000)
            filterAngX = 0;
        if (filterAngY >= 10000)
            filterAngY = 0;

        /* process pid control*/
        pidControl(filterAngX, filterAngY, pidAssemble);

        HAMSTRONE_WriteValueStore(2, (uint32_t)(filterAngX * 100 + 18000));
        HAMSTRONE_WriteValueStore(3, (uint32_t)(filterAngY * 100 + 18000));
        HAMSTRONE_WriteValueStore(4, (uint32_t)(gyroAngZ * 100 + 18000));

        HAMSTRONE_WriteValueStore(6, (uint32_t)(10 * (pidAssemble[0] + pidAssemble[1]) + 200));
        HAMSTRONE_WriteValueStore(7, (uint32_t)(10 * (pidAssemble[0] - pidAssemble[1]) + 200));
        HAMSTRONE_WriteValueStore(8, (uint32_t)(10 * (-pidAssemble[0] + pidAssemble[1]) + 200));
        HAMSTRONE_WriteValueStore(9, (uint32_t)(10 * (-pidAssemble[0] - pidAssemble[1]) + 200));

        HAMSTRONE_WriteValueStore(10, (uint32_t)(value[0]));

        usleep(period);
        clock_gettime(CLOCK_MONOTONIC, &taskendTs);
        // PROPERY TICK RESOULUTION IS SMALLER THAN 1000USEC
        HAMSTRONE_WriteValueStore(1, (uint32_t)((taskendTs.tv_nsec - currentTs.tv_nsec) / 1000000));
    }
}

int tskParsingGPS(int argc, char *argv[])
{
#define MSG_BUF_SIZE 33
    int assembleCnt = 0, i = 0, bufLen = 0, assembleLen = 0, Len = 0;
    char Assemble_Data[200] = {
        0,
    };
    char buf[MSG_BUF_SIZE];
    while (1)
    {
        memset(buf, 0x00, 33);
        read(HAMSTRONE_GLOBAL_SERIAL_PORT, buf, 32);
        bufLen = strlen(buf);
        assembleLen = strlen(Assemble_Data);
        for (i = 0; i < bufLen; i++)
        {
            Assemble_Data[assembleLen] = buf[i];
            assembleLen++;
        }
        Len = strlen(Assemble_Data);
        assembleCnt = Checking(Assemble_Data, Len, assembleCnt);
        HAMSTRONE_WriteValueStore(9, (uint32_t)assembleCnt);
        usleep(200000);
    }
    return 0;
}

int Distance(int argc, char *argv[])
{
#define MSG_BUF_SIZE_DISTANCE 64
    uint8_t assemble[500] = {
        0,
    };
    uint8_t buf[MSG_BUF_SIZE_DISTANCE];
    int dataStart, dataEnd, distanceLow, distanceHigh, dataSplit, dataLen, first, second, temp, exsist, i, j, k, num;
    while (1)
    {
        memset(buf, 0x00, 33);
        read(HAMSTRONE_GLOBAL_SERIAL_PORT, buf, 32);
        dataSplit = 0;
        exsist = 0;
        j = 0;
        k = 1;
        for (i = first; i < second; i++)
        {
            //buf[j] = data[i];
            j++;
        }
        for (i = 0; i < j; i++)
            assemble[i] = buf[i];
        while (1)
        {
            if (assemble[i] == '\0')
            {
                dataLen = i;
                break;
            }
            else
                i++;
        }
        while (1)
        {
            for (i = 0; i < dataLen; i++)
            {
                exsist = 1;
                if (assemble[i] == 0x59)
                {
                    if (assemble[i + 1] == 0x59)
                    {
                        dataStart = i;
                        if (dataSplit == 1)
                        {
                            dataEnd = i - 1;
                            break;
                        }
                        else
                        {
                            dataSplit = 1;
                            distanceLow = i + 2;
                            distanceHigh = i + 3;
                        }
                    }
                    else
                        continue;
                }
            }
            dataSplit = 0;
            j = 0;
            k = 1;
            while (1)
            {
                if (assemble[i] == '\0')
                {
                    dataLen = i;
                    break;
                }
                else
                    i++;
            }
            if (exsist == 0)
            {
                for (i = 0; i < dataLen; i++)
                    assemble[i] = '\0';
                break;
            }
            if (dataLen < 15 || assemble[0] == '\0')
                break;
            for (i = 0; i <= dataLen; i++)
            {
                if (i < dataLen - dataEnd)
                {
                    assemble[i] = assemble[dataEnd + k];
                    k++;
                }
                else
                    assemble[i] = '\0';
            }
        }
        usleep(200000);
    }
    return 0;
}