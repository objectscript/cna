ifndef GLOBALS_HOME
GLOBALS_HOME = /opt/cache
endif # GLOBALS_HOME

CFLAGS = -Wall -Wextra -m64 -fpic -I${GLOBALS_HOME}/dev/cpp/include

all: libcna.so

cna.o: cna.c cna.h

libcna.so: cna.o
	$(CC) -shared -o $@ $<

clean:
	$(RM) *.so *.o 