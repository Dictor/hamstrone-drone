#ifndef HAMSTRONE_MAIN_H
#define HAMSTRONE_MAIN_H

#include <nuttx/config.h>
#include <nuttx/sched.h>
#include <stdio.h>
#include <fcntl.h>
#include "hamster_tongue.h"
#include "config.h"
#include "task.h"
#include "value_store.h"
#include <semaphore.h>

extern int HAMSTRONE_GLOBAL_TELEMERTY_PORT;
extern sem_t HAMSTRONE_GLOBAL_TELEMETRY_SEMAPHORE;
int hamstrone_main(int argc, char *argv[]);
#endif