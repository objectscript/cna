ifndef GLOBALS_HOME
GLOBALS_HOME = /opt/cache
endif # GLOBALS_HOME

CFLAGS = -Wall -Wextra -m64 -fpic -I${GLOBALS_HOME}/dev/cpp/include -I/usr/local/lib/libffi-3.1/include/

all: libcna.so libsq.so

cna.o: cna.c cna.h

libcna.so: cna.o
	$(CC) -shared -o $@ $< -ldl -lffi

sq.o: sq.c

libsq.so: sq.o
	$(CC) -shared -o $@ $<

test.o: test.c all 

test: test.o
	$(CC) -o $@ $< -L. -lcna

clean:
	$(RM) *.so *.o test