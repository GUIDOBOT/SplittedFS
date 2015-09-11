TARGET = SPFS_TESTS
OBJS = spfs_test.o ../spfs/spfs_hooks.o
CFLAGS = -O2 -G0 -Wall -std=c99
ASFLAGS = $(CFLAGS)
LIBS = 
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = SPFS_TESTS
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
