 #https://www-user.tu-chemnitz.de/~heha@/hs/chm/avrdude.chm/

CPATH += -I/usr/lib/avr/include

SOURCES=main.c i2c-host.c printS.c uart_hard.c
TARGET=i2c

MCU=atmega328p
AVRDUDE_MCU=m328p
F_CPU=16000000
BAUD=115200
PROGR=arduino
PORT=/dev/ttyACM0

#ATtiny25/45/85
#MCU=attiny45
#AVRDUDE_MCU=ATtiny45
#F_CPU=1000000
#BAUD=9600
#ende ATtiny

#=========Programmer=============

#Arduinoboard
#PROGR=arduino
#PORT=/dev/ttyUSB0
#Ende Arduinoboard

CC=avr-gcc

CFLAGS=-g -Os -mmcu=${MCU} -DF_CPU=${F_CPU}
CFLAGS+=-I /usr/lib/avr/include
CFLAGS+=-std=c11
CFLAGS+=-Wall
CFLAGS+=-Wextra
#wenns nervt, dann das nächste auskommentieren
#CFLAGS+=-Wno-unused

#fuse!!!!
#LFUSE=0x62 ist default, LFUSE=0xe2 ist mit Vorteiler = 0
LFUSE=0x62
HFUSE=
EFUSE=


all:
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SOURCES}
	avr-objcopy -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex	
	avrdude -c ${PROGR} -p ${AVRDUDE_MCU} -P ${PORT} -b ${BAUD} -U flash:w:${TARGET}.hex
	rm -f *.bin *.hex

file:	
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SOURCES} 
	avr-size --format=avr --mcu=${MCU} ${TARGET}.bin
	avr-objdump -h -S ${TARGET}.bin > ${TARGET}.lis
	rm -f *.bin *.hex
	
asm:
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SOURCES}
	avr-objdump -h -S ${TARGET}.bin > ${TARGET}.lis
	
exe:
	gcc -Wall -o ${TARGET} ${SRCS}
	rm -f *.bin *.hex *.s *.o
	
infos:
	avrdude -c ${PROGR} -p ${AVRDUDE_MCU} -P ${PORT} -b ${BAUD} -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h -v
	
readfuse:
	avrdude -c ${PROGR} -p ${AVRDUDE_MCU} -P ${PORT} -b ${BAUD} -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h
	
writefuses:
	avrdude -c ${PROGR} -p ${AVRDUDE_MCU} -P ${PORT} -b ${BAUD} -U lfuse:w:${LFUSE}:m -U hfuse:w:${HFUSE}:m -U efuse:w:${EFUSE}:m
	
writefuse_l:
	avrdude -c ${PROGR} -p ${AVRDUDE_MCU} -P ${PORT} -b ${BAUD} -U lfuse:w:${LFUSE}:m

writefuse_h:
	avrdude -c ${PROGR} -p ${AVRDUDE_MCU} -P ${PORT} -b ${BAUD} -U hfuse:w:${HFUSE}:m

writefuse_e:
	avrdude -c ${PROGR} -p ${AVRDUDE_MCU} -P ${PORT} -b ${BAUD} -U efuse:w:${EFUSE}:m
	
flashRom:
	avrdude -c ${PROGR} -p ${AVRDUDE_MCU} -P ${PORT} -b ${BAUD} -U flash:w:${TARGET}.hex
	
clean:
	rm -f *.bin *.hex
