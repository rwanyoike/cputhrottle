CC = /usr/bin/g++
CCFLAGS = -g
LDFLAGS =

BOOST_VERSION = 1_43_0
BOOST_ROOT    = ../
BOOST_PREFIX  = $(BOOST_ROOT)
BOOST_INCLUDES = $(BOOST_PREFIX)//boost_$(BOOST_VERSION)
BOOST_LIBS = $(BOOST_PREFIX)/lib

CCFLAGS += -Wall -I$(BOOST_INCLUDES)
LDFLAGS += -L$(BOOST_LIBS) 

.cc.o:
	$(CC) -c $(CCFLAGS) $<

OBJS1 = cputhrottle.o manip.o 
LIBS1 = 
BINARY1 = cputhrottle
OBJS2 = test.o
LIBS2 = 
BINARY2 = test

$(BINARY1): $(OBJS1)
	$(CC) $(LDFLAGS) -o $(BINARY1) $(OBJS1) $(LIBS1) 

$(BINARY2): $(OBJS2)
	$(CC) $(LDFLAGS) -o $(BINARY2) $(OBJS2) $(LIBS) 

all: $(BINARY1) $(BINARY2)

clean:
	rm -f *.o core $(BINARY1) $(BINARY2)
