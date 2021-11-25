#ifndef HAMSTRONE_PWM_H
#define HAMSTRONE_PWM_H

/* Standard header */
#include <sys/ioctl.h>

/* Hamstrone header */
#include "hamstrone_main.h"
#include "config.h"

/* Nuttx header */
#include "nuttx/timers/pwm.h"

void InitMotor(struct pwm_info_s *info);
void PWMWriteAll(struct pwm_info_s *info, uint16_t perc1, uint16_t perc2, uint16_t perc3, uint16_t perc4);
void PWMWriteSingle(struct pwm_info_s *info, int index, uint8_t percentile);

#endif