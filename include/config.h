#ifndef HAMSTRONE_CONFIG_H
#define HAMSTRONE_CONFIG_H

#define HAMSTRONE_CONFIG_VALUE_SIZE 16
#define HAMSTRONE_CONFIG_VALUE_TYPE uint32_t

#define HAMSTRONE_CONFIG_SERIALPORT1_PATH "/dev/ttyS0"
#define HAMSTRONE_CONFIG_SERIALPORT2_PATH "/dev/ttyS1"
#define HAMSTRONE_CONFIG_I2CPORT1_PATH "/dev/i2c3"
#define HAMSTRONE_CONFIG_TIMER1PWM_PATH "/dev/pwm0"
#define HAMSTRONE_CONFIG_I2C_ADDRESS_MPU6050 0x68

#define HAMSTRONE_CONFIG_MPU6050_PWR_MGMT_1 0x6B
#define HAMSTRONE_CONFIG_MPU6050_WHOAMI 0x75
#define HAMSTRONE_CONFIG_MPU6050_TEMP_OUT_H 0x41
#define HAMSTRONE_CONFIG_MPU6050_ACCEL_XOUT_H 0x3B
#define HAMSTRONE_CONFIG_MPU6050_ACCEL_YOUT_H 0x3D
#define HAMSTRONE_CONFIG_MPU6050_ACCEL_ZOUT_H 0x3F
#define HAMSTRONE_CONFIG_MPU6050_GYRO_XOUT_H 0x43
#define HAMSTRONE_CONFIG_MPU6050_GYRO_YOUT_H 0x45
#define HAMSTRONE_CONFIG_MPU6050_GYRO_ZOUT_H 0x47

#define HAMSTRONE_CONFIG_MPU6050_ACCEL_COEFFICIENT 16384.0
#define HAMSTRONE_CONFIG_MPU6050_GYRO_COEFFICIENT 131.0
#define HAMSTRONE_CONFIG_MPU6050_GYRO_TIMEDELTA 0.004
#define HAMSTRONE_CONFIG_RADIAN_TO_ANGLE 57.2957795131
#define HAMSTRONE_CONFIG_COMPLEMENTARY_FILTER_COEFFICIENT 0.96

#define HAMSTRONE_CONFIG_ISSTANDALONE

#endif