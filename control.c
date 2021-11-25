#include "include/control.h"

// PID parameters
double kP[PID_DIMENSION] = {0.2, 0.2};
double kI[PID_DIMENSION] = {0.4, 0.4};
double kD[PID_DIMENSION] = {0.1, 0.1};

// Kalman parameters
double H[4 * 4] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
double Q[4 * 4] = {0, -0.4000, -0.6500, -0.4000, 0.4000, 0, 0.4000, -0.6500, 0.6500, -0.4000, 0, 0.4000, 0.4000, 0.6500, -0.4000, 0};
double R[4 * 4] = {0.01085934229, 0, 0, 0, 0, 0.00052932196, 0, 0, 0, 0, 0.00052932196, 0, 0, 0, 0, 0.01085934229};
double x[4 * 1] = {1, 0, 0, 0};
double P[4 * 4] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
double q_mag[4 * 1], q_acc[4 * 1], z[4 * 1], A[4 * 4], xp[4 * 1], Pp[4 * 4], K[4 * 4];
double dt = 10.846803 / 1000;

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

/*
void updateKalman(mpu9250Data *data, double *result)
{
    // 마그네토 변환하기
    double lx = data->magX;
    double ly = data->magY;
    double lz = data->magZ;
    double gamma = (pow(lx, 2) + pow(ly, 2));

    if (data->magX >= 0)
    {
        q_mag[0] = sqrt(gamma + lx * sqrt(gamma)) / sqrt(2 * gamma);
        q_mag[1] = 0;
        q_mag[2] = 0;
        q_mag[3] = ly / (1.141 * sqrt(gamma + lx * sqrt(gamma)));
    }
    else
    {
        q_mag[0] = ly / (1.141 * sqrt(gamma - lx * sqrt(gamma)));
        q_mag[1] = 0;
        q_mag[2] = 0;
        q_mag[3] = sqrt(gamma - lx * sqrt(gamma)) / sqrt(2 * gamma);
    }

    double ax = data->accX / sqrt(pow(data->accX, 2) + pow(data->accY, 2) + pow(data->accZ, 2));
    double ay = data->accY / sqrt(pow(data->accY, 2) + pow(data->accY, 2) + pow(data->accZ, 2));
    double az = data->accZ / sqrt(pow(data->accZ, 2) + pow(data->accY, 2) + pow(data->accZ, 2));
    //%       쿼터니언으로 바꾸기(가속도 -> 쿼터니언)
    //%   z = [cos(phi/2)*cos(theta/2); sin(phi/2)*cos(theta/2);  cos(phi/2)*sin(theta/2); (- sin(phi/2)*sin(theta/2))];
    q_acc[0] = sqrt(0.5 * (az + 1));
    q_acc[1] = -ay / (2 * sqrt(0.5 * (az + 1)));
    q_acc[2] = ax / (2 * sqrt(0.5 * (az + 1)));
    q_acc[3] = 0;

    //%        q_acc_mag = [0;0;0;0];
    //        % 엑셀과 마그네토 쿼터니언 곱으로 z 만들기
    memset(z, 0, sizeof(z));
    quat_mult(q_acc[1], q_acc[2], q_acc[3], q_acc[4], q_mag[1], q_mag[2], q_mag[3], q_mag[4], z);

    //%   gyro_quat = angle2quat(gyro(i,3)*dt,gyro(i,2)*dt, gyro(i,1)*dt)
    double p = data->gyroX - 0.0248;
    double q = data->gyroY + 0.0316;
    double r = data->gyroZ - 0.0225;

    double temp1[4 * 4] = {0, -p, -q, -r, p, 0, r, -q, q, -r, 0, p, r, q, -p, 0};

    scale(temp1, dt * 0.5, 4, 4);

    double temp2[4 * 4] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    add(temp1, temp2, A, 4, 4, 4); //A=블라블라

    mul(A, x, false, xp, 4, 4, 1);

    double temp3[4 * 4];
    double temp4[4 * 4];

    mul(A, P, false, temp3, 4, 4, 4); //Pp = (A*P*A)T + Q;
    mul(temp3, A, false, temp4, 4, 4, 4);
    tran(temp4, 4, 4);
    add(temp4, Q, Pp, 4, 4, 4);

    mul(Pp, H, false, temp3, 4, 4, 4); //K = Pp*(H)T*inv(H*Pp*(H)T + R); //역행렬
    double temp5[4 * 4];
    double temp6[4 * 4];
    double temp7[4 * 4];
    mul(H, Pp, false, temp4, 4, 4, 4);
    mul(temp4, H, false, temp5, 4, 4, 4);
    add(temp5, R, temp6, 4, 4, 4);
    inv(temp6, 4);
    add(temp3, temp6, K, 4, 4, 4);

    double temp8[4 * 1]; //x = xp + K*(z - H*xp);     % x = [ q1 q2 q3 q4 ]
    double temp9[4 * 1];
    double temp10[4 * 1];
    mul(H, xp, false, temp8, 4, 4, 1);
    sub(z, temp8, temp9, 4, 1, 1);
    mul(K, temp9, false, temp10, 4, 4, 1);
    add(xp, temp10, x, 4, 1, 1);

    scale(x, 1 / norm(x, 4, "2"), 4, 1); //벡터 인자들 다 제곱해서 더해서 루트 scale:행렬 상수곱?

    mul(K, H, false, temp5, 4, 4, 4); // P = Pp - K*H*Pp;
    mul(temp5, Pp, false, temp6, 4, 4, 4);
    sub(Pp, temp6, P, 4, 4, 4);

    result[0] = atan2(2 * (x[2] * x[3] + x[0] * x[1]), 1 - 2 * (pow(x[1], 2) + pow(x[2], 2))); //roll
    result[1] = -asin(2 * (x[1] * x[3] - x[0] * x[2]));                                        //pitch
    result[2] = atan2(2 * (x[1] * x[0] * x[3]), 1 - 2 * (pow(x[2], 2) + pow(x[3], 2)));        //yaw
}
*/

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