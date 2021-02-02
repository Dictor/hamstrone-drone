#ifndef HAMSTRONE_MAIN_H
#define HAMSTRONE_MAIN_H

#include <nuttx/config.h>
#include <nuttx/sched.h>
#include <nshlib/nshlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "hamster_tongue.h"
#include "config.h"
#include "task.h"
#include "value_store.h"
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <mqueue.h>

extern int HAMSTRONE_GLOBAL_TELEMETRY_PORT;
extern int HAMSTRONE_GLOBAL_IMU_PORT;
extern sem_t HAMSTRONE_GLOBAL_TELEMETRY_SEMAPHORE;
int hamstrone_main(int argc, char *argv[]);
#endif