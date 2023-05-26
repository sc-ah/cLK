CC?=gcc
OBJ=nbhextract.o utils.o io.o crc32.o flasher.o info.o prompt.o nbh.o main.o
BIN=HTCFlasher
CFLAGS=-Wall -m32
LDFLAGS=-m32

all: ${OBJ}
	${CC} $(CFLAGS) $(LDFLAGS) ${OBJ} -o ${BIN}
	${CC} $(CFLAGS) $(LDFLAGS) nbhextract.o nbh.o yang.c -o yang

clean:
	-rm -f ${OBJ} ${BIN} yang
