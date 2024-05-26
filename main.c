#include <stdio.h>
#include <util/delay.h>
#include "uart_hard.h"
#include "printS.h"
//#include "pin.h"
#include "i2c-host.h"
//#include "i2c-client.h"

#define test2

int main() {
    initUartHW(500000UL);
	setupI2cSoftHost();
    uint8_t adr = 2;
	_delay_ms(100);
    uint8_t data[4] = {adr, 0b10000000, 0b00000100, 0b00000100};  //10, 231, 129, n
	data[0] = adr;
	while(1) {
#ifdef test1
        startI2c();
        sendI2cData(data[0] + 0);
        readACK();
        sendI2cData(data[1]);
        readACK();
        sendI2cData(data[2]);
        readACK();
        sendI2cData(data[3]);
        sendNACK();
        stopI2c();
        data[3] *= 2;
        if (data[3] == 0)
            data[3] = 1;
        data[2]--;
        data[1]++;
        
#endif
#ifdef test2
        //getDataFromClient(data, 4);
        startI2c();
        sendI2cData(data[0] + 1);
        readACK();
        data[1] = getI2cData();
        sendACK();
        data[2] = getI2cData();
        sendACK();
        data[3] = getI2cData();
        sendNACK();
        stopI2c();
        printSeriel("%d %d %d %d\n", data[0], data[1], data[2], data[3]);
        
#endif
        _delay_ms(1000);
	}	
	return 0;
}
