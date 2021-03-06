// g++ -o fingersynth -Istk/include/ -Lstk/src/ -D__MACOSX_CORE__ fingersynth.cpp -lstk -lncurses -lpthread -framework CoreAudio -framework CoreMIDI -framework CoreFoundation
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
#include <unistd.h> /* only for sleep() */
#include <numeric>
#include <thread>
#include <string>
#include <Brass.h>
#include <Echo.h>
#include <BlitSaw.h>
#include <BlitSquare.h>
#include <fstream>


using namespace stk;
using namespace std;
#define SLEEP(milliseconds) usleep((unsigned long)(milliseconds * 1000.0))

// Defining MIDI Note intervals for particular scales
int major_scale[12] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19};
int minor_scale[12] = {0, 2, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19};
int major_pentatonic[12] = {0, 2, 4, 7, 9, 12, 14, 16, 19, 21, 24, 26};
int minor_pentatonic[12] = {0, 3, 5, 7, 10, 12, 15, 17, 19, 22, 24, 27};
int minor_blues[12] = {0, 3, 5, 6, 7, 10, 12, 15, 17, 18, 19, 22};
int chromatic[12] = {0,1,2,3,4,5,6,7,8,9,10,11};
int scale[12] = {};

int change = 0; // Variable that indicates if a change in any of the device settings was made

int col_max[4] = {2, 2, 2, 2}; // Array that shows how many options we have in a given menu setting
int num_options = 4;

int base_note = 57;        // Variable defining the base note
int scale_number = 0;      // Variable defining the Scape type used. Default: Major
int oscillator_number = 0; // Variable defining the name of the oscillator used. Default: SineWave
char scale_names[6][20] = {"Major", "Minor", "Maj Pen", "Min Pen", "Min Blu", "Chrom"};
char oscillators[3][20] = {"Sine", "Sawth", "Square"};
int yMax, xMax;
int column_width;
const int text_width = 48;

bool done;

float note_to_freq(int note_num)
{
    /*  HELPER FUNCTION
        INPUT: - int note_num: MIDI note number
        OUTPUT: float - frequency of the given note
      */
    int diff = note_num - 69;
    return 440.0 * pow(2.0, ((float)diff) / 12.0);
}

int print_logo(int logo_num, int column_width, WINDOW *w, bool highlight)
{
    /* Helper function that prints fonts to the screen */
    char line[text_width];
    ifstream inFile;
    std::string str;
    int j;
    if (logo_num == 0)
    {
        inFile.open("./logos/tone.txt");
    }
    else if (logo_num == 1)
    {
        inFile.open("./logos/scale.txt");
    }
    else if (logo_num == 2)
    {
        inFile.open("./logos/oscillator.txt");
    }
    else if (logo_num == 3)
    {
        inFile.open("./logos/filter.txt");
    }
    if (highlight)
    {
        wattron(w, A_STANDOUT);
    }
    else
    {
        wattroff(w, A_STANDOUT);
    }
    j = 10;
    while (getline(inFile, str))
    {
        sprintf(line, "%-7s", "                              ");
        sprintf(line, "%-7s", str.c_str());
        mvwprintw(w, j, logo_num * column_width + (column_width - text_width) / 2, "%s", line);
        j++;
    }
    wattroff(w, A_STANDOUT);
    return j;
}

int print_value(int value_num, int val, int column_width, WINDOW *w, bool highlight)
{
    /* Helper function that prints fonts to the screen */
    char line[text_width];
    char blank[text_width] = "                                               ";
    char command[256];
    int j;
    std::string str;

    ifstream inFile;
    if (value_num == 0)
    {
        if (val >= 0)
        {
            sprintf(command, "figlet -w 48 -d ./logos/fonts/ -f slant '%d' > ./logos/note_num.txt", val);
            system(command);
        }
        inFile.open("./logos/note_num.txt");
    }
    else if (value_num == 1)
    {
        if (val >= 0)
        {
            sprintf(command, "figlet -w 48 -d ./logos/fonts/ -f slant '%s' > ./logos/scale_name.txt", scale_names[val]);
            system(command);
        }
        inFile.open("./logos/scale_name.txt");
    }
    else if (value_num == 2)
    {
        if (val >= 0)
        {
            sprintf(command, "figlet -w 48 -d ./logos/fonts/ -f slant '%s' > ./logos/osc_name.txt", oscillators[val]);
            system(command);
        }
        inFile.open("./logos/osc_name.txt");
    }
    else if (value_num == 3)
    {
        if (val >= 0)
        {
            sprintf(command, "figlet -w 48 -d ./logos/fonts/ -f slant '%s' > ./logos/osc_name.txt", oscillators[val]);
            system(command);
        }
        inFile.open("./logos/filter_name.txt");
    }

    if (highlight)
    {
        wattron(w, A_STANDOUT);
    }
    else
    {
        wattroff(w, A_STANDOUT);
    }
    j = 15;
    while (getline(inFile, str))
    {
        if (highlight)
        {
            wattroff(w, A_STANDOUT);
            mvwprintw(w, j, value_num * column_width + (column_width - text_width) / 2, "%s", blank);
            wattron(w, A_STANDOUT);
        }
        else
        {
            mvwprintw(w, j, value_num * column_width + (column_width - text_width) / 2, "%s", blank);
        }
        mvwprintw(w, j, value_num * column_width + (column_width - text_width) / 2 , "%s", blank);
        sprintf(line, "%-7s", str.c_str());
        mvwprintw(w, j, value_num * column_width + (column_width - text_width) / 2 + (text_width - str.length()) / 2, "%s", line);
        j++;
    }
    wattroff(w, A_STANDOUT);
    return j;
}

int change_tonic(int i, int j, WINDOW *w)
{
    /* Helper function that allows the menu choice of a different base note, 
    and changes that note once the user presses enter */
    int current_note = base_note;
    char item[20], ch;
    while ((ch = wgetch(w)) != 10)
    {
        switch (ch)
        {
        case (char)KEY_RIGHT:
            current_note++;
            if (current_note > 100)
            {
                current_note = 100;
            }
            break;
        case (char)KEY_LEFT:
            current_note--;
            if (current_note < 20)
            {
                current_note = 20;
            }
            break;
        }
        print_value(i, current_note, column_width, w, true);
    }
    base_note = current_note;
    wattroff(w, A_STANDOUT);
    return 1;
}

int change_scale(int i, int j, WINDOW *w)
{
    /* Helper function that allows the menu choice of a different scale note, 
    and changes that scale once the user presses enter */
    int current_scale = scale_number;
    char item[20], ch;
    while ((ch = wgetch(w)) != 10)
    {
        switch (ch)
        {
        case (char)KEY_RIGHT:
            current_scale++;
            if (current_scale > 5)
            {
                current_scale = 0;
            }
            break;
        case (char)KEY_LEFT:
            current_scale--;
            if (current_scale < 0)
            {
                current_scale = 5;
            }
            break;
        }
        print_value(i, current_scale, column_width, w, true);
    }
    scale_number = current_scale;
    switch (scale_number)
    {
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
    case 5:
        memcpy(scale, chromatic, sizeof(scale));
        break;
    }
    wattroff(w, A_STANDOUT);
    return 1;
}

int change_oscillator(int i, int j, WINDOW *w)
{
    /* Helper function that allows the menu choice of a oscillator base note, 
    and changes that oscillator once the user presses enter */
    int current_oscillator = oscillator_number;
    char item[20], ch;
    while ((ch = wgetch(w)) != 10)
    {
        switch (ch)
        {
        case (char)KEY_RIGHT:
            current_oscillator++;
            if (current_oscillator > 2)
            {
                current_oscillator = 0;
            }
            break;
        case (char)KEY_LEFT:
            current_oscillator--;
            if (current_oscillator < 0)
            {
                current_oscillator = 2;
            }
            break;
        }
        print_value(i, current_oscillator, column_width, w, true);
    }
    oscillator_number = current_oscillator;
    wattroff(w, A_STANDOUT);
    return 0;
}

void menu_loop()
{

    system("figlet -w 48 -d ./logos/fonts/ -f slant '57' > ./logos/note_num.txt");
    system("figlet -w 48 -d ./logos/fonts/ -f slant Major > ./logos/scale_name.txt");
    system("figlet -w 48 -d ./logos/fonts/ -f slant Sine > ./logos/osc_name.txt");

    char item[20], ch, line[100];
    std::string str;
    char options[4][20] = {"Tonic Note", "Scale Type", "Oscillator", "Filter"};
    sprintf(item, "%d", base_note);
    char values[4][20] = {"57", "Major", "SineWave", "None"};

    int i = 0;
    int j = 0;
    int width, height;
    char list[5][20] = {"One", "Two", "Three", "Four", "Five"};
    bool highlight = false;

    /* INIT NCURSES */
    initscr();
    // Get screen size
    getmaxyx(stdscr, yMax, xMax);

    width = xMax - 10;
    height = yMax - 4;
    column_width = width / 4;
    // Create window for input
    WINDOW *w;
    w = newwin(yMax/2, xMax - 10, 2, 5);
    box(w, 0, 0);
    j = 2;
    noecho();
    ifstream inFile;
    inFile.open("logos/main_logo.txt");

    // Print the main logo
    while (getline(inFile, str))
    {
        sprintf(line, "%-7s", str.c_str());
        mvwprintw(w, j, (width - 100) / 2, "%s", line);
        j++;
    }
    inFile.close();
    wrefresh(w);

    // Print the menu options
    j = 10;
    for (i = 0; i < num_options; i++)
    {
        if (i == 0)
            highlight = true;
        else
        {
            highlight = false;
        }
        j = print_logo(i, column_width, w, highlight);
        print_value(i, -1, column_width, w, false);
    }

    wrefresh(w); // update the terminal screen

    noecho();        // disable echoing of characters on the screen
    keypad(w, true); // enable keyboard input for the window.
    curs_set(0);     // hide the default screen cursor.
    i = 0;
    j = 0;

    // Loop responsible for user input and readjusting parameters. 
    while ((ch = wgetch(w)) != 113)
    {
        print_logo(i, column_width, w, false);
        print_value(i, -1, column_width, w, false);
        switch (ch)
        {
        case (char)KEY_UP:
            j--;
            if (j < 0)
            {
                j = 0;
            };
            break;
        case (char)KEY_DOWN:
            j++;
            if (j >= col_max[i])
            {
                j = col_max[i] - 1;
            }
            break;
        case (char)KEY_LEFT:
            if (j == 1)
            {
                break;
            }
            else
            {
                i--;
                if (i < 0)
                {
                    i = 0;
                }
                break;
            }
        case (char)KEY_RIGHT:
            if (j == 1)
            {
                break;
            }
            else
            {
                i++;
                if (i >= num_options)
                {
                    i = num_options - 1;
                }
                break;
            }
        }
        wattron(w, A_STANDOUT);
        if (j == 0)
        {
            print_logo(i, column_width, w, true);
        }
        else
        {
            if (i == 0)
            {
                print_value(i, base_note, column_width, w, true);
                change = change_tonic(i, j, w);
            }
            else if (i == 1)
            {
                print_value(i, scale_number, column_width, w, true);
                change = change_scale(i, j, w);
            }
            else if (i == 2)
            {
                print_value(i, oscillator_number, column_width, w, true);
                change = change_oscillator(i, j, w);
            }
        }

        wattroff(w, A_STANDOUT);
    }

    delwin(w);
    endwin();
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
    Envelope velo_env1;
    Envelope velo_env2;
    memcpy(scale, major_scale, sizeof(minor_pentatonic));

    env.setTime(0.01);
    velo_env1.setTime(0.01);
    velo_env2.setTime(0.01);
    env.keyOff();
    velo_env1.keyOff();
    velo_env2.keyOff();
    int env1_state = 1;
    for (int i = 0; i < 12; i++)
    {
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

    thread main_menu(menu_loop);

    try
    {
        midiin = new RtMidiIn();
    }
    catch (RtMidiError &error)
    {
        error.printMessage();
        exit(EXIT_FAILURE);
    }
    unsigned int port = 0;
    unsigned int nPorts = midiin->getPortCount();
    for (i = 0; i < nPorts; i++)
    {
        port_name = midiin->getPortName(i);
        if (port_name == "AudioSwift 3")
        {
            port = i;
        }
    }

    try
    {
        port_name = midiin->getPortName(port);
        midiin->openPort(port);
    }
    catch (RtMidiError &error)
    {
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

    while (!done)
    {
        if (change == 1)
        {
            for (int i = 0; i < 12; i++)
            {
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
        if (nBytes > 0)
        {
            mes = (int)message[1];
            val = (int)message[2];
            if (mes == 1)
            {
                note = floor(((float) val) / (10.59));
                if (old_note < 0)
                {
                    envs[note].keyOn();
                }
                else if (old_note != note)
                {
                    envs[old_note].keyOff();
                    envs[note].keyOn();
                }
                old_note = note;
            }
            else if (mes == 2)
            {
                velocity = (double)val/8.0;
                if (old_velo < 0.0)
                {
                    velo_env1.keyOn();
                }
                else if (velocity != old_velo)
                {
                    if (velo_env1.lastOut() == 1) {
                        velo_env1.keyOff();
                        velo_env2.keyOn();
                    }
                    else {
                        velo_env2.keyOff();
                        velo_env1.keyOn();
                    }
                    
                }
                old_velo = velocity;
            }
            else if (mes == 3)
            {
                if (val == 127)
                {
                    env.keyOn();
                }
                else
                {
                    env.keyOff();
                }
            }
            else
            {
                continue;
            }
        }
        switch (oscillator_number)
        {
        case 0:
            for (i = 0; i < 12; i++)
            {
                sample += sines[i].tick() * envs[i].tick();
            }
            break;
        case 1:
            for (i = 0; i < 12; i++)
            {
                sample += 0.5 * sawtooth[i].tick() * envs[i].tick();
            }
            break;
        case 2:
            for (i = 0; i < 12; i++)
            {
                sample +=  0.5 * square[i].tick() * envs[i].tick();
            }
            break;
        }

        sample = sample * env.tick() * (velocity * (velo_env1.tick()+velo_env2.tick())) / 16.0;
        dac.tick(sample);
    }

cleanup:

    delete midiin;
    return 0;
}