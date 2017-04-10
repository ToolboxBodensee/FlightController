#include "srf02.hpp"

Srf02::Srf02(I2C &_i2c, uint8_t addr) : I2cSensor(_i2c, addr){}

void Srf02::startMeasurement(){
    char cmd[] = {0, 0x51}; // CMD Register, Range cm
    I2cSensor::i2c.write(addr, cmd, 2);
}

uint8_t Srf02::isReady(){
    /*
    * Read from Software Revision Register, is 255 while ranging
    */
    char cmd[] = {0};
    char res[1];

    i2c.write(addr, cmd, 1);
    i2c.read(addr, res, 1);

    return res[0] != 255;
}

uint16_t Srf02::readDistance(){
    char cmd[] = {2};
    char res[2];

    i2c.write(addr, cmd, 1);
    i2c.read(addr, res, 2);

    return res[0] << 8 | res[1];
}
