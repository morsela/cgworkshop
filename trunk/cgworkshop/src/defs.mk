LDFLAGS+=-lm -lml -lcv -lcvaux -lcxcore -lhighgui -lglut
LDFLAGS+=-pg
CPPFLAGS+=-O2 -Wall -L./libs/ -I/usr/local/include/opencv -L/usr/X11R6/lib
CPPFLAGS+=-pg -g
CPP=g++

