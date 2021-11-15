#ifndef HAMSTRONE_SENSOR_VALUE_H
#define HAMSTRONE_SENSOR_VALUE_H

/* Hamstrone header */
#include "hamstrone_main.h"
#include "hamster_tongue.h"
#include "config.h"
#include "serialize.h"
#include "value_store.h"
#include "mpu9250.h"

/* Nuttx header */
#include "nuttx/i2c/i2c_master.h"
#include "nuttx/spi/spi_transfer.h"
#include "nuttx/spi/spi.h"

int SPIReadSingle(int fd, enum spi_mode_e mode, uint8_t regaddr, uint8_t *buf);
int SPIRead(int fd, enum spi_mode_e mode, uint8_t regaddr, uint8_t recieveBytes, uint8_t **val);
int SPIWriteSingle(int fd, enum spi_mode_e mode, uint8_t regaddr, uint8_t value);
int I2CWriteSingle(int fd, uint16_t addr, uint8_t regaddr, uint8_t value);
int I2CReadSingle(int fd, uint16_t addr, uint8_t regaddr, uint8_t *buf);
int TCA9548SetChannel(int fd, uint8_t chan);

#endif