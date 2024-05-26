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

#define ADR                         1
#define MAX_BYTE                    4
#define SDA_PIN PINDEF(D, 5)        //PCINT21
#define SCL_PIN PINDEF(D, 6)        //PCINT22

#define PCINT_VECT                  PCINT2_vect
#define PCINT_SCL_ON                PCMSK2 |= (1 << PCINT22)
#define PCINT_SCL_OFF               PCMSK2 &= ~(1 << PCINT22) 
#define PCINT_SDA_ON                PCMSK2 |= (1 << PCINT21)
#define PCINT_SDA_OFF               PCMSK2 &= ~(1 << PCINT21)
#define PCINT_SDA_SCL_FLAG_CLEAR    PCIFR |= (1 << PCIF2)
#define PCINT_SDA_SCL_ENABLE        PCICR |= (1 << PCIE2)
#define PCINT_SDA_SCL_DISABLE       PCICR &= ~(1 << PCIE2)




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

void start_F();
void bit_1_R_L_F();
void bit_1_R_H_F();
void bit_2_R_L_F();
void bit_2_R_H_F();
void bit_3_R_L_F();
void bit_3_R_H_F();
void bit_4_R_L_F();
void bit_4_R_H_F();
void bit_5_R_L_F();
void bit_5_R_H_F();
void bit_6_R_L_F();
void bit_6_R_H_F();
void bit_7_R_L_F();
void bit_7_R_H_F();
void bit_8_R_L_F();
void bit_8_R_H_F();
void ack_R_L_F();
void ack_R_H_F();
void bit_1_S_L_F();
void bit_1_S_H_F();
void bit_2_S_L_F();
void bit_2_S_H_F();
void bit_3_S_L_F();
void bit_3_S_H_F();
void bit_4_S_L_F();
void bit_4_S_H_F();
void bit_5_S_L_F();
void bit_5_S_H_F();
void bit_6_S_L_F();
void bit_6_S_H_F();
void bit_7_S_L_F();
void bit_7_S_H_F();
void bit_8_S_L_F();
void bit_8_S_H_F();
void ack_S_L_F();
void ack_S_H_F();
void stop_F();
void getStop_F();

typedef void (* i2cFunk) (void);
volatile i2cFunk i2cFunktionen[] = {start_F, 
bit_1_R_L_F, bit_1_R_H_F, bit_2_R_L_F, bit_2_R_H_F, bit_3_R_L_F, bit_3_R_H_F, bit_4_R_L_F, bit_4_R_H_F, 
bit_5_R_L_F, bit_5_R_H_F, bit_6_R_L_F, bit_6_R_H_F, bit_7_R_L_F, bit_7_R_H_F, bit_8_R_L_F, bit_8_R_H_F, 
ack_R_L_F, ack_R_H_F, 
bit_1_S_L_F, bit_1_S_H_F, bit_2_S_L_F, bit_2_S_H_F, bit_3_S_L_F, bit_3_S_H_F, bit_4_S_L_F, bit_4_S_H_F, 
bit_5_S_L_F, bit_5_S_H_F, bit_6_S_L_F, bit_6_S_H_F, bit_7_S_L_F, bit_7_S_H_F, bit_8_S_L_F, bit_8_S_H_F, 
ack_S_L_F, ack_S_H_F, 
stop_F, getStop_F};

static volatile uint8_t isDat;
uint8_t* data;

#define LED_PIN PINDEF(D, 2)
void led() {
    setPin(LED_PIN);
    clrPin(LED_PIN);
}
#define LED led();

inline void stop_ISR_SDA();
inline void start_ISR_SDA();
inline void stop_ISR_SCL();
inline void start_ISR_SCL();
void setI2CStop();

void setupI2cSoftClient() {
    setInput(SDA_PIN);
 	setInput(SCL_PIN);
 	setOutput(LED_PIN);
    clrPin(LED_PIN);
    cli();
    PCINT_SDA_SCL_ENABLE;
    stop_ISR_SCL();
    start_ISR_SDA();
    sei();
}

inline void _delay_cycl(uint8_t cy);
inline void _delay_cycl(uint8_t cy) {
    do {
        __asm__ volatile ("nop");
    } while (--cy);
}

inline void stop_ISR_SDA() {
    PCINT_SDA_OFF;
}

inline void start_ISR_SDA() {
    PCINT_SDA_ON;
}

inline void stop_ISR_SCL() {
    PCINT_SCL_OFF;
}

inline void start_ISR_SCL() {

   PCINT_SCL_ON;
}

void getDatafromHost(uint8_t *dat, uint8_t maxB) {
	data = dat;
    isDat = 0;
}

void sendDatatoHost(uint8_t *dat, uint8_t maxB) {
	data = dat;
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


static volatile uint8_t state = start;
static uint8_t dataByteNr = 0;


        void start_F() {
        //while(getPin(SDA_PIN));
            if (getPin(SCL_PIN) == 1)            
                {
                    if (getPin(SDA_PIN) == 0)   //Wechsel auf low -> start
                    {
                        stop_ISR_SDA();         //Ab jetzt SCL ist Interrup-Chef
                        start_ISR_SCL();
                        dataByteNr = 0;
                        state = bit_1_R_L;
                    }
                    else                        //Wechsel auf high -> stop
                    {
                        //stop_ISR_SCL();
                        state = start;
                    }
                }
        }
            
        void bit_1_R_L_F() { //78
            setInput(SDA_PIN);
            data[dataByteNr] = 0;
            state = bit_1_R_H;
        }
        void bit_1_R_H_F() { //70
            state = start;
            //PCINT_SDA_OFF;                //für Test auf stop????????????????????????????????????????????????
            if (getPin(SDA_PIN))
                data[dataByteNr] += 128;
            state = bit_2_R_L;               //80 Cycles bis hier
        }
            
        void bit_2_R_L_F() { //68
            //stop_ISR_SDA();
            state = bit_2_R_H;
        }
        void bit_2_R_H_F() { //63
            if (getPin(SDA_PIN))
                data[dataByteNr] += 64;
            state = bit_3_R_L;
        }
            
        void bit_3_R_L_F() { //61
            state = bit_3_R_H;
        }
        void bit_3_R_H_F() { //65
            if (getPin(SDA_PIN))
                data[dataByteNr] += 32; 
            state = bit_4_R_L;
        }
            
         void bit_4_R_L_F() {    //61
            state = bit_4_R_H;
        }
        void bit_4_R_H_F() { //65
            if (getPin(SDA_PIN))
                data[dataByteNr] += 16;
            state = bit_5_R_L;
        }   
            
       void bit_5_R_L_F() {  //64
            state = bit_5_R_H;
        }
        void bit_5_R_H_F() { //65
            if (getPin(SDA_PIN))
                data[dataByteNr] += 8;
            state = bit_6_R_L;
        }
        
        void bit_6_R_L_F() { //64
            state = bit_6_R_H;
        }
        void bit_6_R_H_F() { //67
            if (getPin(SDA_PIN))
                data[dataByteNr] += 4;
            state = bit_7_R_L;
        }
        
        void bit_7_R_L_F() { //62
            state = bit_7_R_H;
        }
        void bit_7_R_H_F() { //78
            if (getPin(SDA_PIN))
                data[dataByteNr] += 2;
            state = bit_8_R_L;
        }
        
        void bit_8_R_L_F() { //61
            state = bit_8_R_H;
        }
        void bit_8_R_H_F() { //75
            if (getPin(SDA_PIN))
                data[dataByteNr] += 1;
            state = ack_R_L;
        }

            
        void ack_R_L_F() { //78
            // if ((data[0] >> 1) == ADR)
            // {
                setOutput(SDA_PIN);              //ACK senden
                clrPin(SDA_PIN);
                state = ack_R_H;
            // }
            // else
            // {
            //     setOutput(SDA_PIN);              //NACK senden
            //     setPin(SDA_PIN);
            //     state = stop;
            //     isDat = 0;
            // }
            
        }
            
        void ack_R_H_F() {   //89
            dataByteNr++;
            if ((data[0] & 1) == 1)
                state = bit_1_S_L;
            else
                state = bit_1_R_L;

            if (dataByteNr == MAX_BYTE)         //jetzt nur als Hilfe!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            {
                // isDat = 1;
                state = stop;
            }
        }
        
        
        
        
        
        

        
        void bit_1_S_L_F() {
        setOutput(SDA_PIN);
            if ((data[dataByteNr] & 128) == 128)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_1_S_H;
        }
        void bit_1_S_H_F() {
            //state = start;
            //start_ISR_SDA();                //für Test auf stop

            //_delay_us(4);   //???????????????????????????????????????????????????????????????????
            //stop_ISR_SDA();
            state = bit_2_S_L;
        }
        
        void bit_2_S_L_F() {
            setOutput(SDA_PIN);
            if ((data[dataByteNr] & 64) == 64)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_2_S_H;
        }
        void bit_2_S_H_F() {
            state = bit_3_S_L;
        }
        
        void bit_3_S_L_F() {
            if ((data[dataByteNr] & 32) == 32)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_3_S_H;
        }
        void bit_3_S_H_F() {
            state = bit_4_S_L;
        }
            
        void bit_4_S_L_F() {
            if ((data[dataByteNr] & 16) == 16)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_4_S_H;
        }
        void bit_4_S_H_F() {
            state = bit_5_S_L;
        }
        
        void bit_5_S_L_F() {
            if ((data[dataByteNr] & 8) == 8)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_5_S_H;
        }
        void bit_5_S_H_F() {
            state = bit_6_S_L;
        }
        
        void bit_6_S_L_F() {
            if ((data[dataByteNr] & 4) == 4)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_6_S_H;
        }
        void bit_6_S_H_F() {
            state = bit_7_S_L;
        }
        
        void bit_7_S_L_F() {
            if ((data[dataByteNr] & 2) == 2)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_7_S_H;
        }
        void bit_7_S_H_F() {
            state = bit_8_S_L;
        }
        
        void bit_8_S_L_F() {
            if ((data[dataByteNr] & 1) == 1)
                setPin(SDA_PIN);
            else
                clrPin(SDA_PIN);
            state = bit_8_S_H;
        }
        void bit_8_S_H_F() {
            state = ack_S_L;
        }
        
        
        
        
        
        
        
        void ack_S_L_F() {   //64
            setInput(SDA_PIN);
            state = ack_S_H;
        }
        void ack_S_H_F() {   //75
            state = bit_1_S_L;
            dataByteNr++;
            if (dataByteNr == MAX_BYTE)         //jetzt nur als Hilfe!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            {
                
                state = stop;
            }
        } 
     
        void stop_F() {
            setInput(SDA_PIN);
            stop_ISR_SCL();
            start_ISR_SDA();
            isDat = 1;
            state = start;    //start;//printS("%d %d %d %d\n", data[0], data[1], data[2], data[3]);
        }
            
        void getStop_F() {
            state = start;
        }


    
    
ISR(PCINT_VECT) {
    i2cFunktionen[start]();
    uint8_t pinStateNew, pinStateOld = 0;
    //while
    while (state) {
        pinStateNew = getPin(SCL_PIN);
        if (pinStateNew != pinStateOld) {
            i2cFunktionen[state]();
            pinStateOld = pinStateNew;
        }
    }
    PCINT_SDA_SCL_FLAG_CLEAR;
}


