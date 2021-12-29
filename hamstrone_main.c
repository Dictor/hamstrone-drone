/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "include/hamstrone_main.h"
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * hello_main
 ****************************************************************************/

int HAMSTRONE_GLOBAL_TELEMETRY_PORT, HAMSTRONE_GLOBAL_SERIAL_PORT, HAMSTRONE_GLOBAL_I2C_PORT, HAMSTRONE_GLOBAL_MOTOR_PWM, HAMSTRONE_GLOBAL_SPI_PORT;
struct pwm_info_s *HAMSTRONE_GLOBAL_MOTOR_PWM_INFO;
sem_t HAMSTRONE_GLOBAL_TELEMETRY_SEMAPHORE;

int hamstrone_main(int argc, FAR char *argv[])
{
  /* Initailize telemetry serial port lock semaphore */
  if (sem_init(&HAMSTRONE_GLOBAL_TELEMETRY_SEMAPHORE, 0, 1) < 0)
  {
    printf("fatal error: sem init fail\n");
    return -1;
  }
  HAMSTERTONGUE_SetWriteSemaphore(&HAMSTRONE_GLOBAL_TELEMETRY_SEMAPHORE);

  /* Initialize telemetry serial port */
  HAMSTRONE_GLOBAL_TELEMETRY_PORT = open(HAMSTRONE_CONFIG_SERIALPORT1_PATH, O_RDWR);
  if (HAMSTRONE_GLOBAL_TELEMETRY_PORT < 0)
  {
    printf("fatal error: telem port init fail\n");
    return -1;
  }
  HAMSTERTONGUE_SetDefaultFile(HAMSTRONE_GLOBAL_TELEMETRY_PORT);

  /* nsh initialize for mounting i2c device */
  nsh_initialize();

  /* Initialize devices */
  HAMSTRONE_GLOBAL_I2C_PORT = open(HAMSTRONE_CONFIG_I2CPORT1_PATH, O_RDONLY);
  if (HAMSTRONE_GLOBAL_I2C_PORT < 0)
  {
    int currentErrno = errno;
    HAMSTERTONGUE_InitFailf(
        "%s,i2c,errno=%s=%d",
        HAMSTRONE_CONFIG_I2CPORT1_PATH, strerror(currentErrno), currentErrno);
    return -1;
  }

  HAMSTRONE_GLOBAL_SERIAL_PORT = open(HAMSTRONE_CONFIG_SERIALPORT2_PATH, O_RDWR);
  if (HAMSTRONE_GLOBAL_SERIAL_PORT < 0)
  {
    int currentErrno = errno;
    HAMSTERTONGUE_InitFailf(
        "%s,serial_port,errno=%s=%d",
        HAMSTRONE_CONFIG_SERIALPORT2_PATH, strerror(currentErrno), currentErrno);
    return -1;
  }

  HAMSTRONE_GLOBAL_SPI_PORT = open(HAMSTRONE_CONFIG_SPIPORT1_PATH, O_RDWR);
  if (HAMSTRONE_GLOBAL_SPI_PORT < 0)
  {
    int currentErrno = errno;
    HAMSTERTONGUE_InitFailf(
        "%s,spi_port,errno=%s=%d",
        HAMSTRONE_CONFIG_SPIPORT1_PATH, strerror(currentErrno), currentErrno);
    return -1;
  }

  HAMSTRONE_GLOBAL_MOTOR_PWM = open(HAMSTRONE_CONFIG_TIMER1PWM_PATH, O_RDONLY);
  if (HAMSTRONE_GLOBAL_MOTOR_PWM < 0)
  {
    int currentErrno = errno;
    HAMSTERTONGUE_InitFailf(
        "%s,pwm_port,errno=%s=%d",
        HAMSTRONE_CONFIG_TIMER1PWM_PATH, strerror(currentErrno), currentErrno);
    return -1;
  }
  struct pwm_info_s info;
  HAMSTRONE_GLOBAL_MOTOR_PWM_INFO = &info;
  InitMotor(HAMSTRONE_GLOBAL_MOTOR_PWM_INFO);

  tf_rt_initialize_context(NULL, NULL, 128);

  /* Initialize telemetry value store */
  HAMSTRONE_InitValueStore(HAMSTRONE_CONFIG_VALUE_SIZE);

  /* Start tasks */
  task_create("tskTransmitValue", 100, 2048, &tskTransmitValue, NULL);
  task_create("tskUpdateValue", 100, 2048, &tskUpdateValue, NULL);
  //task_create("tskParsingGPS", 100, 14336, &tskParsingGPS, NULL);

  /* Initialize complete */
  HAMSTERTONGUE_WriteAndFreeMessage(
      HAMSTRONE_GLOBAL_TELEMETRY_PORT,
      HAMSTERTONGUE_NewMessage(HAMSTERTONGUE_MESSAGE_VERB_SIGNAL, HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_INITOK, 0));
  while (1)
  {
    sched_yield(); // abandon this task
  };
}
