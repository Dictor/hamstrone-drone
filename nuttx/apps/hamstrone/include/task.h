#ifndef HAMSTRONE_TASK_VALUE_H
#define HAMSTRONE_TASK_VALUE_H

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "hamstrone_main.h"
#include "hamster_tongue.h"
#include "config.h"
#include "serialize.h"
#include <stdlib.h>
#include <time.h>
#include "value_store.h"

int tskTransmitValue(int argc, char * argv []);
int tskUpdateValue(int argc, char *argv[]);

#endif