#include "include/task.h"
#define MAX_SIZE 2

double kP[MAX_SIZE] = { 0.408,0.408 };
double kI[MAX_SIZE] = { 1.02,1.02 };
double kD[MAX_SIZE] = { 0.0408,0.0408 };

void pidControl(double AngX, double AngY, double * pidAssemble)
{
    double degree[MAX_SIZE] = { 0.0, };
    degree[0] = AngX;
    degree[1] = AngY;
 
    static double prevInput[MAX_SIZE] = { 0.0, };
    static double controlI[MAX_SIZE] = { 0.0, };
    double controlP[MAX_SIZE], controlD[MAX_SIZE], dInput[MAX_SIZE], error[MAX_SIZE], desired[MAX_SIZE] = { 10.0, };
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

#define VALUE_CNT 7
    uint8_t regAddr[VALUE_CNT] = {
        HAMSTRONE_CONFIG_MPU6050_ACCEL_XOUT_H,
        HAMSTRONE_CONFIG_MPU6050_ACCEL_YOUT_H,
        HAMSTRONE_CONFIG_MPU6050_ACCEL_ZOUT_H,
        HAMSTRONE_CONFIG_MPU6050_GYRO_XOUT_H,
        HAMSTRONE_CONFIG_MPU6050_GYRO_YOUT_H,
        HAMSTRONE_CONFIG_MPU6050_GYRO_ZOUT_H,
        HAMSTRONE_CONFIG_MPU6050_TEMP_OUT_H,
    };
    uint8_t valuel, valueh;
    uint16_t value[VALUE_CNT];
    double accelX, accelY, accelZ, gyroX, gyroY, gyroZ;
    double accelXsq, accelYsq, accelZsq, accelAngX, accelAngY;
    double gyroAngX, gyroAngY, gyroAngZ;
    double filterAngX, filterAngY;
    double pidAssemble[MAX_SIZE];
    
    int errcnt;

    /* initialize mpu6050 */
    if (I2CWriteSingle(HAMSTRONE_GLOBAL_IMU_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050, HAMSTRONE_CONFIG_MPU6050_PWR_MGMT_1, 0b00000000) < 0)
    {
        HAMSTERTONGUE_WriteAndFreeMessage(
            HAMSTRONE_GLOBAL_TELEMETRY_PORT,
            HAMSTERTONGUE_NewFormatStringMessage(
                HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_I2CREADFAIL,
                24,
                "fd=%d mpu6050 pwr_mgmt_1",
                HAMSTRONE_GLOBAL_IMU_PORT));
    }
    while (1)
    {
        /* update runtime */
        clock_gettime(CLOCK_MONOTONIC, &currentTs);
        HAMSTRONE_WriteValueStore(0, (uint32_t)(currentTs.tv_sec - startTs.tv_sec));

        /* update i2c sensor value */
        for (int i = 0; i < VALUE_CNT; i++)
        {
            errcnt = 0;
            if (I2CReadWriteSingle(HAMSTRONE_GLOBAL_IMU_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050, regAddr[i], &valueh) < 0)
                errcnt++;
            if (I2CReadWriteSingle(HAMSTRONE_GLOBAL_IMU_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050, regAddr[i] + 1, &valuel) < 0)
                errcnt++;
            if (errcnt > 0)
            {
                HAMSTERTONGUE_WriteAndFreeMessage(
                    HAMSTRONE_GLOBAL_TELEMETRY_PORT,
                    HAMSTERTONGUE_NewFormatStringMessage(
                        HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                        HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_I2CREADFAIL,
                        24,
                        "fd=%d errcnt=%d",
                        HAMSTRONE_GLOBAL_IMU_PORT, errcnt));
                continue;
            }
            value[i] = (valueh << 8) | valuel;
        }

        /* calculate gyro and accel angle*/
        accelX = (int16_t)(~value[0] + 1) / HAMSTRONE_CONFIG_MPU6050_ACCEL_COEFFICIENT;
        accelY = (int16_t)(~value[1] + 1) / HAMSTRONE_CONFIG_MPU6050_ACCEL_COEFFICIENT;
        accelZ = (int16_t)(~value[2] + 1) / HAMSTRONE_CONFIG_MPU6050_ACCEL_COEFFICIENT;
        gyroX = (int16_t)(~value[3] + 1) / HAMSTRONE_CONFIG_MPU6050_GYRO_COEFFICIENT;
        gyroY = (int16_t)(~value[4] + 1) / HAMSTRONE_CONFIG_MPU6050_GYRO_COEFFICIENT;
        gyroZ = (int16_t)(~value[5] + 1) / HAMSTRONE_CONFIG_MPU6050_GYRO_COEFFICIENT;
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
        if (filterAngX>=10000)
            filterAngX==0;
        if (filterAngY>=10000)
            filterAngY==0;

        pidControl(filterAngX,filterAngY,pidAssemble);

        HAMSTRONE_WriteValueStore(2, (uint32_t)(filterAngX * 100 + 18000));
        HAMSTRONE_WriteValueStore(3, (uint32_t)(filterAngY * 100 + 18000));
        HAMSTRONE_WriteValueStore(4, (uint32_t)(gyroAngZ * 100 + 18000));

        HAMSTRONE_WriteValueStore(6, (uint32_t)(10*(pidAssemble[0]+pidAssemble[1])+200));
        HAMSTRONE_WriteValueStore(7, (uint32_t)(10*(pidAssemble[0]-pidAssemble[1])+200));
        HAMSTRONE_WriteValueStore(8, (uint32_t)(10*(-pidAssemble[0]+pidAssemble[1])+200));
        HAMSTRONE_WriteValueStore(9, (uint32_t)(10*(-pidAssemble[0]-pidAssemble[1])+200));
        
        usleep(period);
        clock_gettime(CLOCK_MONOTONIC, &taskendTs);
        // PROPERY TICK RESOULUTION IS SMALLER THAN 1000USEC
        HAMSTRONE_WriteValueStore(1, (uint32_t)((taskendTs.tv_nsec - currentTs.tv_nsec) / 1000000));
    }
}

int tskParsingGPS(int argc, char *argv[])
{
	#define MSG_BUF_SIZE 33
    int assembleCnt=0, i=0, bufLen=0, assembleLen=0, Len=0;
    char Assemble_Data[200]={0,};
    char buf[MSG_BUF_SIZE];
    while(1)
	{
        memset(buf,0x00,33);
        read(HAMSTRONE_GLOBAL_GPS_PORT, buf, 32);
        bufLen=strlen(buf);
        assembleLen=strlen(Assemble_Data);
        for(i=0;i<bufLen;i++)
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

int I2CWriteSingle(int fd, uint16_t addr, uint8_t regaddr, uint8_t value)
{
    struct i2c_msg_s msg[1];
    struct i2c_transfer_s trans;
    uint8_t rawbuf[2] = {regaddr, value};

    msg[0].addr = addr;
    msg[0].flags = 0;
    msg[0].buffer = rawbuf;
    msg[0].length = 2;
    msg[0].frequency = 400000;

    trans.msgv = (struct i2c_msg_s *)msg;
    trans.msgc = 1;

    return ioctl(fd, I2CIOC_TRANSFER, &trans);
}

int I2CReadWriteSingle(int fd, uint16_t addr, uint8_t regaddr, uint8_t *buf)
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
