/*
 * SDA ist Datenleitung ATMEGA328 PC4 ATTinyx5 PB0
 * SCL ist Clockleitung ATMEGA328 PC5 ATTinyx5 PB2
 * R/W=1 -> Lesen; RW=0 -> Schreiben
 * MSB: Most Significant Bit
 * alle Datenpakete sind 9 bit lang: 8 Datenbit, 1 ACK
 * alle Adresspakete sind 9 bit lang: 7 Adressbit, 1 RW-bit, 1 ACK
 * lesen: R = 1
 * schreiben: W = 0
 * ACK: Slave setzt Leitung auf Low (SCL ist gerade high)
 * Adresse 0000 000 ist reserviert, hiermit werden alle Slaves angesprochen
 * Übertragung: START, Adresspaket, Datenpaket, STOP
 * START: SCL ist high, SDA wechselt nach low
 * STOP: SCL ist nach ACK-Takt wieder(!) auf high, SDA wechselt nach high
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "i2c-client.h"
#include "print.h"



//PCINT22
#define SDA_PIN                     PINDEF(D, 5)        //PCINT21
#define SCL_PIN                     PINDEF(D, 6)        //PCINT22

#define PCINT_VECT                  PCINT2_vect
#define PCINT_MASKE                 PCMSK2 |= (1 << PCINT22)
#define PCINT_FLAG_CLEAR            PCIFR &= ~(1 << PCIF2)
#define PCINT_CONTROLL_ENABLE       PCICR |= (1 << PCIE2)
#define PCINT_CONTROLL_DISABLE      PCICR &= ~(1 << PCIE2)

#define PCINT_SCL_ON                PCMSK2 |= (1 << PCINT22)
#define PCINT_SCL_OFF               PCMSK2 &= ~(1 << PCINT22) 
#define PCINT_SDA_ON                PCMSK2 |= (1 << PCINT21)
#define PCINT_SDA_OFF               PCMSK2 &= ~(1 << PCINT21)
#define PCINT_SDA_SCL_FLAG_CLEAR    PCIFR &= ~(1 << PCIF2)

#define ACK   1
#define STOP  0
#define READ  1 //vom Host aus gesehen!
#define WRITE 0 //vom Host aus gesehen!

#define LED_PIN PINDEF(D, 2)

static volatile uint8_t* data;
static volatile uint8_t bitNr = 0;
//static volatile uint8_t maske = 0;

static volatile uint8_t isDat = 0;

const uint8_t adrByte = 0;

void stop_ISR_SDA();
void start_ISR_SDA();
void stop_ISR_SLC();
void start_ISR_SCL();
void stopInterrupt();
void startInterrupt();
void clearInteruptFlag();
void getSendData();


void led() {
    setPin(LED_PIN);
    clrPin(LED_PIN);
}
#define LED led();

void setupI2cSoftClient() {
 	setInput(SDA_PIN);
 	setInput(SCL_PIN);
    startInterrupt();
 	sei();
    setOutput(LED_PIN);
    clrPin(LED_PIN);
    // data[0] = 1;
    // data[1] = 2;
    // data[2] = 4;
    // data[3] = 8;
}



void stopInterrupt() {
    PCINT_CONTROLL_DISABLE;
}

void startInterrupt() {
    PCINT_MASKE;
    PCINT_FLAG_CLEAR;
    PCINT_CONTROLL_ENABLE;
}

void clearInteruptFlag() {
    PCINT_FLAG_CLEAR;
}

void getDatafromHost(uint8_t *dat) {
	data = dat;
    isDat = 0;
}

void sendDatatoHost(uint8_t *dat) {
	data = dat;
	isDat = 0;
}

uint8_t isDataH() {
    return isDat; 
}
static volatile uint8_t count = 9;
static volatile uint8_t rw = WRITE;
static volatile uint8_t dataByteNr = 0;
static volatile uint8_t maske = 0b10000000;
ISR(PCINT_VECT) {

// static uint8_t ack = 0;
// static uint8_t nack = 0;
// static uint8_t nochmal = 1;

//

    if (rw == 0) 
    {
        if(getPin(SCL_PIN) == 1)                   //SCL HIGH
        {
            if (count > 1)
            {
                if (getPin(SDA_PIN))
                    data[dataByteNr] |= maske;
                else
                    data[dataByteNr] &= ~maske;
                maske >>= 1;
                count--;LED                //85 Cycles bis hier
            }
            else if (count == 1)
            {
                if (getPin(SDA_PIN) == 0)   //ACK
                {
                    count = 9;
                    maske = 0b10000000;
                    dataByteNr++;
                    rw = data[0] % 2;
                }
                else                        //NACK
                {
                    count = STOP;
                }
            }
    
            else if (count == STOP)
            {
                maske = 0b10000000;
                dataByteNr = 0;
                count = 9;
                isDat = 1;
                printS("%d %d %d %d\n", data[0], data[1], data[2], data[3]);               
            }
        }
    }


    else    //rw == READ
    {
        if(getPin(SCL_PIN) == 0)                   //SCL LOW
        {
            if (count > 1)                  //bit
            {
                setOutput(SDA_PIN);
                if ((data[dataByteNr] & maske) == maske)
                    setPin(SDA_PIN);
                else
                    clrPin(SDA_PIN);
                maske >>= 1;
                count--;
            }
            else if (count == 1)            //ACK
            {
                count = 9;
                maske = 0b10000000;
                dataByteNr++;
                
            }
            else if (count == STOP)
            {
                maske = 0b10000000;
                dataByteNr = 0;
                count = 9;
                isDat = 1;
                //rw = WRITE;
            }
        }
        else                                       //SCL HIGH
        {
            if (count == 1)
            {
                setInput(SDA_PIN);
                if (getPin(SDA_PIN))
                {
                    count = STOP;
                }

            }
        }
        
    }
    
}

//TODO: Es ist noch nicht auf Adresse geprüft
//ACK/NACK wird nicht überprüft, außer Ende der zu übertragenen Daten
//Zuerst bitNr == START: erster Low überhaupt, bitNr wird 1
//Dann immer SCL High, SCL Low
// ISR(PCINT_VECT) {
//     // if (rw == WRITE) 
//     // {
    
//         if(getPin(SCL_PIN) == 1)                   //SCL HIGH
//         {
//             if (bitNr < 9)
//             {
//                 if (getPin(SDA_PIN))
//                     data[dataByteNr] |= maske;
//                 else
//                     data[dataByteNr] &= ~maske;
//             }
            
//             else if (bitNr == 9)     //ACK-Abfrage
//             {
//                 if (getPin(SDA_PIN))    //NACK
//                     bitNr = NACK;
//                 // if ((data[0] & 0b00000001) == READ) {
//                 //     rw = READ;
//                     // bitNr = 1;
//                     // maske = 0b10000000;
//                 // }
//                 // else
//                 //     rw = WRITE;
//             }
              
//         }
        
//         else    //SCL LOW
//         {  
//             if (bitNr < 9)
//             {
//                 bitNr++;
//                 maske >>= 1;                
//             }
//             else if (bitNr == START)
//             {
//                 bitNr = 1;
//                 maske = 0b10000000;
//                 dataByteNr = 0;
//                 rw = WRITE;
//             }
//             else if (bitNr == 9)
//             {
//                 bitNr = 1;
//                 maske = 0b10000000;
//                 dataByteNr++;
//             }
//             else if (bitNr == NACK)
//             {led();
//                 setInput(SDA_PIN);
//                 bitNr = START;
//                 isDat = 1;
//             }
//         }
    // }
    // else //rw == READ
    // {
    //     if(getPin(SCL_PIN) == 1)                   //SCL HIGH
    //     {
    //         if (bitNr < 9)
    //         {
    //             maske >>= 1;
    //             bitNr++;
    //         }
            
    //         else if (bitNr == 9)     //ACK-Abfrage
    //         {
    //             if (getPin(SDA_PIN))    //NACK
    //                 bitNr = NACK;
    //         }
        
    //     }
    //     else                                        //SCL LOW
    //     {
    //         if (bitNr < 9)
    //         {led();
    //             setOutput(SDA_PIN);
    //             if ((data[dataByteNr] & maske) == maske)
    //                 setPin(SDA_PIN);
    //             else
    //                 clrPin(SDA_PIN);
    //             setInput(SDA_PIN);
                
    //         }
    //         else if (bitNr == 9)
    //         {
    //             // setOutput(SDA_PIN);
    //             // if ((data[dataByteNr] & maske) == maske)
    //             //     setPin(SDA_PIN);
    //             // else
    //             //     clrPin(SDA_PIN);
    //             // setInput(SDA_PIN);
                
    //         }
    //         else if (bitNr == NACK)
    //         {
    //             setInput(SDA_PIN);
    //             bitNr = START;
    //             isDat = 1;
    //             rw = WRITE;
    //         }        
    //     }    
    // }
//}
