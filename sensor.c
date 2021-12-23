#include "include/sensor.h"

int TCA9548SetChannel(int fd, uint8_t chan)
{
    return I2CWriteRegisterSingle(fd, HAMSTRONE_CONFIG_I2C_ADDRESS_TCA9548, HAMSTRONE_CONFIG_TCA9548_CHAN, 1 << chan);
}

int SPIWriteSingle(int fd, enum spi_mode_e mode, uint8_t regaddr, uint8_t value)
{
    struct spi_sequence_s seq;
    struct spi_trans_s trans;
    uint8_t tx[4] = {regaddr, value, 0, 0};
    uint8_t rx[4] = {0};

    trans.delay = 0;
    trans.deselect = true;
    trans.nwords = 2;
    trans.txbuffer = tx;
    trans.rxbuffer = rx;

    seq.dev = SPIDEV_USER(0);
    seq.mode = mode;
    seq.nbits = 8;
    seq.ntrans = 1;
    seq.trans = &trans;
    seq.frequency = HAMSTRONE_CONFIG_SPIPORT1_FREQUENCY;
    int ret = ioctl(fd, SPIIOC_TRANSFER, &seq);
    return ret;
}

int SPIReadSingle(int fd, enum spi_mode_e mode, uint8_t regaddr, uint8_t *val)
{
    struct spi_sequence_s seq;
    struct spi_trans_s trans;
    uint8_t tx[4] = {regaddr, 0, 0, 0};
    uint8_t rx[4] = {0};

    trans.delay = 0;
    trans.deselect = true;
    trans.nwords = 2;
    trans.txbuffer = tx;
    trans.rxbuffer = rx;

    seq.dev = SPIDEV_USER(0);
    seq.mode = mode;
    seq.nbits = 8;
    seq.ntrans = 1;
    seq.trans = &trans;
    seq.frequency = HAMSTRONE_CONFIG_SPIPORT1_FREQUENCY;

    int ret = ioctl(fd, SPIIOC_TRANSFER, (unsigned long)((uintptr_t)&seq));
    *val = rx[1];
    return ret;
}

int SPIRead(int fd, enum spi_mode_e mode, uint8_t regaddr, uint8_t recieveBytes, uint8_t *val)
{
    struct spi_sequence_s seq;
    struct spi_trans_s trans;
    uint8_t *tx = malloc(recieveBytes + 1);
    uint8_t *rx = malloc(recieveBytes + 1);
    memset(tx, 0, recieveBytes + 1);
    memset(rx, 0, recieveBytes + 1);
    tx[0] = regaddr;

    trans.delay = 0;
    trans.deselect = true;
    trans.nwords = recieveBytes + 1;
    trans.txbuffer = tx;
    trans.rxbuffer = rx;

    seq.dev = SPIDEV_USER(0);
    seq.mode = mode;
    seq.nbits = 8;
    seq.ntrans = 1;
    seq.trans = &trans;
    seq.frequency = HAMSTRONE_CONFIG_SPIPORT1_FREQUENCY;

    int ret = ioctl(fd, SPIIOC_TRANSFER, (unsigned long)((uintptr_t)&seq));
    memcpy(val, rx + 1, sizeof(uint8_t) * recieveBytes);
    free(tx);
    free(rx);
    return ret;
}

int I2CWriteSingle(int fd, uint16_t addr, uint8_t value)
{
    struct i2c_msg_s msg[1];
    struct i2c_transfer_s trans;
    uint8_t rawbuf[2] = {value};

    msg[0].addr = addr;
    msg[0].flags = 0;
    msg[0].buffer = rawbuf;
    msg[0].length = 1;
    msg[0].frequency = 400000;

    trans.msgv = (struct i2c_msg_s *)msg;
    trans.msgc = 1;

    return ioctl(fd, I2CIOC_TRANSFER, &trans);
}

int I2CWriteRegisterSingle(int fd, uint16_t addr, uint8_t regaddr, uint8_t value)
{
    struct i2c_msg_s msg[1];
    struct i2c_transfer_s trans;
    uint8_t rawbuf[2] = {regaddr, value};

    msg[0].addr = addr;
    msg[0].flags = 0;
    msg[0].buffer = rawbuf;
    msg[0].length = 2;
    msg[0].frequency = 400000;

    trans.msgv = (struct i2c_msg_s *)msg;
    trans.msgc = 1;

    return ioctl(fd, I2CIOC_TRANSFER, &trans);
}

int I2CReadSingle(int fd, uint16_t addr, uint8_t regaddr, uint8_t *buf)
{
    struct i2c_msg_s msg[2];
    struct i2c_transfer_s trans;
    uint8_t regaddrbuf[1] = {regaddr};

    msg[0].addr = addr;
    msg[0].flags = 0;
    msg[0].buffer = regaddrbuf;
    msg[0].length = 1;
    msg[0].frequency = 400000;

    msg[1].addr = addr;
    msg[1].flags = I2C_M_READ;
    msg[1].buffer = buf;
    msg[1].length = 1;
    msg[1].frequency = 400000;

    trans.msgv = (struct i2c_msg_s *)msg;
    trans.msgc = 2;

    return ioctl(fd, I2CIOC_TRANSFER, &trans);
}

int I2CRead(int fd, uint16_t addr, uint8_t length, uint8_t *buf)
{
    struct i2c_msg_s msg[1];
    struct i2c_transfer_s trans;

    msg[0].addr = addr;
    msg[0].flags = I2C_M_READ;
    msg[0].buffer = buf;
    msg[0].length = length;
    msg[0].frequency = 400000;

    trans.msgv = (struct i2c_msg_s *)msg;
    trans.msgc = 1;

    return ioctl(fd, I2CIOC_TRANSFER, &trans);
}
