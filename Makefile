include $(APPDIR)/Make.defs

MAINSRC = hamstrone_main.c
CSRCS += hamster_tongue.c serialize.c task.c value_store.c gps.c pwm.c sensor.c mpu9250.c

PROGNAME  = hamstrone
PRIORITY  = SCHED_PRIORITY_DEFAULT
STACKSIZE = 2 * CONFIG_DEFAULT_TASK_STACKSIZE
MODULE    = $(CONFIG_HAMSTRONE)


include $(APPDIR)/Application.mk