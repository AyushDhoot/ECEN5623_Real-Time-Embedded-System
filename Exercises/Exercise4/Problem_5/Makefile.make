INCLUDE_DIRS = 
LIB_DIRS = 
CC=g++

CDEFS=
CFLAGS= -O0 -pg -g --std=c++11 $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lrt 
CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lrt -lpthread

HFILES= 
CFILES= 
CPPFILES= problem_5.cpp

SRCS= ${HFILES} ${CFILES}
CPPOBJS= ${CPPFILES:.cpp=.o}

all:	problem_5 

clean:
	-rm -f *.o *.d
	-rm -f problem_5

distclean:
	-rm -f *.o *.d

problem_5: problem_5.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o `pkg-config --libs opencv` $(CPPLIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CC) $(CFLAGS) -c $<
