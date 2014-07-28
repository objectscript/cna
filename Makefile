CC = gcc
RM = rm

CFLAGS = -Wall -g -Wextra -m64 -fpic

SYS := $(shell gcc -dumpmachine)
ifneq (, $(findstring linux, $(SYS)))
	
	SUFFIX = so
	LDFLAGS = -shared
	LIBS = -ldl -lffi
	CFLAGS += -I/usr/local/lib/libffi-3.1/include/
	ifndef GLOBALS_HOME
		GLOBALS_HOME = /opt/cache
	endif

else ifneq (, $(findstring mingw, $(SYS)))
	
	SUFFIX = dll
	LDFLAGS = -mdll
	LIBS = -L./libs -lffi
	CFLAGS += -I./libs/include
	ifndef GLOBALS_HOME
		GLOBALS_HOME = C:/InterSystems/Cache
	endif

else 
	$(error Unsupported build platform)
endif

CFLAGS += -I${GLOBALS_HOME}/dev/cpp/include


all: libcna.$(SUFFIX) libsq.$(SUFFIX)

cna.o: cna.c cna.h

libcna.$(SUFFIX): cna.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)

sq.o: sq.c

libsq.$(SUFFIX): sq.o
	$(CC) $(LDFLAGS) -o $@ $<

clean:
	$(RM) *.$(SUFFIX) *.o