/*
 * von http://www.eckart-winkler.de/computer/c/varargs.htm
 * 
 * Formatzeichen:
 * %c = Char
 * %s = String
 * %d = Integer
 * char *txt
 * 
 * *txt ist der Text in dem Formatzeichen eingebettet sein können,
 * deren Parameter hinter *txt, durch Kommatas separiert, angegeben werden.
 * Beispiel:
 * outSeriel("test %c, %s, %d", 'c', "String", 123);
 */

#include <stdint.h>
#include "printS.h"
#include "uart_hard.h"

//#include "uart_attiny.h"

void sendThis(uint8_t b);
char getHexDigit(char digit);

void sendThis(uint8_t b) {
//	sendDat(b); //für uart_attiny.h
    uart_Transmit_Hard(b);
}

/*
 * Maximal 1 Byte
 * *txt: Text in dem Formatzeichen eingebettet sein können
 * Formatzeichen: %c: char, %s: string, %d: int8_t als dezimal, %h: int8_t als hex, %b: int8_t als bin 
 */
void printSeriel(char *txt, ...) {
    va_list args;
    char *pt;
    char ch;
    char *str;
    int n;
    char back[10];
    char *ph = back; 
    int h;
    int maske = 0x80;
    int i = 1;

    va_start(args, txt);                //Parameterabfrage initialisieren
    
    for (pt = txt; *pt; pt++) {
        if (*pt != '%') {               //Alle Zeichen ausser % ausgeben 
            sendThis(*pt);
            continue;
        }
        
        switch (*(++pt)) {              //Formatkennzeichner abarbeiten
        
            case 'c': //char
                    ch = (char)va_arg(args, int);
                    sendThis(ch); 
                    break;
            
            case 's': //string
                    str = (char*)va_arg(args, int*);
                    while(*str) {
                        sendThis(*str);
                        str++;
                    } 
                    break;
            
            case 'd': //dezimal
                    n = (int)va_arg(args, int);
                    if(!n) {				//Gleich null?
                        sendThis('0'); 
                    }
                    if(n < 0) {				//Vorzeichen?
                        sendThis('-'); 
                        n = -n; 
                    }
                    while(n) { 				//Ziffern rueckwaerts nach back schreiben
                        h = n % 10; 
                        n = n / 10; 
                        *ph++ = (char)('0' + h); 
                    }
                    while(ph > back)        //Nun Ziffern ausgeben
                        sendThis(*(--ph));
                    break;
                
            case 'h':   //hexa
                    n = (int)va_arg(args, int);
                    sendThis('0');
                    sendThis('x');
                    if (n == 0)
                        sendThis('0');
                    if (n < 16)
                        sendThis('0');
                    while(n) {
                        h = n % 16; 
                        n = n / 16; 
                        *ph++ = h; 
                    }
                    while(ph > back)
                        sendThis(getHexDigit(*(--ph)));
                    break;
            
            case 'b':   //binär
                    n = (int)va_arg(args, int);
                    
                    //sendThis('0');
                    //sendThis('b');
                    //sendThis(' ');
                    
                    for (; i < 5; ++i) {
                        if (n & maske)
                            sendThis('1');
                        else
                            sendThis('0');
                        maske >>= 1;   
                    }
                    sendThis(' ');
                    for (; i < 9; ++i) {
                        if (n & maske)
                            sendThis('1');
                        else
                            sendThis('0');
                        maske >>= 1;   
                    }
                    break;
            
            case 0:
                    return;
                
            default:
                    sendThis(*pt); 
        }
    }
    
    va_end(args);						//Parameterabfrage beenden und Schluss
}

char getHexDigit(char digit) {
    switch (digit) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        case 10: return 'A';
        case 11: return 'B';
        case 12: return 'C';
        case 13: return 'D';
        case 14: return 'E';
        case 15: return 'F';
    }
    return 0;
}
