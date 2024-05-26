#ifndef I2S_HOST_H
#define I2S_HOST_H

#include <stdint.h>
#include "pin.h"

#define SDA_PIN PINDEF(D, 5)
#define SCL_PIN PINDEF(D, 6)

#define DLY_ADD 8  //DLY_ADD = 0: 100 kHz i2c-clock, 1: 3 F_CPU-Cyclen mehr, 2: 6 F_CPU-Cyclen mehr usw. 

void setupI2cSoftHost();
void startI2c();
void stopI2c();
void sendACK();
void sendNACK();
uint8_t readACK();
uint8_t getI2cData();
void sendI2cData(uint8_t data);

#endif	//I2S_HOST_H