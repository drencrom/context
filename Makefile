LD=${CC}

# use the following line if you want to compile the source for 64 bit
# architectures

#mode=64bit

# use the following line if you want to compile the source for 32 bits
# architectures

mode=32bit

ifeq (${mode},"64bit")
 CFLAGS+=-m64 -DSIXTYFOURBITS
 CFLAGSOPT+=-m64 -DSIXTYFOURBITS
 LDFLAGS+=-m64
else
 CFLAGS+=-m32
 CFLAGSOPT+=-m32
 LDFLAGS+=-m32
endif

CFLAGSOPT+=-g -O3 -Wall -ansi -pedantic 
CFLAGS+=-g -Wall -ansi -pedantic -DDEBUG 
LDFLAGS+=-lgsl -lgslcblas -lm

OBJ=main.o\
    wotd.o\
    reverse.o\
    mapfile.o\
    alpha.o\
    fsmTree.o\
    decoderTree.o\
    suffixTree.o\
    statistics.o\
    see.o\
    encoder.o\
    decoder.o\
    arithmetic/bitio.o\
    arithmetic/coder.o\
    gammaFunc.o

OBJOPT=main.opt.o\
       wotd.opt.o\
       reverse.opt.o\
       mapfile.opt.o\
       alpha.opt.o\
       fsmTree.opt.o\
       decoderTree.opt.o\
       suffixTree.opt.o\
       statistics.opt.o\
       see.opt.o\
       encoder.opt.o\
       decoder.opt.o\
       arithmetic/bitio.o\
       arithmetic/coder.o\
       gammaFunc.opt.o

.PHONY: all arithmetic doc clean doc-clean

all: arithmetic hpzip.opt hpzip

arithmetic:
	${MAKE} -C arithmetic "CFLAGS=${CFLAGSOPT}" 

hpzip: ${OBJ}
	${LD} ${LDFLAGS} ${OBJ} -o $@
	ln -sf hpzip hpunzip

hpzip.opt: ${OBJOPT}
	${LD} ${LDFLAGS} ${OBJOPT} -o $@
	ln -sf hpzip.opt hpunzip.opt

doc:
	doxygen Doxyfile

doc-clean:
	rm -rf doc/*

clean:
	rm -f *.[ox] 
	${MAKE} -C arithmetic clean

%.opt.o: %.c
	$(CC) $(CFLAGSOPT) -c $? -o $@
