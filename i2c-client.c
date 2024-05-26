    /*
     nach START kommt erste fallende Flanke von SCL, Master output, Slave input
Master                          Slave                               Slave 1. Runde   Slave 2.Runde  Slave 3. Runde
SCL Low,  SDA output, SDA set   SDA input                   bit 1
SCL High,                       SDA read                                             start_ISR_SDA() Check auf ENDE
SCL Low,  SDA set                                           bit 2                    stop_ISR_SDA()
SCL High,                       SDA read
SCL Low,  SDA set                                           bit 3
SCL High,                       SDA read
SCL Low,  SDA set                                           bit 4
SCL High,                       SDA read
SCL Low,  SDA set                                           bit 5
SCL High,                       SDA read
SCL Low,  SDA set                                           bit 6
SCL High,                       SDA read
SCL Low,  SDA set                                           bit 7
SCL High,                       SDA read
SCL Low,  SDA set                                           bit 8
SCL High,                       SDA read
SCL Low,  SDA input             SDA output, SDA Low/High    ACK/NACK    check r/w   
SCL High, waite for SDA read


     */


#include <avr/interrupt.h>
#include <util/delay.h>
#include "pin.h"
#include "i2c-client.h"
#include "printS.h"

//ADR = Adresse angleichen
#define ADR 1

//SDA_PIN, SCL_PIN, PCINTn, PCINTn_vect, PCMSKn, PCIFn angleichen

#define SDA_PIN PINDEF(D, 5)        //PCINT21, PCINT2_vect, PCMSK2, PCIF2
#define SCL_PIN PINDEF(D, 6)        //PCINT22, PCINT2_vect, PCMSK2, PCIF2

#define PCINT_VECT                  PCINT2_vect
#define PCINT_SCL_ON                PCMSK2 |= (1 << PCINT22)
#define PCINT_SCL_OFF               PCMSK2 &= ~(1 << PCINT22) 
#define PCINT_SDA_ON                PCMSK2 |= (1 << PCINT21)
#define PCINT_SDA_OFF               PCMSK2 &= ~(1 << PCINT21)
#define PCINT_SDA_SCL_ENABLE        PCICR |= (1 << PCIE2)

enum {
start = 0,
bit_1_R_L,
bit_1_R_H,
bit_2_R_L,
bit_2_R_H,
bit_3_R_L,
bit_3_R_H,
bit_4_R_L,
bit_4_R_H,
bit_5_R_L,
bit_5_R_H,
bit_6_R_L,
bit_6_R_H,
bit_7_R_L,
bit_7_R_H,
bit_8_R_L,
bit_8_R_H,
ack_R_L,
ack_R_H,
bit_1_S_L,
bit_1_S_H,
bit_2_S_L,
bit_2_S_H,
bit_3_S_L,
bit_3_S_H,
bit_4_S_L,
bit_4_S_H,
bit_5_S_L,
bit_5_S_H,
bit_6_S_L,
bit_6_S_H,
bit_7_S_L,
bit_7_S_H,
bit_8_S_L,
bit_8_S_H,
ack_S_L,
ack_S_H,
stop,
getStop
};

static volatile uint8_t isDat;
static volatile uint8_t* data;
static volatile uint8_t maxByte;

#define LED_PIN PINDEF(D, 2)
void led() {
    setPin(LED_PIN);
    clrPin(LED_PIN);
}
#define LED led();

void setI2CStop();

void setupI2cSoftClient() {
    uint8_t trash[2];
    setInput(SDA_PIN);
 	setInput(SCL_PIN);
 	setOutput(LED_PIN);
    clrPin(LED_PIN);
    cli();
    PCINT_SDA_SCL_ENABLE;
    PCINT_SDA_ON;
    PCINT_SCL_OFF;
    sei();
    getDatafromHost(trash, 2);     //erste Aufnahme ist für die Tonne
}

void getDatafromHost(uint8_t *dat, uint8_t maxB) {
	data = dat; 
	maxByte = maxB;
    isDat = 0;
}

void sendDatatoHost(uint8_t *dat, uint8_t maxB) {
	data = dat;
	maxByte = maxB;
	isDat = 0;
}

uint8_t isDataH() {
    return isDat; 
}

void setI2CStop() {
    clrPin(SDA_PIN);
    setOutput(SDA_PIN);
    setPin(SDA_PIN);
    setInput(SDA_PIN);
}

/*__attribute__((naked)) */
ISR(PCINT_VECT) {
    static volatile uint8_t state = start;
    static volatile uint8_t dataByteNr;

    switch (state) {
        case start:
            if (getPin(SCL_PIN) == 1) {
                if (getPin(SDA_PIN) == 0) {  //Wechsel auf low -> start
                    PCINT_SDA_OFF;  //siehe case stop:
                    PCINT_SCL_ON;
                    dataByteNr = 0;
                    state = bit_1_R_L;
                }
                else                        //Wechsel auf high -> stop
                    state = start;
            }
            break;

//empfangen===============================================
        case bit_1_R_L:
            setInput(SDA_PIN);
            data[dataByteNr] = 0;
            state = bit_1_R_H;
            break;
        case bit_1_R_H:
            if (getPin(SDA_PIN))
                data[dataByteNr] += 128;
            state = bit_2_R_L;
            break;
            
        case bit_2_R_L:
            state = bit_2_R_H;
            break;
        case bit_2_R_H:
            if (getPin(SDA_PIN))
                data[dataByteNr] += 64;
            state = bit_3_R_L;
            break;
            
        case bit_3_R_L:
            state = bit_3_R_H;
            break;
        case bit_3_R_H:
            if (getPin(SDA_PIN))
                data[dataByteNr] += 32; 
            state = bit_4_R_L;
            break;
            
         case bit_4_R_L:
            state = bit_4_R_H;
            break;
        case bit_4_R_H:
            if (getPin(SDA_PIN))
                data[dataByteNr] += 16;
            state = bit_5_R_L;
            break;   
            
       case bit_5_R_L:
            state = bit_5_R_H;
            break;
        case bit_5_R_H:
            if (getPin(SDA_PIN))
                data[dataByteNr] += 8;
            state = bit_6_R_L;
            break;
        
        case bit_6_R_L:
            state = bit_6_R_H;
            break;
        case bit_6_R_H:
            if (getPin(SDA_PIN))
                data[dataByteNr] += 4;
            state = bit_7_R_L;
            break;
        
        case bit_7_R_L:
            state = bit_7_R_H;
            break;
        case bit_7_R_H:
            if (getPin(SDA_PIN))
                data[dataByteNr] += 2;
            state = bit_8_R_L;
            break;
        
        case bit_8_R_L:
            state = bit_8_R_H;
            break;
        case bit_8_R_H:
            if (getPin(SDA_PIN))
                data[dataByteNr] += 1;
            state = ack_R_L; 
            break;
 

        case ack_R_L:
            if ((data[0] / 2) == ADR) {
                setOutput(SDA_PIN);              //ACK senden
                clrPin(SDA_PIN);
                state = ack_R_H;
            }
            else {
                setOutput(SDA_PIN);              //NACK senden
                setPin(SDA_PIN);
                state = stop;
                isDat = 0;
            }
            break;
            
        case ack_R_H:
            dataByteNr++;
            if ((data[0] & 1) == 1)
                state = bit_1_S_L;
            else
                state = bit_1_R_L;
                
            if (dataByteNr == maxByte) {
                isDat = 1;
                state = stop;
            }
            break;
            
//senden========================================================
        case bit_1_S_L:
            setOutput(SDA_PIN);
            if ((data[dataByteNr] & 128) == 128)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_1_S_H;
            break;
        case bit_1_S_H:
            state = bit_2_S_L;
            break;
        
        case bit_2_S_L:
            if ((data[dataByteNr] & 64) == 64)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_2_S_H;
            break;
        case bit_2_S_H:
            state = bit_3_S_L;
            break;
        
        case bit_3_S_L:
            if ((data[dataByteNr] & 32) == 32)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_3_S_H;
            break;
        case bit_3_S_H:
            state = bit_4_S_L;
            break;
            
        case bit_4_S_L:
            if ((data[dataByteNr] & 16) == 16)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_4_S_H;
            break;
        case bit_4_S_H:
            state = bit_5_S_L;
            break;
        
        case bit_5_S_L:
            if ((data[dataByteNr] & 8) == 8)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_5_S_H;
            break;
        case bit_5_S_H:
            state = bit_6_S_L;
            break;
        
        case bit_6_S_L:
            if ((data[dataByteNr] & 4) == 4)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_6_S_H;
            break;
        case bit_6_S_H:
            state = bit_7_S_L;
            break;
        
        case bit_7_S_L:
            if ((data[dataByteNr] & 2) == 2)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_7_S_H;
            break;
        case bit_7_S_H:
            state = bit_8_S_L;
            break;
        
        case bit_8_S_L:
            if ((data[dataByteNr] & 1) == 1)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_8_S_H;
            break;
        case bit_8_S_H:
            state = ack_S_L;
            break;

        case ack_S_L:
            setInput(SDA_PIN);
            state = ack_S_H;
            break;
        case ack_S_H:
            state = bit_1_S_L;
            dataByteNr++;
            if (dataByteNr == maxByte) {
                isDat = 1;
                state = stop;
            }
            break; 
     
        case stop:
            setInput(SDA_PIN);
            PCINT_SCL_OFF;  //siehe case start:
            PCINT_SDA_ON;
            state = start;
            break;
            
        default:
            break;
    }
    



}
