/**
 * Many logics refer to https://github.com/brianc118/MPU9250
 * Copyright (C) 2015 Brian Chen
 */

#include "include/mpu9250.h"

#define INIT_REGISTER_COUNT 15
uint8_t initRegister[INIT_REGISTER_COUNT][2] = {
    {MPUREG_PWR_MGMT_1, BIT_H_RESET},
    {MPUREG_PWR_MGMT_1, 0x01},
    {MPUREG_PWR_MGMT_2, 0x00},
    {MPUREG_ACCEL_CONFIG, BITS_FS_2G},
    {MPUREG_GYRO_CONFIG, BITS_FS_250DPS},
    {MPUREG_INT_PIN_CFG, 0x12},
    {MPUREG_USER_CTRL, 0x30},
    {MPUREG_I2C_MST_CTRL, 0x0D},
    {MPUREG_I2C_SLV0_ADDR, AK8963_I2C_ADDR},
    {MPUREG_I2C_SLV0_REG, AK8963_CNTL2}, // ak reset
    {MPUREG_I2C_SLV0_DO, 0x01},
    {MPUREG_I2C_SLV0_CTRL, 0x81},
    {MPUREG_I2C_SLV0_REG, AK8963_CNTL1},
    {MPUREG_I2C_SLV0_DO, 0x12},
    {MPUREG_I2C_SLV0_CTRL, 0x81}};

float gyroBias[3];
float accelBias[3];
float magBias[3];

/**
 * @brief init mpu9250 and ak8963 and calibrate both
 * 
 * @return int negative if error
 */
int initMPU9250()
{
    // calibrate mpu9250
    calibrateMPU9250(gyroBias, accelBias);
    mpudebug("initMPU9250: calibrateMPU9250");
    mpudebug("initMPU9250: accelBias: %f, %f, %f", accelBias[0], accelBias[1], accelBias[2]);
    mpudebug("initMPU9250: gyroBias: %f, %f, %f", gyroBias[0], gyroBias[1], gyroBias[2]);
    // init register setting
    for (int i = 0; i < INIT_REGISTER_COUNT; i++)
    {
        SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, initRegister[i][0], initRegister[i][1]);
        mpudebug("initMPU9250: init reg %d = %d", initRegister[i][0], initRegister[i][1]);
        usleep(1000);
    }

    // calibrate ak8963
    //calibrateAK8963();
    //mpudebug("initMPU9250: calibrateAK8963");

    // read mpu9250 whoami
    uint8_t whoami;
    SPIReadSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_WHOAMI | READ_FLAG, &whoami);
    mpudebug("initMPU9250: mpu9250 wai=%d", whoami);
    if (whoami != MPU9250_WHOAMI_VALUE)
        return ERROR_INVALID_WHOAMI;

    // read ak8963 whoami
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_I2C_SLV0_ADDR, AK8963_I2C_ADDR | READ_FLAG); //Set the I2C slave addres of AK8963 and set for read.
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_I2C_SLV0_REG, AK8963_WIA);                   //I2C slave 0 register address from where to begin data transfer
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_I2C_SLV0_CTRL, 0x81);
    usleep(1000);
    SPIReadSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_EXT_SENS_DATA_00 | READ_FLAG, &whoami);
    mpudebug("initMPU9250: ak8963 wai=%d", whoami);
    if (whoami != AK8963_WHOAMI_VALUE)
        //return ERROR_INVALID_WHOAMI;
        return 0;
}

int readMPU9250(mpu9250Data *ret)
{
    // request read ak8963
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_I2C_SLV0_ADDR, AK8963_I2C_ADDR | READ_FLAG);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_I2C_SLV0_REG, AK8963_HXL);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_I2C_SLV0_CTRL, 0x87);
    mpudebug("readMPU9250: ak8963 request");

    // read mpu9250
    uint8_t data[21];
    if (SPIRead(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_ACCEL_XOUT_H | READ_FLAG, 21, data) < 0)
    {
        mpudebug("readMPU9250: read error");
        return ERROR_READ_FAIL;
    }
    mpudebug("readMPU9250: read ok");

    uint8_t i;
    int16_t value[10];
    for (i = 0; i < 3; i++) // accel : i = 0, 1, 2
    {
        value[i] = ((int16_t)data[2 * i] << 8) | data[2 * i + 1];
    }
    for (i = 4; i < 7; i++) // gyro : i = 4, 5, 6
    {
        value[i - 1] = ((int16_t)data[2 * i] << 8) | data[2 * i + 1];
    }
    for (i = 7; i < 10; i++) // mag : i = 7, 8, 9; inverse order
    {
        value[i - 1] = ((int16_t)data[2 * i + 1] << 8) | data[2 * i];
    }
    value[10] = ((int16_t)data[6] << 8) | data[7];

    /* calculate gyro and accel angle*/
    ret->accX = ((float)value[0] / MPU9250_ACCEL_COEFFICIENT) - accelBias[0];
    ret->accY = ((float)value[1] / MPU9250_ACCEL_COEFFICIENT) - accelBias[1];
    ret->accZ = ((float)value[2] / MPU9250_ACCEL_COEFFICIENT) - accelBias[2];
    ret->gyroX = ((float)value[3] / MPU9250_GYRO_COEFFICIENT) - gyroBias[0];
    ret->gyroY = ((float)value[4] / MPU9250_GYRO_COEFFICIENT) - gyroBias[1];
    ret->gyroZ = ((float)value[5] / MPU9250_GYRO_COEFFICIENT) - gyroBias[2];
    ret->magX = (float)value[6] * magBias[0];
    ret->magX = (float)value[7] * magBias[1];
    ret->magX = (float)value[8] * magBias[2];
    ret->temp = (((float)value[10] - 21) / 333.87) + 21;
    mpudebug("readMPU9250: a %f, %f, %f", ret->accX, ret->accY, ret->accZ);
    mpudebug("readMPU9250: g %f, %f, %f", ret->gyroX, ret->gyroY, ret->gyroZ);
    mpudebug("readMPU9250: m %f, %f, %f", ret->magX, ret->magY, ret->magZ);
}

/**
 * @brief calibrate MPU9250
 * 
 * @param dest1 gyro bias result
 * @param dest2 accel bias result
 */
void calibrateMPU9250(float *dest1, float *dest2)
{
    uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
    uint16_t ii, packet_count, fifo_count;
    int32_t gyro_bias[3] = {0, 0, 0}, accel_bias[3] = {0, 0, 0};

    // reset device
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
    usleep(1000000);

    // get stable time source; Auto select clock source to be PLL gyroscope reference if ready
    // else use the internal oscillator, bits 2:0 = 001
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_PWR_MGMT_1, 0x01);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_PWR_MGMT_2, 0x00);
    usleep(200000);

    // Configure device for bias calculation
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_INT_ENABLE, 0x00);   // Disable all interrupts
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_FIFO_EN, 0x00);      // Disable FIFO
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_PWR_MGMT_1, 0x00);   // Turn on internal clock source
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_I2C_MST_CTRL, 0x00); // Disable I2C master
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_USER_CTRL, 0x0C);    // Reset FIFO and DMP
    usleep(15000);

    // Configure MPU6050 gyro and accelerometer for bias calculation
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_CONFIG, 0x01);       // Set low-pass filter to 188 Hz
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_SMPLRT_DIV, 0x00);   // Set sample rate to 1 kHz
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_GYRO_CONFIG, 0x00);  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity

    uint16_t gyrosensitivity = 131;    // = 131 LSB/degrees/sec
    uint16_t accelsensitivity = 16384; // = 16384 LSB/g

    // Configure FIFO to capture accelerometer and gyro data for bias calculation
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_USER_CTRL, 0x40); // Enable FIFO
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_FIFO_EN, 0x78);   // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in MPU-9150)
    usleep(40000);                                                                       // accumulate 40 samples in 40 milliseconds = 480 bytes

    // At end of sample accumulation, turn off FIFO sensor read
    uint8_t datah, datal;
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_FIFO_EN, 0x00); // Disable gyro and accelerometer sensors for FIFO
    SPIReadSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_FIFO_COUNTH | READ_FLAG, &datah);
    SPIReadSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, (MPUREG_FIFO_COUNTH + 1) | READ_FLAG, &datal);
    fifo_count = ((uint16_t)datah << 8) | datal;
    packet_count = fifo_count / 12; // How many sets of full gyro and accelerometer data for averaging

    for (ii = 0; ii < packet_count; ii++)
    {
        int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
        SPIRead(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_FIFO_R_W | READ_FLAG, 12, data); // read data for averaging
        accel_temp[0] = (int16_t)(((int16_t)data[0] << 8) | data[1]);                                // Form signed 16-bit integer for each sample in FIFO
        accel_temp[1] = (int16_t)(((int16_t)data[2] << 8) | data[3]);
        accel_temp[2] = (int16_t)(((int16_t)data[4] << 8) | data[5]);
        gyro_temp[0] = (int16_t)(((int16_t)data[6] << 8) | data[7]);
        gyro_temp[1] = (int16_t)(((int16_t)data[8] << 8) | data[9]);
        gyro_temp[2] = (int16_t)(((int16_t)data[10] << 8) | data[11]);

        accel_bias[0] += (int32_t)accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
        accel_bias[1] += (int32_t)accel_temp[1];
        accel_bias[2] += (int32_t)accel_temp[2];
        gyro_bias[0] += (int32_t)gyro_temp[0];
        gyro_bias[1] += (int32_t)gyro_temp[1];
        gyro_bias[2] += (int32_t)gyro_temp[2];
    }
    accel_bias[0] /= (int32_t)packet_count; // Normalize sums to get average count biases
    accel_bias[1] /= (int32_t)packet_count;
    accel_bias[2] /= (int32_t)packet_count;
    gyro_bias[0] /= (int32_t)packet_count;
    gyro_bias[1] /= (int32_t)packet_count;
    gyro_bias[2] /= (int32_t)packet_count;

    if (accel_bias[2] > 0L)
    {
        accel_bias[2] -= (int32_t)accelsensitivity;
    } // Remove gravity from the z-axis accelerometer bias calculation
    else
    {
        accel_bias[2] += (int32_t)accelsensitivity;
    }

    // Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
    data[0] = (-gyro_bias[0] / 4 >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
    data[1] = (-gyro_bias[0] / 4) & 0xFF;      // Biases are additive, so change sign on calculated average gyro biases
    data[2] = (-gyro_bias[1] / 4 >> 8) & 0xFF;
    data[3] = (-gyro_bias[1] / 4) & 0xFF;
    data[4] = (-gyro_bias[2] / 4 >> 8) & 0xFF;
    data[5] = (-gyro_bias[2] / 4) & 0xFF;

    // Push gyro biases to hardware registers
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_XG_OFFS_USRH, data[0]);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_XG_OFFS_USRL, data[1]);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_YG_OFFS_USRH, data[2]);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_YG_OFFS_USRL, data[3]);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_ZG_OFFS_USRH, data[4]);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_ZG_OFFS_USRL, data[5]);

    // Output scaled gyro biases for display in the main program
    dest1[0] = (float)gyro_bias[0] / (float)gyrosensitivity;
    dest1[1] = (float)gyro_bias[1] / (float)gyrosensitivity;
    dest1[2] = (float)gyro_bias[2] / (float)gyrosensitivity;

    // Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
    // factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
    // non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
    // compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
    // the accelerometer biases calculated above must be divided by 8.

    int32_t accel_bias_reg[3] = {0, 0, 0};                                                         // A place to hold the factory accelerometer trim biases
    SPIRead(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_XA_OFFSET_H | READ_FLAG, 2, data); // Read factory accelerometer trim values
    accel_bias_reg[0] = (int32_t)(((int16_t)data[0] << 8) | data[1]);
    SPIRead(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_YA_OFFSET_H | READ_FLAG, 2, data);
    accel_bias_reg[1] = (int32_t)(((int16_t)data[0] << 8) | data[1]);
    SPIRead(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_ZA_OFFSET_H | READ_FLAG, 2, data);
    accel_bias_reg[2] = (int32_t)(((int16_t)data[0] << 8) | data[1]);

    uint32_t mask = 1uL;             // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
    uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis

    for (ii = 0; ii < 3; ii++)
    {
        if ((accel_bias_reg[ii] & mask))
            mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
    }

    // Construct total accelerometer bias, including calculated average accelerometer bias from above
    accel_bias_reg[0] -= (accel_bias[0] / 8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
    accel_bias_reg[1] -= (accel_bias[1] / 8);
    accel_bias_reg[2] -= (accel_bias[2] / 8);

    data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
    data[1] = (accel_bias_reg[0]) & 0xFF;
    data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
    data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
    data[3] = (accel_bias_reg[1]) & 0xFF;
    data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
    data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
    data[5] = (accel_bias_reg[2]) & 0xFF;
    data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers

    // Apparently this is not working for the acceleration biases in the MPU-9250
    // Are we handling the temperature correction bit properly?
    // Push accelerometer biases to hardware registers
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_XA_OFFSET_H, data[0]);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_XA_OFFSET_L, data[1]);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_YA_OFFSET_H, data[2]);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_YA_OFFSET_L, data[3]);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_ZA_OFFSET_H, data[4]);
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_ZA_OFFSET_L, data[5]);

    // Output scaled accelerometer biases for display in the main program
    dest2[0] = (float)accel_bias[0] / (float)accelsensitivity;
    dest2[1] = (float)accel_bias[1] / (float)accelsensitivity;
    dest2[2] = (float)accel_bias[2] / (float)accelsensitivity;
}

void calibrateAK8963()
{
    uint8_t response[3];
    float data;
    int i;
    // Choose either 14-bit or 16-bit magnetometer resolution
    //uint8_t MFS_14BITS = 0; // 0.6 mG per LSB
    uint8_t MFS_16BITS = 1; // 0.15 mG per LSB
    // 2 for 8 Hz, 6 for 100 Hz continuous magnetometer data read
    uint8_t M_8HZ = 0x02; // 8 Hz update
                          //uint8_t M_100HZ = 0x06; // 100 Hz continuous magnetometer

    /* get the magnetometer calibration */

    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_I2C_SLV0_ADDR, AK8963_I2C_ADDR | READ_FLAG); // Set the I2C slave    addres of AK8963 and set for read.
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_I2C_SLV0_REG, AK8963_ASAX);                  // I2C slave 0 register address from where to begin data transfer
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_I2C_SLV0_CTRL, 0x83);                        // Read 3 bytes from the magnetometer

    //WriteReg(MPUREG_I2C_SLV0_CTRL, 0x81);                     // Enable I2C and set bytes
    usleep(100000);
    //response[0]=WriteReg(MPUREG_EXT_SENS_DATA_01|READ_FLAG, 0x00); //Read I2C

    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, AK8963_CNTL1, 0x00); // set AK8963 to Power Down
    usleep(50000);                                                                   // long wait between AK8963 mode changes
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, AK8963_CNTL1, 0x0F); // set AK8963 to FUSE ROM access
    usleep(50000);                                                                   // long wait between AK8963 mode changes

    SPIRead(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, MPUREG_EXT_SENS_DATA_00, 3, response);
    //response=WriteReg(MPUREG_I2C_SLV0_DO, 0x00);              // Read I2C
    for (i = 0; i < 3; i++)
    {
        data = response[i];
        magBias[i] = ((data - 128) / 256 + 1) * AK8963_SENSITIVITY_FACTOR;
    }
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, AK8963_CNTL1, 0x00); // set AK8963 to Power Down
    usleep(50000);
    // Configure the magnetometer for continuous read and highest resolution.
    // Set bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL
    // register, and enable continuous mode data acquisition (bits [3:0]),
    // 0010 for 8 Hz and 0110 for 100 Hz sample rates.
    SPIWriteSingle(HAMSTRONE_GLOBAL_SPI_PORT, MPU9250_SPI_MODE, AK8963_CNTL1, MFS_16BITS << 4 | M_8HZ); // Set magnetometer data resolution and sample ODR
    usleep(50000);
}