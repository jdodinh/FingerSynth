// g++ -Istk/include/ -Lstk/src/ -D__MACOSX_CORE__ main.cpp -lstk -lpthread -framework CoreAudio -framework CoreMIDI -framework CoreFoundation
#include "SineWave.h"
#include "RtWvOut.h"
#include "Envelope.h"
#include "ADSR.h"
#include "FileWvOut.h"
#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <ncurses.h>
#include <cmath>
#include <time.h>
#include <signal.h>
#include "RtMidi.h"
#include <unistd.h>  /* only for sleep() */
#include <numeric>

#define WIDTH 30
#define HEIGHT 10 

using namespace stk;
using namespace std;
#define SLEEP(milliseconds) usleep( (unsigned long) (milliseconds * 1000.0) )

bool done;


float note_to_freq (int note_num) {
    int diff = note_num - 69;
    return 440.0 * pow(2.0, ((float)diff)/12.0);
}

int main()
{
    // Set the global sample rate before creating class instances.
    Stk::setSampleRate(44100.0);
    Stk::showWarnings(true);
    

    // menu_win = newwin(HEIGHT, WIDTH, starty, startx);
    SineWave sines[128];
    int velocities[128] = {};
    Envelope envs[128];
    for (int i = 0; i < 128; i++) {
        envs[i].keyOff();
        envs[i].setTime(0.01);
        sines[i].setFrequency(note_to_freq(i));
    }
    cout << endl;
    RtMidiIn *midiin = 0;
    int nBytes;
    double stamp;
    std::vector<unsigned char> message;
    
    RtWvOut dac(2); // Define and open the default realtime output device for one-channel playback
    StkFloat temp;
    int i = 0;
    int counter = 0;
    int counter2 = 0;
    int note = 60;
    int velocity = 0;
    float freq = note_to_freq(note);
    std::string port_name;
    int active_notes[128] = {};
    int num_notes = 0;

    // cout << "Enter a key number" << endl;
    try {
        midiin = new RtMidiIn();
    }
    catch (RtMidiError &error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }
    unsigned int port = 0;
    unsigned int nPorts = midiin->getPortCount();
    for (i = 0; i<nPorts; i++) {
        port_name = midiin->getPortName(i);
        // cout << port_name << endl;
        if (port_name == "AudioSwift 3") {
            port = i;
        }
    }

    try {
        port_name = midiin->getPortName(port);
        cout << port_name << endl;
        midiin->openPort(port);
    }
    catch (RtMidiError &error) {
        error.printMessage();
        goto cleanup;
    }
    midiin->ignoreTypes(false, false, false);

    done = false;
    int mes_num;
    int channel;
    int ind;
    int status;
    double sample;


    while (!done) {
        sample = 0.0;
        num_notes = 0;
        stamp = midiin->getMessage(&message);
        nBytes = message.size();
        // for (i = 0; i<nBytes; i++) {
        //     cout << (int) message[i] << " ";
        // }
        if (nBytes > 0) {
            // cout << endl;
            note = (int) message[1];
            velocity = (int) message[2];
            if (velocity > 0) {
                velocities[note] = velocity;
                active_notes[note] = 1;
                envs[note].keyOn();
            }
            else {
                active_notes[note] = 0;
                envs[note].keyOff();
            }
        } 
        for (i = 0; i < 128; i++) {
            // if (active_notes[i] == 1) {
            sample += sines[i].tick() * envs[i].tick() * ((double) velocities[i])/128.0;
            num_notes += active_notes[i];
            // }
        }
        if (num_notes == 0) {
            num_notes = 1;
        }
        sample = sample/(pow((double) num_notes, 0.5));
        cout << sample << " " << num_notes << endl;
        dac.tick(sample);
    }

    cleanup:

    delete midiin;
    return 0;
}