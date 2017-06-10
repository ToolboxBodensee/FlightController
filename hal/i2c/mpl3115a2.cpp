#include "mpl3115a2.hpp"

Mpl3115a2::Mpl3115a2(I2C &_i2c, uint8_t addr) : i2c(_i2c){
    this->addr = addr;

    // Set to Altimeter and Oversampling
    char cmd[] = {0x26, 0xB8}; //CTRL-Reg, Oversampling x128/Altimeter
    i2c.write(addr, cmd, 2);


    // Enable Data Ready Flags
    cmd[0] = 0x13;
    cmd[1] = 0b111;
    i2c.write(addr, cmd, 2);

    // Set active
    cmd[0] = 0x26;
    cmd[1] = 0xB9;
    i2c.write(addr, cmd, 2);
}

uint8_t Mpl3115a2::isReady(){
    char cmd[] = {0};
    char res[1];

    i2c.write(addr, cmd, 1, true);
    i2c.read(addr, res, 1);

    return res[0] & 0b1<<3;
}

// CHANGED
uint8_t Mpl3115a2::isAvailable(){
    char cmd[] = {0x0C};
    char res[1];

    i2c.write(addr, cmd, 1);
    i2c.read(addr, res, 1, true);

    return res[0] == 0xC4;
}

float Mpl3115a2::getAltitude(){
    char cmd[] = {1};
    char res[3];

    i2c.write(addr, cmd, 1, true);
    i2c.read(addr, res, 3);

    float csb = (res[3]>>4)/16.0;

    return (res[0] << 8 | res[1]);
}
