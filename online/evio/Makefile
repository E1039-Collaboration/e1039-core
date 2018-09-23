ifeq ($(shell getconf LONG_BIT),64)
	CC = gcc -m64 -DBIT64 -fPIC
else
	CC = gcc -m32 -fPIC
endif

ARFLAGS = rv

sources = evio.c swap_util.c ceMsgLib.c ceMsgTbl.c
lib_targets := $(patsubst %.c, libmyevio.a(%.o), $(sources))

libmyevio.a: $(lib_targets)

evio.o:	evio.c evfile_msg.h

evtest:	evtest.o evio.o swap_util.o $(CODALIB)
	$(CC) -o $@ evtest.o evio.o swap_util.o $(CODALIB)

#evfile_msg.h: evfile.msg 
#	makeMsgTbl

.PHONY: clean
clean:
	rm -f *.a *.o *~
