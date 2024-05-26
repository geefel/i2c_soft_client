#ifndef I2CSOFTCLIENT_H
#define I2CSOFTCLIENT_H

#include <stdint.h>

void setupI2cSoftClient();
void getDatafromHost(uint8_t *dat, uint8_t maxB);
void sendDatatoHost(uint8_t *dat, uint8_t maxB);
uint8_t isDataH();

#endif // I2CSOFTCLIENT_H
