CFLAGS=-ansi -Wpedantic -Wall -Werror -D_THREAD_SAFE -D_REENTRANT -D_POSIX_C_SOURCE=200112L
LIBRARIES=-lpthread
LFLAGS=

all: elefanti.exe

elefanti.exe: elefanti.o DBGpthread.o
	gcc ${LFLAGS} -o elefanti.exe elefanti.o DBGpthread.o ${LIBRARIES}

elefanti.o: elefanti.c DBGpthread.h
	gcc -c ${CFLAGS} elefanti.c

DBGpthread.o: DBGpthread.c printerror.h
	gcc -c ${CFLAGS} DBGpthread.c

.PHONY: clean run

clean:
	rm -f *.exe *.o *~ core

run: elefanti.exe
	./elefanti.exe


