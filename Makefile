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
		$(error Couldn't find GLOBALS_HOME)
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

TESTSDIR = tests


all: libcna.$(SUFFIX) $(TESTSDIR)/libtest.$(SUFFIX)

cna.o: cna.c storage.h

storage.o: storage.c storage.h

libcna.$(SUFFIX): cna.o storage.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(TESTSDIR)/teslib.o: $(TESTSDIR)/testlib.c

$(TESTSDIR)/libtest.$(SUFFIX): $(TESTSDIR)/testlib.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	$(RM) *.$(SUFFIX) *.o $(TESTSDIR)/*.$(SUFFIX) $(TESTSDIR)/*.o