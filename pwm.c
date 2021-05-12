#include "include/pwm.h"

void InitMotor(struct pwm_info_s *info) {
    info->frequency = 200;
    info->channels[0].channel = 1;
    info->channels[1].channel = 2;
    info->channels[2].channel = 3;
    info->channels[3].channel = 4;
    
    info->channels[0].duty = ESC_DUTY_MAX;
    info->channels[1].duty = ESC_DUTY_MAX;
    info->channels[2].duty = ESC_DUTY_MAX;
    info->channels[3].duty = ESC_DUTY_MAX;
    ioctl(HAMSTRONE_GLOBAL_MOTOR_PWM, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t)info));
    ioctl(HAMSTRONE_GLOBAL_MOTOR_PWM, PWMIOC_START, 0);
    usleep(500000);
    
    PWMWriteAll(info, 0, 0, 0, 0);
    usleep(500000);
}

void PWMWriteAll(struct pwm_info_s *info, uint8_t perc1, uint8_t perc2, uint8_t perc3, uint8_t perc4) {
    info->channels[0].duty = ESC_DUTY_MIN + perc1 * ESC_DUTY_PERCENTILE_STEP;
    info->channels[1].duty = ESC_DUTY_MIN + perc2 * ESC_DUTY_PERCENTILE_STEP;
    info->channels[2].duty = ESC_DUTY_MIN + perc3 * ESC_DUTY_PERCENTILE_STEP;
    info->channels[3].duty = ESC_DUTY_MIN + perc4 * ESC_DUTY_PERCENTILE_STEP;
    ioctl(HAMSTRONE_GLOBAL_MOTOR_PWM, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t)info));
}

void PWMWriteSingle(struct pwm_info_s *info, int index, uint8_t percentile) {
    if (percentile > 100) percentile = 100;
    info->channels[index].duty = ESC_DUTY_MIN + percentile * ESC_DUTY_PERCENTILE_STEP;
    ioctl(HAMSTRONE_GLOBAL_MOTOR_PWM, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t)info));
}