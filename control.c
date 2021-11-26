#include "include/control.h"

// PID parameters
double kP[PID_DIMENSION] = {3, 3};
double kI[PID_DIMENSION] = {2, 2};
double kD[PID_DIMENSION] = {0.5, 0.5};


void updatePID(double AngX, double AngY, double *result)
{
    double degree[PID_DIMENSION] = {
        0.0,
    };
    degree[0] = AngX;
    degree[1] = AngY;

    static double prevInput[PID_DIMENSION] = {
        0.0,
    };
    static double controlI[PID_DIMENSION] = {
        0.0,
    };
    double controlP[PID_DIMENSION], controlD[PID_DIMENSION], dInput[PID_DIMENSION], error[PID_DIMENSION], desired[PID_DIMENSION] = {
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

        result[i] = controlP[i] + controlI[i] + controlD[i];
    }
}

//쿼터니언 곱
void quat_mult(double a_1, double a_2, double a_3, double a_4, double b_1, double b_2, double b_3, double b_4, double *result)
{
    result[0] = a_1 * b_1 - a_2 * b_2 - a_3 * b_3 - a_4 * b_4;
    result[1] = a_1 * b_2 + a_2 * b_1 + a_3 * b_4 - a_4 * b_3;
    result[2] = a_1 * b_3 - a_2 * b_4 + a_3 * b_1 + a_4 * b_2;
    result[3] = a_1 * b_4 + a_2 * b_3 - a_3 * b_2 + a_4 * b_1;
}

void updateComplimentary(mpu9250Data *data, double *result)
{
    static double xsq, ysq, zsq, accangx, accangy, gyangx, gyangy, gyangz, filangx, filangy, dgz;
    xsq = pow(data->accX, 2);
    ysq = pow(data->accY, 2);
    zsq = pow(data->accZ, 2);
    accangx = atan(data->accY / sqrt(xsq + zsq)) * HAMSTRONE_CONFIG_RADIAN_TO_ANGLE;
    accangy = atan(-1 * data->accX / sqrt(ysq + zsq)) * HAMSTRONE_CONFIG_RADIAN_TO_ANGLE;
    gyangx += data->gyroX * TIMEDELTA;
    gyangy += data->gyroY * TIMEDELTA;
    dgz = data->gyroZ * TIMEDELTA;
    gyangz += abs(dgz) < 0.3 ? 0 : dgz; //cutoff yaw delta below 0.3 deg

    result[0] = accangx * HAMSTRONE_CONFIG_COMPLEMENTARY_FILTER_COEFFICIENT + (1 - HAMSTRONE_CONFIG_COMPLEMENTARY_FILTER_COEFFICIENT) * gyangx;
    result[1] = accangy * HAMSTRONE_CONFIG_COMPLEMENTARY_FILTER_COEFFICIENT + (1 - HAMSTRONE_CONFIG_COMPLEMENTARY_FILTER_COEFFICIENT) * gyangz;
    result[2] = gyangz;
}

/* 
Kalman filter implementation from https://github.com/leech001/MPU6050
(C) leech001
GPL v3 LICENSE
*/
Kalman_t KalmanX = {
    .Q_angle = 0.001f,
    .Q_bias = 0.003f,
    .R_measure = 0.03f};

Kalman_t KalmanY = {
    .Q_angle = 0.001f,
    .Q_bias = 0.003f,
    .R_measure = 0.03f,
};

double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, double dt)
{
    double rate = newRate - Kalman->bias;
    Kalman->angle += dt * rate;

    Kalman->P[0][0] += dt * (dt * Kalman->P[1][1] - Kalman->P[0][1] - Kalman->P[1][0] + Kalman->Q_angle);
    Kalman->P[0][1] -= dt * Kalman->P[1][1];
    Kalman->P[1][0] -= dt * Kalman->P[1][1];
    Kalman->P[1][1] += Kalman->Q_bias * dt;

    double S = Kalman->P[0][0] + Kalman->R_measure;
    double K[2];
    K[0] = Kalman->P[0][0] / S;
    K[1] = Kalman->P[1][0] / S;

    double y = newAngle - Kalman->angle;
    Kalman->angle += K[0] * y;
    Kalman->bias += K[1] * y;

    double P00_temp = Kalman->P[0][0];
    double P01_temp = Kalman->P[0][1];

    Kalman->P[0][0] -= K[0] * P00_temp;
    Kalman->P[0][1] -= K[0] * P01_temp;
    Kalman->P[1][0] -= K[1] * P00_temp;
    Kalman->P[1][1] -= K[1] * P01_temp;

    return Kalman->angle;
};

void updateKalman(mpu9250Data *data, double *result)
{
    static double compRes[3] = {0, 0, 0};
    updateComplimentary(data, compRes);
    result[0] = Kalman_getAngle(&KalmanY, compRes[0], data->gyroX, TIMEDELTA);
    result[1] = Kalman_getAngle(&KalmanX, compRes[1], data->gyroY, TIMEDELTA);
    result[2] = compRes[2];
}