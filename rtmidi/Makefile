### Example Makefile for MUMT-306 Homework assignments on OS-X systems

PROGRAMS = qmidiin midiout
RM = /bin/rm
CC       = g++
DEFS     = -D__MACOSX_CORE__ -D__LITTLE_ENDIAN__ # use __WINDOWS_MM__ for Windoze
CFLAGS   = -O2 -Wall
LIBRARY = -lm -framework CoreMIDI -framework CoreFoundation -framework CoreAudio # use winmm.lib for Windoze

all : $(PROGRAMS)

clean : 
	-rm -f *~ *.o
	-rm $(PROGRAMS)

strip : 
	strip $(PROGRAMS)

## Include all source files necessary for a program in the dependencies and compiler statements below
qmidiin: qmidiin.cpp RtMidi.o
	$(CC) $(CFLAGS) $(DEFS) -o qmidiin qmidiin.cpp RtMidi.o $(LIBRARY)

midiout: midiout.cpp RtMidi.o
	$(CC) $(CFLAGS) $(DEFS) -o midiout midiout.cpp RtMidi.o $(LIBRARY)

RtMidi.o: RtMidi.cpp RtMidi.h
	$(CC) $(CFLAGS) $(DEFS) -c RtMidi.cpp -o RtMidi.o
