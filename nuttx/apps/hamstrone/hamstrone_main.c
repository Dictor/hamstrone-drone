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


int HAMSTRONE_GLOBAL_TELEMERTY_PORT;
sem_t HAMSTRONE_GLOBAL_TELEMETRY_SEMAPHORE;

int hamstrone_main(int argc, FAR char *argv[]) {
  if (sem_init(&HAMSTRONE_GLOBAL_TELEMETRY_SEMAPHORE, 0, 1) < 0) {
    printf("fatal error: sem init fail\n");
    return 1;
  }
  HAMSTERTONGUE_SetWriteSemaphore(&HAMSTRONE_GLOBAL_TELEMETRY_SEMAPHORE);

  /* Initialize telemetry serial port */
  HAMSTRONE_GLOBAL_TELEMERTY_PORT = open(HAMSTRONE_CONFIG_PORT_PATH, O_RDWR);
  if (HAMSTRONE_GLOBAL_TELEMERTY_PORT < 0) {
    printf("fatal error: telem init fail\n");
    return 1;
  }

  /* Initialize telemetry value store */
  HAMSTRONE_InitValueStore(HAMSTRONE_CONFIG_VALUE_SIZE);

  /* Start tasks */
  const char* tskargv[] = {"1000000"};
  task_create("tskTransmitValue", 100, 2048, &tskTransmitValue, tskargv);
  task_create("tskUpdateValue", 100, 2048, &tskUpdateValue, NULL);

  /* Initialize complete */
  HAMSTERTONGUE_WriteAndFreeMessage(
    HAMSTRONE_GLOBAL_TELEMERTY_PORT, 
    HAMSTERTONGUE_NewMessage(HAMSTERTONGUE_MESSAGE_VERB_SIGNAL, HAMSTERTONGUE_MESSAGE_NOUN_SIGNAL_INITOK, 0)
  );
  while(1) {
    sched_yield(); // abandon this task
  };
}
