#include "include/task.h"

struct Ele_Num {
    int Element[50];
    int num;
};

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

    #define VALUE_CNT 4
    uint8_t valuel, valueh;
    uint16_t value;
    uint8_t devAddr[VALUE_CNT] = {
        HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050,
        HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050,
        HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050,
        HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050,
    };
    uint8_t regAddr[VALUE_CNT] = {
        HAMSTRONE_CONFIG_MPU6050_ACCEL_XOUT_H,
        HAMSTRONE_CONFIG_MPU6050_ACCEL_YOUT_H,
        HAMSTRONE_CONFIG_MPU6050_ACCEL_ZOUT_H,
        HAMSTRONE_CONFIG_MPU6050_TEMP_OUT_H,
    };
    int errcnt;

    if (I2CWriteSingle(HAMSTRONE_GLOBAL_IMU_PORT, HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050, HAMSTRONE_CONFIG_MPU6050_PWR_MGMT_1, 0b00000000) < 0) {
        HAMSTERTONGUE_WriteAndFreeMessage(
            HAMSTRONE_GLOBAL_TELEMETRY_PORT,
            HAMSTERTONGUE_NewFormatStringMessage(
                HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_I2CREADFAIL,
                24,
                "fd=%d pwr_mgmt_1",
                HAMSTRONE_GLOBAL_IMU_PORT
            )
        );
    }
    while (1)
    {
        /* update runtime */
        clock_gettime(CLOCK_MONOTONIC, &currentTs);
        HAMSTRONE_WriteValueStore(0, (uint32_t)(currentTs.tv_sec - startTs.tv_sec));

        /* update mpu6050 */
        for (int i = 0; i < VALUE_CNT; i++) {
            errcnt = 0;
            if (I2CReadWriteSingle(HAMSTRONE_GLOBAL_IMU_PORT, devAddr[i], regAddr[i], &valueh) < 0) errcnt++;
            if (I2CReadWriteSingle(HAMSTRONE_GLOBAL_IMU_PORT, devAddr[i], regAddr[i]+1, &valuel) < 0) errcnt++;
            if (errcnt > 0) {
                HAMSTERTONGUE_WriteAndFreeMessage(
                    HAMSTRONE_GLOBAL_TELEMETRY_PORT,
                    HAMSTERTONGUE_NewFormatStringMessage(
                        HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
                        HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_I2CREADFAIL,
                        24,
                        "fd=%d errcnt=%d",
                        HAMSTRONE_GLOBAL_IMU_PORT, errcnt
                    )
                );
                continue;
            }
            value = (valueh << 8) | valuel;
            HAMSTRONE_WriteValueStore(2 + i, (uint32_t)value);
        }
        usleep(period);
        clock_gettime(CLOCK_MONOTONIC, &taskendTs);
        // PROPERY TICK RESOULUTION IS SMALL THAN 1000USEC
        HAMSTRONE_WriteValueStore(1, (uint32_t)((taskendTs.tv_nsec - currentTs.tv_nsec) / 1000000));
    }
}

void GPS_type_GGA(char * Data_receive,struct Ele_Num GGAN)
{
	int Comma_cnt=0, i, j;
	for(i=0;i<strlen(Data_receive);i++)
	{
		if(Data_receive[i]==',')
			Comma_cnt++;	
	}
	if(Comma_cnt!=14)
	{
		HAMSTERTONGUE_Debugf("Data was damagegd\n");
	}
	else
	{
		printf("%d\n", GGAN.num);
		for(i=0;i<GGAN.num-1;i++)
		{
			if(i==5||i==6||i==7||i==9)
			{
				for(j=GGAN.Element[i];j<GGAN.Element[i+1]-1;j++)
					HAMSTERTONGUE_Debugf("%c ", Data_receive[j]);
			}
		}
        printf("\n");
	}
}

void GPS_type_GLL(char * Data_receive,struct Ele_Num GLLN)
{
	int Comma_cnt=0, i, j;
	for(i=0;i<strlen(Data_receive);i++)
	{
		if(Data_receive[i]==',')
			Comma_cnt++;	
	}
	if(Comma_cnt!=7)
	{
		HAMSTERTONGUE_Debugf("Data was damagegd\n");
	}
	else
	{
		printf("%d\n", GLLN.num);
		for(i=0;i<GLLN.num-1;i++)
		{
			if(i==1||i==2||i==3||i==4)
			{
				for(j=GLLN.Element[i];j<GLLN.Element[i+1]-1;j++)
					HAMSTERTONGUE_Debugf("%c ", Data_receive[j]);
			}
		}
        printf("\n");
	}
}

void Split(char *Data_receive)
{
    int Data_cnt = 0, Ele_cnt = 1, i, start, len=0;
    char GPS_Type[4];
    struct Ele_Num Ele;
    Ele.Element[0]=0;
    while (1)
    {
        if (Data_receive[Data_cnt] == ',')
        {
            Ele.Element[Ele_cnt] = Data_cnt + 1;
            Ele_cnt++;
            if(Data_receive[Data_cnt+1]==',')
            	Data_cnt++;
        }
        len++;
        if(len==strlen(Data_receive))
        	break;
        Data_cnt++;
    }
    for(i=0;i<3;i++)
    	GPS_Type[i]=Data_receive[i+2];
	Ele.num = Ele_cnt;
	if(strcmp(GPS_Type,"GGA")==0)
	{
		GPS_type_GGA(Data_receive,Ele);	
	}		
	else if(strcmp(GPS_Type,"GLL")==0)
	{
		GPS_type_GLL(Data_receive,Ele);
	}
}

int tskParsingGPS(int argc, char *argv[])
{
    #define MSG_BUF_SIZE 32
    char buf[MSG_BUF_SIZE];
    char Assemble_Data[100]={0,};
    char Splited_Data[100]={0,};
    int ret, reterr, i, k=0, Data_len, Data_start, Data_end, Data_split=0, Condition=0;
    struct Ele_Num Ele;
    mqd_t mq = mq_open("/mqgps", O_RDWR);
    HAMSTERTONGUE_Debugf("msq: %d", mq);
    HAMSTERTONGUE_Debugf("First\n");
    while(1) { 
        ret = mq_receive(mq, buf, MSG_BUF_SIZE, NULL);
        strcat(Assemble_Data,buf);
        Data_len=strlen(Assemble_Data);
        for(i=0;i<Data_len;i++)
        {
        	if(Assemble_Data[i]=='$')
        	{
        		if(Data_split==1)
        		{
        			Data_end=i;
        			Condition=1;
        			break;
				}
				else
				{
					Data_split=1;
					Data_start=i+1;
				}
			}
		}
		if(Condition==1)
		{
            HAMSTERTONGUE_Debugf("Third\n");
			for(i=0; i<Data_end-Data_start;i++)
				Splited_Data[i]=Assemble_Data[Data_start+i];
			for(i=0;i<Data_len-Data_end;i++)
			{
				Assemble_Data[i]=Assemble_Data[Data_end+k];
				k++;
			}
			for(i=Data_len-Data_end;i<Data_len;i++)
				Assemble_Data[i]='\0';
        	Split(Splited_Data);
		}
        HAMSTERTONGUE_Debugf("Second\n");
		Data_split=0;
		Condition=0;
		k=0;
        usleep(5000000); 
    } 
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
