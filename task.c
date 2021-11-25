#include "include/task.h"

int tskTransmitValue(int argc, char *argv[])
{
    int period = atoi(argv[1]);
    if (period <= 0)
        period = 100000; //100ms
    HAMSTERTONGUE_Message *msg = HAMSTERTONGUE_NewMessage(HAMSTERTONGUE_MESSAGE_VERB_VALUE, 0, sizeof(HAMSTRONE_CONFIG_VALUE_TYPE) * HAMSTRONE_CONFIG_VALUE_SIZE);

    while (1)
    {
        HAMSTRONE_Serialize32Array(HAMSTRONE_GetValueStorePointer(), msg->Payload, HAMSTRONE_CONFIG_VALUE_SIZE, 0);
        HAMSTERTONGUE_WriteMessage(HAMSTRONE_GLOBAL_TELEMETRY_PORT, msg);
        usleep(period);
    }
}

#define SO6203_COUNT 0
#define TFMINI_COUNT 0
int tskUpdateValue(int argc, char *argv[])
{
    int period = atoi(argv[1]);
    if (period <= 0)
        period = 10000; //2ms

    struct timespec startTs, currentTs, taskendTs;
    clock_gettime(CLOCK_MONOTONIC, &startTs);

    mpu9250Data mpudata;
    double angle[3], pidangle[3];
    uint16_t motor[4] = {0, 0, 0, 0};
    uint16_t bright[SO6203_COUNT], dist[TFMINI_COUNT];

    /* initialize SO6203 */
    if (initSO6203() < 0)
    {
        HAMSTERTONGUE_WriteAndFreeMessage(
            HAMSTRONE_GLOBAL_TELEMETRY_PORT,
            HAMSTERTONGUE_NewFormatStringMessage(
                HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_SENSORINITFAIL,
                32,
                "init so6203"));
    }

    if (initMPU9250() < 0)
    {
        HAMSTERTONGUE_WriteAndFreeMessage(
            HAMSTRONE_GLOBAL_TELEMETRY_PORT,
            HAMSTERTONGUE_NewFormatStringMessage(
                HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_SENSORINITFAIL,
                16,
                "init mpu9250"));
        return;
    }

    while (1)
    {
        /* update runtime */
        clock_gettime(CLOCK_MONOTONIC, &currentTs);
        HAMSTRONE_WriteValueStore(0, (uint32_t)(currentTs.tv_sec - startTs.tv_sec));

        /* update so6203 sensor value */
        if (readSO6203(0, SO6203_COUNT, bright) < 0)
        {
            HAMSTERTONGUE_WriteAndFreeMessage(
                HAMSTRONE_GLOBAL_TELEMETRY_PORT,
                HAMSTERTONGUE_NewFormatStringMessage(
                    HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                    HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_SENSORREADFAIL,
                    16,
                    "read so6203"));
        }
        for (int i = 0; i < SO6203_COUNT; i++)
        {
            HAMSTRONE_WriteValueStore(10 + i, (uint32_t)bright[i]);
        }

        /* update tfmini sensor value */
        if (readSO6203(0, TFMINI_COUNT, dist) < 0)
        {
            HAMSTERTONGUE_WriteAndFreeMessage(
                HAMSTRONE_GLOBAL_TELEMETRY_PORT,
                HAMSTERTONGUE_NewFormatStringMessage(
                    HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                    HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_SENSORREADFAIL,
                    16,
                    "read tfmini"));
        }
        for (int i = 0; i < TFMINI_COUNT; i++)
        {
            HAMSTRONE_WriteValueStore(10 + TFMINI_COUNT + i, (uint32_t)dist[i]);
        }

        /* update mpu9250 sensor value */
        if (readMPU9250(&mpudata) < 0)
        {
            HAMSTERTONGUE_WriteAndFreeMessage(
                HAMSTRONE_GLOBAL_TELEMETRY_PORT,
                HAMSTERTONGUE_NewFormatStringMessage(
                    HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                    HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_SENSORREADFAIL,
                    16,
                    "read mpu9250"));
        }

        /* calculate posture */
        updateKalman(&mpudata, angle);
        //updateComplimentary(&mpudata, angle);
        HAMSTRONE_WriteValueStore(2, (uint32_t)(angle[0] * 100 + 18000));
        HAMSTRONE_WriteValueStore(3, (uint32_t)(angle[1] * 100 + 18000));
        HAMSTRONE_WriteValueStore(4, (uint32_t)(angle[2] * 100 + 18000));
        HAMSTRONE_WriteValueStore(5, (uint32_t)(mpudata.temp * 100 + 10000));

        /* process pid control*/
        updatePID(angle[0], angle[1], pidangle);
        motor[0] = 2 * (pidangle[0] + pidangle[1]) + 150;
        motor[1] = 2 * (pidangle[0] - pidangle[1]) + 150;
        motor[2] = 2 * (-pidangle[0] + pidangle[1]) + 150;
        motor[3] = 2 * (-pidangle[0] - pidangle[1]) + 150;

        PWMWriteAll(HAMSTRONE_GLOBAL_MOTOR_PWM_INFO, motor[0], motor[1], motor[2], motor[3]);
        HAMSTRONE_WriteValueStore(6, (uint32_t)motor[0]);
        HAMSTRONE_WriteValueStore(7, (uint32_t)motor[1]);
        HAMSTRONE_WriteValueStore(8, (uint32_t)motor[2]);
        HAMSTRONE_WriteValueStore(9, (uint32_t)motor[3]);

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