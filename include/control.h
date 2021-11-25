#ifndef HAMSTRONE_CONTROL_H
#define HAMSTRONE_CONTROL_H

#include "mpu9250.h"
#define PID_DIMENSION 2
#define TIMEDELTA 0.01

typedef struct _Kalman_t
{
    double Q_angle;
    double Q_bias;
    double R_measure;
    double angle;
    double bias;
    double P[2][2];
} Kalman_t;

//void updateKalman(mpu9250Data *data, double *result);
void updatePID(double AngX, double AngY, double *result);
void updateComplimentary(mpu9250Data *data, double *result);
void updateKalman(mpu9250Data *data, double *result);

#endif
