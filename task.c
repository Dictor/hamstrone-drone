#include "include/task.h"

int tskTransmitValue(int argc, char *argv[])
{
    int period = atoi(argv[1]);
    if (period <= 0)
        period = 200000; //200ms
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
        period = 1000; //2ms

    struct timespec startTs, currentTs, taskendTs;
    clock_gettime(CLOCK_MONOTONIC, &startTs);

#define SENSOR_CNT 1
#define VALUE_CNT 4
    uint8_t valuel, valueh;
    uint16_t value;
    uint8_t devAddr[SENSOR_CNT] = {
        HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050,
    };
    uint8_t regAddr[SENSOR_CNT][VALUE_CNT] = {
        {
            HAMSTRONE_CONFIG_MPU6050_ACCEL_XOUT_H,
            HAMSTRONE_CONFIG_MPU6050_ACCEL_YOUT_H,
            HAMSTRONE_CONFIG_MPU6050_ACCEL_ZOUT_H,
            HAMSTRONE_CONFIG_MPU6050_GYRO_XOUT_H,
            HAMSTRONE_CONFIG_MPU6050_GYRO_YOUT_H,
            HAMSTRONE_CONFIG_MPU6050_GYRO_ZOUT_H,
            HAMSTRONE_CONFIG_MPU6050_TEMP_OUT_H,
        }};
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
        for (int s = 0; s < SENSOR_CNT; s++)
        {
            for (int i = 0; i < VALUE_CNT; i++)
            {
                errcnt = 0;
                if (I2CReadWriteSingle(HAMSTRONE_GLOBAL_IMU_PORT, devAddr[s], regAddr[s][i], &valueh) < 0)
                    errcnt++;
                if (I2CReadWriteSingle(HAMSTRONE_GLOBAL_IMU_PORT, devAddr[s], regAddr[s][i] + 1, &valuel) < 0)
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
                value = (valueh << 8) | valuel;
                HAMSTRONE_WriteValueStore(2 + (s * VALUE_CNT) + i, (uint32_t)value);
            }
        }
        usleep(period);
        clock_gettime(CLOCK_MONOTONIC, &taskendTs);
        // PROPERY TICK RESOULUTION IS SMALLER THAN 1000USEC
        HAMSTRONE_WriteValueStore(1, (uint32_t)((taskendTs.tv_nsec - currentTs.tv_nsec) / 1000000));
    }
}

int tskParsingGPS(int argc, char *argv[])
{
	#define MSG_BUF_SIZE 32
    char buf[MSG_BUF_SIZE];
    int rd;
    char *Assemble_Data[200]={0,};
    struct Ele_Num Ele;
    memset(buf,0x00,32);
    while(1) 
	{
        rd=read(HAMSTRONE_GLOBAL_GPS_PORT,buf,32);
        if(rd<0)
			HAMSTERTONGUE_Debugf("Error\n");
        else
            HAMSTERTONGUE_Debugf("%d\n", rd);
        HAMSTERTONGUE_Debugf("zero %s\n", buf);
        strcat(Assemble_Data,buf);
        HAMSTERTONGUE_Debugf("first %s\n", Assemble_Data);
        Checking(Assemble_Data);
        usleep(200000);
    }
    return 0;
}

int I2CWriteSingle(int fd, uint16_t addr, uint8_t regaddr, uint8_t value) {
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
