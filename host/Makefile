#
# Makefile for AT91 utilities and programs
#
#

CONFIG_KERNELPATH ?= ~/linux-2.6.19.2
CROSS_COMPILE ?= arm-linux-

G++ = $(CROSS_COMPILE)g++
CFLAGS = -I ~/userland/install/include \
         -I $(CONFIG_KERNELPATH)/include/ \
         -L ~/userland/install/lib

EXES = at91run at91up at91down at91reg at91reset at91flash at91adler at91mac fram_read fram_write

define compile
     @echo "\tCC\t" $?
     @$(G++) -c -o $@ $<
endef
 
define compile_link
     @echo "\tCCLD\t" $@
     @$(G++) $(CFLAGS) -o $@ $^
endef
    
%.o: %.cpp
	$(compile)
   	
at91up: at91up.cpp sambaUtil.o
	$(compile_link)

at91run: at91run.cpp sambaUtil.o uploadFlash.o
	$(compile_link)

at91down: at91down.cpp sambaUtil.o
	$(compile_link)

at91reg: at91reg.cpp sambaUtil.o
	$(compile_link)

at91reset: at91reset.cpp sambaUtil.o
	$(compile_link)

at91flash: at91flash.cpp sambaUtil.o uploadFlash.o
	$(compile_link)

fram_read: fram_read.cpp sambaUtil.o hexDump.o
	$(compile_link)

fram_write: fram_write.cpp sambaUtil.o hexDump.o
	$(compile_link)

at91adler: at91adler.cpp sambaUtil.o
	$(compile_link) -lz

etherSend: etherSend.cpp sambaUtil.o hexDump.o
	$(compile_link) -lz

at91mac: at91mac.cpp sambaUtil.o hexDump.o
	$(compile_link) -lz

all: $(EXES)

.PHONY: clean

clean:
	rm -f *.o $(EXES)
