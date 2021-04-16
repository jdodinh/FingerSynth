//*****************************************//
//  qmidiin.cpp
//  by Gary Scavone, 2003-2004.
//
//  Simple program to test MIDI input and
//  retrieval from the queue.
//
//*****************************************//

// ./qmidiin

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include "RtMidi.h"

// Platform-dependent sleep routines.
#if defined(__WINDOWS_MM__)
#include <windows.h>
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds )
#else // Unix variants

#include <unistd.h>

#define SLEEP(milliseconds) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

bool done;

static void finish(int /*ignore*/) { done = true; }


void usage(void) {
    // Error function in case of incorrect command-line
    // argument specifications.
    std::cout << "\nusage: qmidiin <port>\n";
    std::cout << "    where port = the device to use (default = 0).\n\n";
    exit(0);
}

int main(int argc, char *argv[]) {
    RtMidiIn *midiin = 0;
    std::vector<unsigned char> message;
    int nBytes;
    double stamp;

    std::string channel_messages[7] = {"Note Off", "Note On", "Aftertouch", "Control Change", "Program Change",
                                       "Channel Pressure", "Pitch Bend"};
    std::string channel_messages_b1[7] = {"Note Number: ", "Note Number: ", "Note Number: ", "Controller Number: ",
                                          "Program Number: ", "Pressure Value: ", "Value LSB: "};
    std::string channel_messages_b2[7] = {"Note Velocity: ", "Note Velocity: ", "Key Pressure Value: ", "Control Value: ", "None",
                                          "None", "Value MSB: "};

    std::string system_messages[16] = {"System Exclusive","Time Code Qtr Frame","Song Position","Song Select",
                                       "Undefined","Undefined","Tune Request", "End of Sysex", "Timing Tick", "Undefined",
                                       "Start Song", "Continue Song", "Stop Song", "Undefined", "Active Sensing", "System Reset"};

    std::string system_messages_b1[16];
    std::string system_messages_b2[16];
    system_messages_b1[1] = "Time Code Value: ";
    system_messages_b1[2] = "Position LSB: ";
    system_messages_b1[3] = "Song Number: ";
    system_messages_b2[2] = "Position MSB: ";
    // Minimal command-line check.
    if (argc > 2) usage();

    // RtMidiIn constructor
    try {
        midiin = new RtMidiIn();
    }
    catch (RtMidiError &error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }

    // Check available ports vs. specified.
    unsigned int port = 0;
    unsigned int nPorts = midiin->getPortCount();
    if (argc == 2) port = (unsigned int) atoi(argv[1]);
    if (port >= nPorts) {
        delete midiin;
        std::cout << "Invalid port specifier!\n";
        usage();
    }

    try {
        midiin->openPort(port);
    }
    catch (RtMidiError &error) {
        error.printMessage();
        goto cleanup;
    }

    // Don't ignore sysex, timing, or active sensing messages.
    midiin->ignoreTypes(false, false, false);

    // Install an interrupt handler function.
    done = false;
    (void) signal(SIGINT, finish);
    int status;
    // Periodically check input queue.
    std::cout << "Reading MIDI from port " << midiin->getPortName(port) << " ... quit with Ctrl-C.\n";
    int mes_num;
    int channel;
    int ind;
    int counter = 0;
    while (!done) {
        stamp = midiin->getMessage(&message);
        nBytes = message.size();
        if (nBytes > 0) {
            status = ((int) message[0]);
            if (status < 240) {
                mes_num = status / 16;
                channel = status % 16;
                ind = mes_num - 8;
                std::cout << "Channel Message: \nMessage Name: " << channel_messages[ind] << "\t" << "Channel Number: "
                          << channel << "\t" << channel_messages_b1[ind] << (int) message[1] << "\t";
                if (nBytes > 2) {
                    std::cout <<"\t"<< channel_messages_b2[ind] << (int) message[2] << "\t";
                }
                if (nBytes > 0) {
                    std::cout << "\t stamp = " << stamp << std::endl<<"\n";
                }
            }
            if (status < 256 && status >= 240) {
                mes_num = status - 240;
                if (mes_num == 0) {
                    std::cout << "System Exclusive Message: \n";
//                    std::cout << nBytes;
                }
                else if (mes_num < 8) {
                    std::cout << "System Common Message: \n";
                }
                else if (mes_num < 16) {
                    std::cout << "System Real Time Message: \n";
                }
                std::cout << "Message Name: " << system_messages[mes_num] << "\t";
                if (nBytes > 1) {
                    std::cout << system_messages_b1[mes_num] << (int)message[1] << "\t";
                }
                if (nBytes > 2) {
                    if (mes_num == 0) {
                        std::cout << "Subsequent sysex bytes: ";
                        for (int j = 2; j<nBytes; j++) {
                            std::cout << (int)message[j] << "\t";
                        }
                    }
                    else {
                        std::cout << system_messages_b2[mes_num] << (int)message[2] << "\t";
                    }
                }
                if (nBytes > 0) {
                    std::cout << "\t stamp = " << stamp << std::endl<< "\n";
                }
            }
        }

//    for ( i=0; i<nBytes; i++ )
//      std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
//    if ( nBytes > 0 )
//      std::cout << "stamp = " << stamp << std::endl;

        // Sleep for 10 milliseconds.
        SLEEP(10);
    }

    // Clean up
    cleanup:
    delete midiin;

    return 0;
}
