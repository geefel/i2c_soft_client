#ifndef I2S_SOFT_CLIENT_H
#define I2S_SOFT_CLIENT_H

#include <stdint.h>
#include "pin.h"

void setupI2cSoftClient();
void getDatafromHost(uint8_t *dat);
void sendDatatoHost(uint8_t *dat);
uint8_t isDataH();

#endif	//I2S_SOFT_CLIENT_H
