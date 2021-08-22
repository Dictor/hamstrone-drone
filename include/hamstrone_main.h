#ifndef HAMSTRONE_MAIN_H
#define HAMSTRONE_MAIN_H

/* Standard headers */
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <mqueue.h>
#include <stdio.h>
#include <fcntl.h>

/* Nuttx headers */
#include <nuttx/config.h>
#include <nuttx/sched.h>
#include <nshlib/nshlib.h>
#include "nuttx/timers/pwm.h"

/* Hamstrone headers */
#include "hamster_tongue.h"
#include "config.h"
#include "task.h"
#include "value_store.h"
#include "pwm.h"

#define HAMSTERTONGUE_InitFailf(...)                    \
    HAMSTERTONGUE_WriteAndFreeMessage(                  \
        HAMSTRONE_GLOBAL_TELEMETRY_PORT,                \
        HAMSTERTONGUE_NewFormatStringMessage(           \
            HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,          \
            HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_INITFAIL, \
            48, __VA_ARGS__));

extern int HAMSTRONE_GLOBAL_TELEMETRY_PORT;
extern int HAMSTRONE_GLOBAL_SERIAL_PORT;
extern int HAMSTRONE_GLOBAL_I2C_PORT;
extern int HAMSTRONE_GLOBAL_MOTOR_PWM;
extern int HAMSTRONE_GLOBAL_SPI_PORT;
extern struct pwm_info_s *HAMSTRONE_GLOBAL_MOTOR_PWM_INFO;
extern sem_t HAMSTRONE_GLOBAL_TELEMETRY_SEMAPHORE;
int hamstrone_main(int argc, char *argv[]);
#endif