
CC=gcc


CFLAGS= -O0 -g 
  
CFILES= linux_implementation.c

SRCS= ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	linux_implementation

clean:
	-rm -f *.o *.d
	-rm -f linux_implementation

linux_implementation : linux_implementation.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -lrt

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
