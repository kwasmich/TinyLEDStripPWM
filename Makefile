MCU=attiny85
MCU_DUDE=attiny85
OBJCOPY=avr-objcopy
CC=avr-gcc
AVRSIZE=avr-size
CFLAGS=-Wall -Os -mmcu=$(MCU)
LDFLAGS=
SOURCES=main.c i2c/usiTwiSlave.c
OBJECTS=$(SOURCES:%.c=%.o)
EXECUTABLE=TinyLEDStripPWM

all: $(SOURCES) $(EXECUTABLE).hex

$(EXECUTABLE).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(EXECUTABLE).hex: $(EXECUTABLE).elf
	$(OBJCOPY) -O ihex $< $@
	$(AVRSIZE) $<

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

flash: $(EXECUTABLE).hex
	sudo avrdude -c linuxspi -P /dev/spidev0.0 -p $(MCU_DUDE) -U flash:w:$(EXECUTABLE).hex:i

fuse:
	sudo avrdude -c linuxspi -P /dev/spidev0.0 -p $(MCU_DUDE) -U lfuse:w:0xE1:m

clean:
	rm -f $(OBJECTS)
	rm -f $(EXECUTABLE).elf
	rm -f $(EXECUTABLE).hex
