//*****************************************//
//  midiout.cpp
//  by Gary Scavone, 2003-2004.
//
//  Simple program to test MIDI output.
//
//*****************************************//

// ./midiout

#include <iostream>
#include <cstdlib>
#include "RtMidi.h"

// Platform-dependent sleep routines.
#if defined(__WINDOWS_MM__)
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

#define MIDI_ON 144
#define MIDI_OFF 128
#define MIDI_PROGRAM 192
#define MIDI_CHANNEL 0

#define DRUM_CHANNEL 9
#define BASS_CHANNEL 0
#define REED_CHANNEL 1

#define KICK_1 35
#define KICK_2 36
#define CYMB_1 51
#define CLAP   39

#define BASS 39
#define REED 73
// DEFINE THE NUMBER OF REPEATS HERE
#define REPEATS 2



// This function should be embedded in a try/catch block in case of
// an exception.  It offers the user a choice of MIDI ports to open.
// It returns false if there are no ports available.
bool chooseMidiPort( RtMidiOut *rtmidi );

int main( void )
{
    RtMidiOut *midiout = 0;
    std::vector<unsigned char> message;

    // RtMidiOut constructor
    try {
        midiout = new RtMidiOut();
    }
    catch ( RtMidiError &error ) {
        error.printMessage();
        exit( EXIT_FAILURE );
    }
    unsigned int nPorts = midiout->getPortCount();
    std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
    std::string portName;
    for ( unsigned int i=0; i<nPorts; i++ ) {
        try {
            portName = midiout->getPortName(i);
        }
        catch ( RtMidiError &error ) {
            error.printMessage();
            goto cleanup;
        }
        std::cout << "  Input Port #" << i+1 << ": " << portName << '\n';
    }
    // Call function to select port.
    try {
    if ( chooseMidiPort( midiout ) == false ) goto cleanup;
    }
    catch ( RtMidiError &error ) {
        error.printMessage();
        goto cleanup;
    }

    message.push_back( MIDI_PROGRAM + BASS_CHANNEL );
    message.push_back( BASS );
    midiout->sendMessage( &message );
    message[0] = MIDI_PROGRAM + REED_CHANNEL;
    message[1] = REED;
    midiout->sendMessage( &message );

    message.push_back(0);

    for (int r = 0; r< REPEATS; r++) {
        std::cout << "Repeat #" << r+1 << "\n";
        for (int counter = 0; counter < 16; counter++) {
            if (counter % 2 == 0) {
                message[0] = MIDI_OFF + DRUM_CHANNEL;
                message[1] = KICK_1;
                message[2] = 0;
                midiout->sendMessage( &message );
                message[0] = MIDI_OFF + DRUM_CHANNEL;
                message[1] = KICK_2;
                message[2] = 0;
                midiout->sendMessage( &message );

                message[0] = MIDI_ON + DRUM_CHANNEL;
                message[1] = KICK_1;
                message[2] = 90;
                midiout->sendMessage( &message );
                message[0] = MIDI_ON + DRUM_CHANNEL;
                message[1] = KICK_2;
                message[2] = 90;
                midiout->sendMessage( &message );
            }
            if (counter % 2 == 1) {
                message[0] = MIDI_OFF + DRUM_CHANNEL;
                message[1] = CLAP;
                message[2] = 0;
                midiout->sendMessage( &message );
                message[0] = MIDI_ON + DRUM_CHANNEL;
                message[2] = 90;
                midiout->sendMessage( &message );
            }
            if (counter ==0) {
                message[0] = MIDI_OFF + REED_CHANNEL;
                message[1] = 53;
                message[2] = 0;
                midiout->sendMessage( &message );
                message[0] = MIDI_ON + REED_CHANNEL;
                message[1] = 55;
                message[2] = 100;
                midiout->sendMessage( &message );
            }
            if (counter ==3 || counter ==5 || counter == 9 || counter ==0 || counter == 12) {
                message[0] = MIDI_ON + BASS_CHANNEL;
                message[1] = 36;
                message[2] = 100;
                midiout->sendMessage( &message );
            }
            if (counter ==4 || counter == 10) {
                message[0] = MIDI_ON + BASS_CHANNEL;
                message[1] = 39;
                message[2] = 100;
                midiout->sendMessage( &message );
            }
            if (counter ==8) {
                message[0] = MIDI_OFF +REED_CHANNEL;
                message[1] = 48;
                message[2] = 0;
                midiout->sendMessage( &message );
                message[0] = MIDI_ON +REED_CHANNEL;
                message[1] = 53;
                message[2] = 100;
                midiout->sendMessage( &message );
                message[0] = MIDI_ON +BASS_CHANNEL;
                message[1] = 34;
                midiout->sendMessage( &message );
            }
            if (counter ==6) {
                message[0] = MIDI_OFF +REED_CHANNEL;
                message[1] = 55;
                message[2] = 0;
                midiout->sendMessage( &message );
                message[0] = MIDI_ON +REED_CHANNEL;
                message[1] = 48;
                message[2] = 100;
                midiout->sendMessage( &message );
            }
            if (counter ==11) {
                message[0] = MIDI_ON + BASS_CHANNEL;
                message[1] = 41;
                message[2] = 100;
                midiout->sendMessage( &message );
            }
            if (counter == 12) {
                message[0] = MIDI_OFF + REED_CHANNEL;
                message[1] = 53;
                message[2] = 0;
                midiout->sendMessage( &message );
                message[0] = MIDI_ON + REED_CHANNEL;
                message[1] = 51;
                message[2] = 100;
                midiout->sendMessage( &message );
            }
            if (counter == 14) {
                message[0] = MIDI_OFF + REED_CHANNEL;
                message[1] = 51;
                message[2] = 0;
                midiout->sendMessage( &message );
                message[0] = MIDI_ON + REED_CHANNEL;
                message[1] = 53;
                message[2] = 100;
                midiout->sendMessage( &message );
            }

            SLEEP(250);

            message[0] = MIDI_OFF + BASS_CHANNEL;
            message[1] = 36;
            message[2] = 100;
            midiout->sendMessage( &message );

            message[0] = MIDI_OFF + BASS_CHANNEL;
            message[1] = 39;
            message[2] = 100;
            midiout->sendMessage( &message );

            message[0] = MIDI_OFF + BASS_CHANNEL;
            message[1] = 34;
            message[2] = 100;
            midiout->sendMessage( &message );

            message[0] = MIDI_OFF + BASS_CHANNEL;
            message[1] = 41;
            message[2] = 100;
            midiout->sendMessage( &message );
        }
    }

    message[0] = MIDI_OFF + REED_CHANNEL;
    message[1] = 53;
    message[2] = 0;
    midiout->sendMessage( &message );
    // Sysex: 240, 67, 4, 3, 2, 247
    message[0] = 240;
    message[1] = 67;
    message[2] = 4;
    message.push_back( 3 );
    message.push_back( 2 );
    message.push_back( 247 );
    midiout->sendMessage( &message );

    // Clean up
    cleanup:
    delete midiout;

    return 0;
}

bool chooseMidiPort( RtMidiOut *rtmidi )
{
  std::cout << "\nWould you like to open a virtual output port? [y/N] ";

  std::string keyHit;
  std::getline( std::cin, keyHit );
  if ( keyHit == "y" ) {
    rtmidi->openVirtualPort();
    return true;
  }

  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No output ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      std::cout << "  Output port #" << i << ": " << portName << '\n';
    }

    do {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while ( i >= nPorts );
  }

  std::cout << "\n";
  rtmidi->openPort( i );

  return true;
}
