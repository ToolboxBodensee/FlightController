#include "mpl3115a2.hpp"

Mpl3115a2::Mpl3115a2(I2C &_i2c, uint8_t addr) : i2c(_i2c){
    this->addr = addr;

    // Set to Altimeter and Oversampling
    char cmd[] = {0x26, 0b10111000}; //CTRL-Reg, Oversampling x128/Altimeter
    i2c.write(addr, cmd, 2);


    // Enable Data Ready Flags
    cmd[0] = 0x13;
    cmd[1] = 0b111;
    i2c.write(addr, cmd, 2);

    // Set active
    cmd[0] = 0x26;
    cmd[1] = 0b10111001;
    i2c.write(addr, cmd, 2);
}

uint8_t Mpl3115a2::isReady(){
    char cmd[] = {0};
    char res[1];

    i2c.write(addr, cmd, 1);
    i2c.read(addr, res, 1, true);

    return res[0] & 0b1<<2; //CHANGED check wheter <<3 was better
}

float Mpl3115a2::getAltitude(){
    char cmd[] = {1};
    char res[3];

    i2c.write(addr, cmd, 1);
    i2c.read(addr, res, 3);

    return (res[0] << 24 | res[1] << 16  | res[2] << 8);
}
