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


using namespace stk;
using namespace std;
#define SLEEP(milliseconds) usleep( (unsigned long) (milliseconds * 1000.0) )

int minor_pentatonic[12] = {0, 3, 5, 7, 10, 12, 15, 17, 19, 22, 24, 27};
int minor_blues[12] = {0, 3, 5, 6, 7, 10, 12, 15, 17, 18, 19, 22};
int major_scale[12] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19};
int minor_scale[12] = {};
int base_note = 57;
int scale [12] = {};
string selections[3] = {"Tonic Note", "Scale Type", "Oscillator"};

bool done;

float note_to_freq (int note_num) {
    int diff = note_num - 69;
    return 440.0 * pow(2.0, ((float)diff)/12.0);
}


int main()
{
    // Set the global sample rate before creating class instances.
    Stk::setSampleRate(48000.0);
    Stk::showWarnings(true);
    initscr();
    noecho();
    cbreak();
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    WINDOW * menuwin = newwin(6, xMax-12, yMax-8, 5);
    box(menuwin, 0, 0);
    refresh();
    wrefresh(menuwin);
    keypad(menuwin, true);

    // menu_win = newwin(HEIGHT, WIDTH, starty, startx);
    memcpy(scale, major_scale, sizeof(minor_pentatonic));
    SineWave sines[12];
    int velocities[12] = {};
    Envelope envs[12];
    Envelope env;
    Envelope velo_env;
    env.setTime(0.01);
    velo_env.setTime(0.01);
    env.keyOff();
    velo_env.keyOff();
    int env1_state = 1;
    for (int i = 0; i < 12; i++) {
        envs[i].keyOff();
        envs[i].setTime(0.01);
        sines[i].setFrequency(note_to_freq(base_note + scale[i]));
    }
    RtMidiIn *midiin = 0;
    int nBytes;
    double stamp;
    std::vector<unsigned char> message;
    
    RtWvOut dac(2); // Define and open the default realtime output device for one-channel playback
    double temp;
    int i = 0;
    std::string port_name;
    int note;
    int mes = 0;
    int val = 0;
    int num_notes = 0;
    int old_note = -1;
    int old_velo = -1;

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
        // cout << port_name << endl;
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
    int on;
    int status;
    double sample;
    double sample1;
    double sample2;
    double velocity;
    
    while (!done) {

        mvwprintw(menuwin, 1, 1, "Hello World");
        refresh();
        


        temp = 0.0;
        sample = 0.0;
        stamp = midiin->getMessage(&message);
        nBytes = message.size();
        // for (i = 0; i<nBytes; i++) {
        //     cout << (int) message[i] << " ";
        // }
        if (nBytes > 0) {
            // cout << endl;
            mes = (int) message[1];
            val = (int) message[2];
            if (mes == 1) {
                note = val / 11;
                if (old_note < 0) {
                    envs[note].keyOn();
                }
                else if (old_note != note) {
                    envs[old_note].keyOff();
                    envs[note].keyOn();
                }
                old_note = note;
            }
            else if (mes == 2) {
                velocity = (double) val;
                if (old_velo < 0) {
                    velo_env.keyOn();
                }
                else if (velocity != old_velo) {
                    velo_env.keyOff();
                    velo_env.keyOn();
                }
                old_velo = velocity;
            }
            else if (mes == 3) {
                if (val == 127) {
                    env.keyOn();
                }
                else {
                    env.keyOff();
                }
            }
            else {continue;}
        } 

        for (i = 0; i < 12; i++) {
            sample += sines[i].tick() * envs[i].tick();
        }
        sample = sample * env.tick() * (velocity * velo_env.tick())/128.0 ;
        dac.tick(sample);
    }

    cleanup:

    delete midiin;
    return 0;
}