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
#include <thread>
#include <string>
#include <Brass.h>
#include <Echo.h>
#include <BlitSaw.h>
#include <BlitSquare.h>

using namespace stk;
using namespace std;
#define SLEEP(milliseconds) usleep( (unsigned long) (milliseconds * 1000.0) )

int major_scale[12] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19};
int minor_scale[12] = {0, 2, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19};
int major_pentatonic[12] = {0, 2, 4, 7, 9, 12, 14, 16, 19, 21, 24, 26};
int minor_pentatonic[12] = {0, 3, 5, 7, 10, 12, 15, 17, 19, 22, 24, 27};
int minor_blues[12] = {0, 3, 5, 6, 7, 10, 12, 15, 17, 18, 19, 22};
int scale [12] = {};
int change = 0;
int col_max[4] = {2, 2, 2, 2};
int num_options = 4;
int base_note = 57;
int scale_number = 0;
int oscillator_number = 0;
char scale_names[5][20] = {"Major", "Minor", "Major Pentatonic", "Minor Pentatonic", "Minor Blues"};
char oscillators[3][20] = {"SineWave", "Sawtooth", "Square"};




bool done;

float note_to_freq (int note_num) {
    int diff = note_num - 69;
    return 440.0 * pow(2.0, ((float)diff)/12.0);
}


int change_tonic(int i, int j, WINDOW * w) {
    int current_note = base_note;
    char item[20], ch;
    while(( ch = wgetch(w)) != 10) {
        switch (ch) {
            case (char) KEY_RIGHT:
                current_note++;
                if (current_note > 100) {
                    current_note = 100;
                }
                break;
            case (char) KEY_LEFT:
                current_note--; 
                if (current_note < 20) {
                    current_note = 20;
                }
                break;
        }
        wattroff(w, A_STANDOUT);
        mvwprintw( w, 6, 20 * i + 5, "                    ");
        sprintf(item, "%d", current_note);
        wattron(w, A_STANDOUT);
        mvwprintw( w, 6, 20 * i + 5, "%s", item);
    }
    base_note = current_note;
    wattroff( w, A_STANDOUT );
    return 1;
}

int change_scale(int i, int j, WINDOW * w) {
    int current_scale = scale_number;
    char item[20], ch;
    while(( ch = wgetch(w)) != 10) {
        switch (ch) {
            case (char) KEY_RIGHT:
                current_scale++;
                if (current_scale > 4) {
                    current_scale = 0;
                }
                break;
            case (char) KEY_LEFT:
                current_scale--;
                if (current_scale < 0) {
                    current_scale = 4;
                }
                break;
        }
        wattroff(w, A_STANDOUT);
        mvwprintw( w, 6, 20 * i + 5, "                    ");
        sprintf(item, "%-7s", scale_names[current_scale]);
        wattron(w, A_STANDOUT);
        mvwprintw( w, 6, 20 * i + 5, "%s", item);
    }
    scale_number = current_scale;
    switch (scale_number) {
        case 0:
            memcpy(scale, major_scale, sizeof(scale));
            break;
        case 1:
            memcpy(scale, minor_scale, sizeof(scale));
            break;
        case 2:
            memcpy(scale, major_pentatonic, sizeof(scale));
            break;
        case 3:
            memcpy(scale, minor_pentatonic, sizeof(scale));
            break;
        case 4:
            memcpy(scale, minor_blues, sizeof(scale));
            break;
    }
    return 1;
}

int change_oscillator(int i, int j, WINDOW * w) {
    int current_oscillator = oscillator_number;
    char item[20], ch;
    while(( ch = wgetch(w)) != 10) {
        switch (ch) {
            case (char) KEY_RIGHT:
                current_oscillator++;
                if (current_oscillator > 2) {
                    current_oscillator = 0;
                }
                break;
            case (char) KEY_LEFT:
                current_oscillator--; 
                if (current_oscillator < 0) {
                    current_oscillator = 2;
                }
                break;
        }
        wattroff(w, A_STANDOUT);
        mvwprintw( w, 6, 20 * i + 5, "                    ");
        sprintf(item, "%-7s", oscillators[current_oscillator]);
        wattron(w, A_STANDOUT);
        mvwprintw( w, 6, 20 * i + 5, "%s", item);
    }
    oscillator_number = current_oscillator;
    wattroff( w, A_STANDOUT );
    return 0;
}

void menu_loop() {

    char item[20], ch;
    char options[4][20] = {"Tonic Note", "Scale Type", "Oscillator", "Filter"};
    sprintf(item, "%d", base_note);
    char values[4][20] = {"57", "Major", "SineWave", "None"};
    
    int i = 0;
    int j = 0;
    int z = 0;
    /* INIT NCURSES */
    char list[5][20] = { "One", "Two", "Three", "Four", "Five" };
    

    // Get screen size
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    cout << xMax << endl;

    initscr();
    // Create window for input
    WINDOW * w;
    w = newwin( 15, 100, 1, 5 );
    box(w, 0, 0);
    j = 5;
    for( i=0; i<num_options; i++ ) {
        if( i == 0 ) 
            wattron( w, A_STANDOUT ); // highlights the first item.
        else
            wattroff( w, A_STANDOUT );
        sprintf(item, "%-7s",  options[i]);
        mvwprintw( w, j, 20 * i + 5, "%s", item );
        sprintf(item, "%-7s",  values[i]);
        wattroff( w, A_STANDOUT );
        mvwprintw( w, j+1, 20 * i + 5, "%s", item);
    }
 
    wrefresh( w ); // update the terminal screen

    noecho(); // disable echoing of characters on the screen
    keypad( w, true ); // enable keyboard input for the window.
    curs_set( 0 ); // hide the default screen cursor.
    i = 0;
    j = 0;

    while(( ch = wgetch(w)) != 113){ 
         
            // right pad with spaces to make the items appear with even width.
            sprintf(item, "%-7s",  options[i]); 
            mvwprintw( w, 5, 20*i + 5, "%s", item ); 
            sprintf(item, "%-7s",  values[i]); 
            mvwprintw( w, 6, 20*i + 5, "%s", item );
              // use a variable to increment or decrement the value based on the input.
            switch( ch ) {
                case (char) KEY_UP:
                    j--;
                    if (j < 0) {j = 0;};
                    break;
                case (char) KEY_DOWN:
                    j++;
                    if (j >= col_max[i]) {
                        j = col_max[i] - 1;
                    }
                    break;
                case (char) KEY_LEFT:
                    if (j == 1) {break;}
                    else {
                        i--;
                        if (i < 0) {i = 0;}
                        break;
                    }
                case (char) KEY_RIGHT:
                    if (j == 1) {break;}
                    else {
                        i++;
                        if (i >= num_options) {i = num_options - 1;}
                        break;
                    }
                    
            }

            // now highlight the next item in the list.
            wattron( w, A_STANDOUT );
            if (j == 0) {
                sprintf(item, "%-7s",  options[i]);
                mvwprintw( w, 5, 20*i+5,"%s", item);
            }
            else {
                sprintf(item, "%-7s",  values[i]);
                mvwprintw( w, 6, 20*i+5,"%s", item);
                if (i == 0) {
                    change = change_tonic(i,j, w);
                    sprintf(values[i], "%d", base_note);
                }
                else if (i == 1) {
                    change = change_scale(i, j, w);
                    sprintf(values[i], "%-7s", scale_names[scale_number]);
                }
                else if (i == 2) {
                    change = change_oscillator(i, j, w);
                    sprintf(values[i], "%-7s", oscillators[oscillator_number]);
                }
            }
            
            wattroff( w, A_STANDOUT );

    }
 
    delwin(w);
    endwin();


    // for (ii = 0; ii<3; ii++) {
    //     mvprintw(ii+1, 1, selections[ii].c_str());
    // }
    // wrefresh(menuwin);

    // keypad(menuwin, true);
    // initscr();
    // noecho();
    // cbreak();
}

int main()
{
    // Set the global sample rate before creating class instances.
    Stk::setSampleRate(48000.0);
    Stk::showWarnings(true);
    
    SineWave sines[12];
    BlitSaw sawtooth[12];
    BlitSquare square[12];
    Echo echo;
    echo.setDelay(100);
    echo.setMaximumDelay(500);
    Envelope envs[12];
    Envelope env;
    Envelope velo_env;
    memcpy(scale, major_scale, sizeof(minor_pentatonic));
    
    env.setTime(0.01);
    velo_env.setTime(0.01);
    env.keyOff();
    velo_env.keyOff();
    int env1_state = 1;
    for (int i = 0; i < 12; i++) {
        envs[i].keyOff();
        envs[i].setTime(0.01);
        sines[i].setFrequency(note_to_freq(base_note + scale[i]));
        sawtooth[i].setFrequency(note_to_freq(base_note + scale[i]));
        square[i].setFrequency(note_to_freq(base_note + scale[i]));
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

    thread main_menu (menu_loop);

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

        if (change == 1) {
            for (int i = 0; i < 12; i++) {
                sines[i].setFrequency(note_to_freq(base_note + scale[i]));
                sawtooth[i].setFrequency(note_to_freq(base_note + scale[i]));
                square[i].setFrequency(note_to_freq(base_note + scale[i]));
            }
            change = 0;
        }
            
        

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
        switch (oscillator_number) {
            case 0:
                for (i = 0; i < 12; i++) {
                    sample += sines[i].tick() * envs[i].tick();
                }
                break;
            case 1:
                for (i = 0; i < 12; i++) {
                    sample += sawtooth[i].tick() * envs[i].tick();
                }
                break;
            case 2:
                for (i = 0; i < 12; i++) {
                    sample += square[i].tick() * envs[i].tick();
                }
                break;
        }
        
        sample = sample * env.tick() * (velocity * velo_env.tick())/128.0 ;
        dac.tick(sample);
    }

    cleanup:

    delete midiin;
    return 0;
}