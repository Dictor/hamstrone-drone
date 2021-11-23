#ifndef HAMSTRONE_TASK_VALUE_H
#define HAMSTRONE_TASK_VALUE_H

/* Standard header */
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <mqueue.h>
#include <math.h>

/* Hamstrone header */
#include "hamstrone_main.h"
#include "hamster_tongue.h"
#include "config.h"
#include "serialize.h"
#include "value_store.h"
#include "gps.h"
#include "mpu9250.h"
#include "sensor.h"

int tskTransmitValue(int argc, char *argv[]);
int tskUpdateValue(int argc, char *argv[]);
int tskParsingGPS(int argc, char *argv[]);
#define MPU9250_GYRO_TIMEDELTA 0.004

#endif