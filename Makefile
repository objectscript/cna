CC := gcc
RM := rm
CD := cd
MAKE := make

CFLAGS += -Wall -Wextra -fpic -O2 -fno-strict-aliasing -Wno-unused-parameter

SYS := $(shell gcc -dumpmachine)
LIBFFI_PATH := ./libs/libffi/$(SYS)
INCLUDES := -I$(LIBFFI_PATH)/include
LIBS := -L$(LIBFFI_PATH)/.libs -lffi

ifneq (, $(findstring linux, $(SYS)))
	SUFFIX := so
	LDFLAGS := -shared
	LIBS += -ldl
else ifneq (, $(findstring mingw, $(SYS)))
	SUFFIX = dll
	LDFLAGS = -mdll
	CFLAGS += -I./libs/$(PLATFORM)/include/
else 
	$(error Unsupported build platform)
endif


ifndef GLOBALS_HOME
    $(error Couldn't find GLOBALS_HOME)
endif


INCLUDES += -I${GLOBALS_HOME}/dev/cpp/include
CFLAGS += $(INCLUDES)
TESTSDIR := tests

.PHONY: all clean libffi libffi_clean

all: libcna.$(SUFFIX) $(TESTSDIR)/libtest.$(SUFFIX)	

libffi: 
	cd libs/libffi && ./configure --build=$(SYS) --enable-shared=no && $(MAKE)

cna.o: cna.c storage.h

storage.o: storage.c storage.h

libcna.$(SUFFIX): cna.o storage.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(TESTSDIR)/teslib.o: $(TESTSDIR)/testlib.c

$(TESTSDIR)/libtest.$(SUFFIX): $(TESTSDIR)/testlib.o
	$(CC) $(LDFLAGS) -o $@ $^

libffi-clean:
	$(CD) libs/libffi && $(MAKE) clean

clean:
	$(RM) *.$(SUFFIX) *.o $(TESTSDIR)/*.$(SUFFIX) $(TESTSDIR)/*.o