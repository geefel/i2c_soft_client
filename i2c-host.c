#include "i2c-host.h"
#include <util/delay.h>
#include "pin.h"

void setupI2cSoftHost() {
    setOutput(SCL_PIN);
    setPin(SCL_PIN);
    setOutput(SDA_PIN);
}

void startI2c() {
    _delay_loop_1(25 + DLY_ADD);
    setOutput(SDA_PIN);
    setPin(SCL_PIN);
    _delay_loop_1(10);
    clrPin(SDA_PIN);
    _delay_loop_1(16 + DLY_ADD);
    clrPin(SCL_PIN);
}

void stopI2c() {
    setOutput(SDA_PIN);
    clrPin(SDA_PIN);
    _delay_loop_1(26 + DLY_ADD);
    setPin(SCL_PIN);
    _delay_loop_1(26 + DLY_ADD);
    setPin(SDA_PIN);
}

void sendACK() {
    _delay_loop_1(5 + DLY_ADD / 2);
    setOutput(SDA_PIN);
    clrPin(SDA_PIN);
    _delay_loop_1(17 + DLY_ADD / 2);
    
    setPin(SCL_PIN);
    _delay_loop_1(26 + DLY_ADD);;
    clrPin(SCL_PIN);
}

void sendNACK() {
    _delay_loop_1(5 + DLY_ADD / 2);
    setOutput(SDA_PIN);
    setPin(SDA_PIN);
    _delay_loop_1(17 + DLY_ADD / 2);
    
    setPin(SCL_PIN);
    _delay_loop_1(26 + DLY_ADD);
    clrPin(SCL_PIN);
}

uint8_t readACK() {
    uint8_t ack;
    _delay_loop_1(5 + DLY_ADD / 2);
    setInput(SDA_PIN);
    _delay_loop_1(17 + DLY_ADD / 2);
    setPin(SCL_PIN);
    if (getPin(SDA_PIN))
        ack = 0;
    else
        ack = 1;
    _delay_loop_1(26 + DLY_ADD);
    clrPin(SCL_PIN);
    return ack;
}

void sendI2cData(uint8_t data) {
	uint8_t maske = 0b10000000;
	setOutput(SDA_PIN);
	
	for (uint8_t b = 0; b < 8; b++) {
        _delay_loop_1(5 + DLY_ADD / 2);
        
        if ((data & maske) == maske)
            setPin(SDA_PIN);
        else
			clrPin(SDA_PIN);
        maske /= 2;
        
        _delay_loop_1(17 + DLY_ADD / 2);
		setPin(SCL_PIN);
        _delay_loop_1(26 + DLY_ADD);
        clrPin(SCL_PIN);
	}
}

uint8_t getI2cData() {
    uint8_t maske = 0b10000000;
    uint8_t erg = 0;
    setInput(SDA_PIN);
    for (uint8_t i = 0; i < 8; i++) {
        _delay_loop_1(25 + DLY_ADD);
        setPin(SCL_PIN);
        _delay_loop_1(5 + DLY_ADD / 2);
        
        if (getPin(SDA_PIN))
            erg += maske;
        maske /= 2;
        
        _delay_loop_1(20 + DLY_ADD / 2);
        clrPin(SCL_PIN);
    }
    return erg;
}

