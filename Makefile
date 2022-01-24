##Variables

#AS = as8051
#AS = sdas8051
#ASFLAGS = -losga

#LD = aslink
#LD = sdld
#LDFLAGS = -f

CC = sdcc
CCFLAGS = -mmcs51 

PRG = stcgal
PRGFLAGS = -P stc89 -b 2400

BIN = main

##Rules
#%.rel: %.asm
#	${AS} ${ASFLAGS} $^

%.hex: %.ihx
	packihx $^ > $@

#%.ihx: ${addsuffix .rel, ${FILES}}
#	${LD} ${LDFLAGS} $@

%.ihx: %.c
	${CC} ${CCFLAGS} $^

##Targets
.PHONY: all
all: build clean

.PHONY: build
build: ${BIN}.hex

.PHONY: clean
clean:
	rm -f *.{rel,ihx,map,lst,rst,sym,hlr,lk,asm,mem}
	
.PHONY: cleanall
cleanall: clean
	rm -f *.hex

.PHONY: flash
flash: all
	${PRG} ${PRGFLAGS} ${BIN}.hex
