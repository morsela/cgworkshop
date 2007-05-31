#
# Sample Makefile
#
# USAGE:
#  1. Replace the a.c b.c with your source files (.c, cpp only)
#  2. replace myproj with the name of the executable.
#  3. In the first time and every time you add a #include statement
#     in your program, type "make depend"
#  4. To build the program, type make.
#

LIBS = -lm -lfreeimage -lcv -lcvaux -lcxcore -lhighgui
CC = gcc
CPP = g++
CFLAGS = -O2 -Wall -L./libs/ -I/usr/local/include/opencv
PROJECT_NAME = fe

SOURCES = Main.cpp FeatureExtraction.cpp
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

.cpp.o:
	$(CPP) -c $(CFLAGS) $<


all: $(PROJECT_NAME)

$(PROJECT_NAME): $(OBJECTS)
	$(CPP) $(CFLAGS) -o $@ $^ $(LIBS)
#	cp $@ ../app/

clean: 
	$(RM) *.o *~

depend:
	$(CPP) -M $(SOURCES) > .depend

-include .depend
