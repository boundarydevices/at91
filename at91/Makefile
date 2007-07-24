#
# Makefile for AT91 utilities and programs
#
#

CFLAGS= -I../ -mcpu=arm7tdmi -Wall
CC = arm-elf-gcc
LD = arm-elf-ld
NM = arm-elf-nm
AR = arm-elf-ar
RANLIB = arm-elf-ranlib
OBJCOPY = arm-elf-objcopy

BINARIES = uartEcho.rom.bin usbEcho.rom.bin usbPeriph.rom.bin usbFlash.ram.bin usbEnum.ram.bin

crcCheck: crcCheck.cpp
	g++ -o $@ $< -lz

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -D_ASMLANGUAGE=1 -c -o $@ $<

%.ram: % ram.lds
	$(LD) -nostdlib -T ram.lds $(filter-out %.lds,$^) -o $@
	$(NM) $* | sort > $*.map

%.rom: % rom.lds
	$(LD) -nostdlib -T rom.lds $(filter-out %.lds,$^) -o $@
	$(NM) $@ | sort > $@.map

%.rom.bin: %.rom Makefile crcCheck
	$(NM) $< | sort > $<.map
	$(OBJCOPY) -O binary $< $@
	perl fixupImg.pl $@
	./crcCheck $@

%.ram.bin: %.ram Makefile crcCheck
	$(NM) $< | sort > $<.map
	$(OBJCOPY) -O binary $< $@
	perl fixupImg.pl $@
	./crcCheck $@

STARTUPOBJS = llInit.o lib1funcs.o
ROMSTART = romStart.o
RAMSTART = ramStart.o

$(ROMSTART): startup.S hardware.h
	$(CC) $(CFLAGS) -D_ASMLANGUAGE=1 -c -o $@ $<

$(RAMSTART): startup.S hardware.h
	$(CC) $(CFLAGS) -D_ASMLANGUAGE=1 -DRAMSTART=1 -c -o $@ $<

APPOBJS = sambaClone.o messageRx.o appEthernet.o appFlash.o appFRAM.o \
          appGPIO.o appSerial.o serialRegs.o controlRx.o leds.o \
          usbPulldown.o \
          usart.o usb_ll.o reboot.o sg.o memory.o hardware.o \
          flash.o adler32.o assert.o debug.o delay.o \
          usbDesc.o
APPLIB = libAppModules.a

$(APPLIB): $(APPOBJS)
	$(AR) r $@ $(APPOBJS)
	$(RANLIB) $@

uartEcho: $(RAMSTART) shiftRAM.o $(STARTUPOBJS) uartEcho.o $(APPLIB) 
	$(LD) -nostdlib -r $^ -o $@

usbEcho: $(RAMSTART) shiftRAM.o $(STARTUPOBJS) usbEcho.o $(APPLIB) 
	$(LD) -nostdlib -r $^ -o $@

usbPeriph: $(ROMSTART) $(STARTUPOBJS) usbPeriph.o $(APPLIB) 
	$(LD) -nostdlib -r $^ -o $@

etherMin: startup.o lib1funcs.o etherMin.o $(APPLIB) 
	$(LD) -nostdlib -r $^ -o $@

usbFlash: $(RAMSTART) shiftRAM.o $(STARTUPOBJS) usbFlash.o $(APPLIB) 
	$(LD) -nostdlib -r $(filter-out %.a,$^) -L . -lAppModules -o $@

usbEnum: $(RAMSTART) shiftRAM.o $(STARTUPOBJS) usbEnum.o $(APPLIB) 
	$(LD) -nostdlib -r $(filter-out %.a,$^) -L . -lAppModules -o $@

usbFRAM: $(RAMSTART) shiftRAM.o $(STARTUPOBJS) usbFRAM.o $(APPLIB) 
	$(LD) -nostdlib -r $(filter-out %.a,$^) -L . -lAppModules -o $@

usbEthernet: $(RAMSTART) shiftRAM.o $(STARTUPOBJS) usbEthernet.o $(APPLIB) 
	$(LD) -nostdlib -r $(filter-out %.a,$^) -L . -lAppModules -o $@

fram: $(RAMSTART) shiftRAM.o $(STARTUPOBJS) fram.o ram.lds
	$(LD) -nostdlib -r $^ -o $@

setPhy: $(RAMSTART) shiftRAM.o $(STARTUPOBJS) setPhy.o ram.lds
	$(LD) -nostdlib -r $^ -o $@

all: $(BINARIES) 

.PHONY: clean

clean:
	rm -f *.o *.map *.bin *.a $(BINARIES) crcCheck usbEcho uartEcho usbFlash usbPeriph usbEnum 
