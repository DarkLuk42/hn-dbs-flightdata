#
# GNU makefile for importflights
#================================


# compiler settings
#--------------------------------

# settings for GNU C compiler
CC = g++
CFLAGS = -Wall -g -I`pg_config --includedir`
LDFLAGS = -L`pg_config --libdir` -lstdc++

PROGRAM = importflights
OBJECTS = main.o copydata.o database.o


# from here on, no alterations
# should be necessary
#----------------------------------

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) 
	$(CC) -o $@ $+ $(LDFLAGS) -lpq 

%.o: %.cpp copydata.h database.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $*.cpp

clean:
	rm -f *.o $(PROGRAM)

