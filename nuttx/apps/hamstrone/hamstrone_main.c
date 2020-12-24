/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "include/hamstrone_main.h"
#include "include/config.h"
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * hello_main
 ****************************************************************************/

int HAMSTRONE_GLOBAL_TELEMERTY_PORT;
uint32_t* HAMSTRONE_GLOBAL_VALUE;


int main(int argc, FAR char *argv[])
{
  HAMSTRONE_GLOBAL_TELEMERTY_PORT = open(HAMSTRONE_CONFIG_PORT_PATH, O_RDWR);
  if (port < 0) {
    printf("fail to open port\n");
    return 1;
  }
  HAMSTRONE_GLOBAL_VALUE = malloc(sizeof(HAMSTRONE_CONFIG_VALUE_TYPE) * HAMSTRONE_CONFIG_VALUE_SIZE);
}
