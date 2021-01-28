#ifndef HAMSTRONE_TASK_VALUE_H
#define HAMSTRONE_TASK_VALUE_H

/* Standard header */
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

/* Hamstrone header */
#include "hamstrone_main.h"
#include "hamster_tongue.h"
#include "config.h"
#include "serialize.h"
#include "value_store.h"

/* Nuttx header */
#include "nuttx/i2c/i2c_master.h"

int tskTransmitValue(int argc, char * argv []);
int tskUpdateValue(int argc, char *argv[]);
int I2CWriteSingle(int fd, uint16_t addr, uint8_t regaddr, uint8_t value);
int I2CReadWriteSingle(int fd, uint16_t addr, uint8_t regaddr, uint8_t *buf);

#endif