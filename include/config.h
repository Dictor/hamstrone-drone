#ifndef HAMSTRONE_CONFIG_H
#define HAMSTRONE_CONFIG_H

#define HAMSTRONE_CONFIG_VALUE_SIZE 16
#define HAMSTRONE_CONFIG_VALUE_TYPE uint32_t

#define HAMSTRONE_CONFIG_SERIALPORT1_PATH "/dev/ttyS0"
#define HAMSTRONE_CONFIG_I2CPORT1_PATH "/dev/i2c1"
#define HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050 0x68

#define HAMSTRONE_CONFIG_MPU6050_PWR_MGMT_1 0x6B
#define HAMSTRONE_CONFIG_MPU6050_WHOAMI 0x75
#define HAMSTRONE_CONFIG_MPU6050_TEMP_OUT_H 0x41
#define HAMSTRONE_CONFIG_MPU6050_TEMP_OUT_L 0x42
#define HAMSTRONE_CONFIG_MPU6050_ACCEL_XOUT_H 0x3B
#define HAMSTRONE_CONFIG_MPU6050_ACCEL_XOUT_L 0x3C
#define HAMSTRONE_CONFIG_MPU6050_ACCEL_YOUT_H 0x3D
#define HAMSTRONE_CONFIG_MPU6050_ACCEL_YOUT_L 0x3E
#define HAMSTRONE_CONFIG_MPU6050_ACCEL_ZOUT_H 0x3F
#define HAMSTRONE_CONFIG_MPU6050_ACCEL_ZOUT_L 0x40

#define HAMSTRONE_CONFIG_ISSTANDALONE

#endif