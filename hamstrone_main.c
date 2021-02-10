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

int HAMSTRONE_GLOBAL_TELEMETRY_PORT, HAMSTRONE_GLOBAL_IMU_PORT;
sem_t HAMSTRONE_GLOBAL_TELEMETRY_SEMAPHORE;

int hamstrone_main(int argc, FAR char *argv[])
{
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
  nsh_initialize();

  /* Initialize i2c port */
  HAMSTRONE_GLOBAL_IMU_PORT = open(HAMSTRONE_CONFIG_I2CPORT1_PATH, O_RDONLY);
  if (HAMSTRONE_GLOBAL_IMU_PORT < 0)
  {
    int currentErrno = errno;
    HAMSTERTONGUE_WriteAndFreeMessage(
        HAMSTRONE_GLOBAL_TELEMETRY_PORT,
        HAMSTERTONGUE_NewFormatStringMessage(
            HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
            HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_INITFAIL,
            48,
            "%s,i2c,errno=%s=%d",
            HAMSTRONE_CONFIG_I2CPORT1_PATH, strerror(-currentErrno), currentErrno));
    return -1;
  }
  
  /* Initialize telemetry value store */
  HAMSTRONE_InitValueStore(HAMSTRONE_CONFIG_VALUE_SIZE);

  /* Initialize gps message queue */
  struct mq_attr mq_opt;
  mq_opt.mq_maxmsg = 5;
  mq_opt.mq_msgsize = 32;

  mqd_t mq = mq_open("/mqgps", O_RDWR | O_CREAT, 0777, &mq_opt);
  if (mq < 0) {
    int currentErrno = errno;
    HAMSTERTONGUE_WriteAndFreeMessage(
        HAMSTRONE_GLOBAL_TELEMETRY_PORT,
        HAMSTERTONGUE_NewFormatStringMessage(
            HAMSTERTONGUE_MESSAGE_VERB_SIGNAL,
            HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_INITFAIL,
            48,
            "/mqgps,errno=%s=%d",
            strerror(currentErrno), currentErrno));
    return -1;
  }
  
  /* Start tasks */
  task_create("tskTransmitValue", 100, 2048, &tskTransmitValue, NULL);
  task_create("tskUpdateValue", 100, 2048, &tskUpdateValue, NULL);
  task_create("tskParsingGPS", 100, 4096, &tskParsingGPS, NULL);

  /* Initialize complete */
  HAMSTERTONGUE_WriteAndFreeMessage(
      HAMSTRONE_GLOBAL_TELEMETRY_PORT,
      HAMSTERTONGUE_NewMessage(HAMSTERTONGUE_MESSAGE_VERB_SIGNAL, HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_INITOK, 0));
  while (1)
  {
    sched_yield(); // abandon this task
  };
}
