# Finger Synth

A fun and portable synthesizer that can be played with the use of a single finger, and which allows a variety of controls for the synthesized sound.

![Alt text](images/fingersynth.png?raw=true "Finger Synth Menu")

## Introduction
Interestingly enough, similarly to my MUMT306 project, the inspiration for this project also has ties to Daft Punk's music. It was while scrolling through instagram one day that I stumbled someone suggesting to check out a video of someone recreating some of Daft Punk's tracks using this small pocket synthesizer called a 'Stylophone'. The link to the video can be found [here](https://www.youtube.com/watch?v=dllfRLUX5xk). I found the stylophone to be an expremely cool invention, and especially I thoroughly enjoyed what the creator *maromaro1337* did with the invention.

Consequently, while I was initially settled on creating some sort of plug in for a DAW of some kind, I quickly saw that with the tools presented in MUMT307 throughout the semester I could attempt to recreate a simiar and perhaps enhanced version of instrument in pure digital format. I found that the Synthesis ToolKit provides more than enough functionality to make the implementation of such a tool possible, and therefore I quickly decided to change my topic idea. The following sections describe the project's technical details, gives insight into the development process and highlights areas of improvement and paths further exploration and development of the project.

## Project Overview and Development
The entirety of the program runs on a single c++ program. The very first element that needed to be implemented was the ability to read some inpud data from the user, and transform this data into a synthesized sound. The most intuitive way to do so at first was to try to read keyboard inputs, in order to synthesize sounds. This did not go too well, as there are not many good c++ libraries that allow for processing of keystroked in the desired way. Namely, we wanted to have insight into exactly when a key was hit/released in real time, and this proved to be a more difficult task in c++ than I anticipated. 

I proceeded to into the possibilities of using the keyboard or trackpad to generate midi messages, which then would be read using the RtMidi class. It was in this process that I stumbled upon a software called [AudioSwift](https://audioswiftapp.com/). This software truly helped bridge the gap between the input data from the trackpad, and the receival of data by Finger Synth. In order to be properly operate with Finger Synth, AudioSwift is configured to operate in XY mode (on of its five modes of operations). While this mode is initially designed to send Control Change messages to MIDI devices, I decided that the nature of the messages was the most applicable to Finger Synth. There are three types of messages sent based on trackpad gestures, labelled by their message number 1, 2 or 3:

1. Massage type 1: This indicates a change in the position of the finger in the X direction on the trackpad, and the values sent vary between 0 and 127. If we slide the finger horizontally from left to right on the trackpad at the same height, a series of those messages will be sent in increasing order from 0 to 127. 
1. Massage type 2: This message function is analogous to Message number 1, except this time in the Y direction, with the increase happening from the bottom to the top of the trackpad. 
1. Message type 3: This message is sent to indicate the presence of the finger on the trackpad. If the user lays their finger on the trackpad, a type 3 message with value 127 will be sent. If the user releases their finger, a type message will be sent with value 0.
As it can probably be seen, at this point the interpretation of these messages was left to the developer, and is done within the program.

## Program Structure

Now that the mode of input is clarified, it is important to give a good overview of the way the program is structured. There are two main components of Finger Synth that need to be described. Firstly, there is the *backend* synthesis program, that read input from the trackpad and transforms it into corresponding data sent to the DAC, and there is the *frontend* menu functionality which gives provides the used with a visual interface, as well as the possibility to change certain features of Finger Synth.

Since both of these features require continuous monitoring of user input, yet have completely different functionalities, a two threaded structure was required for the program. The main process takes care of the synthesis, is in a constant while loop reading and interpreting user data, but before entering the while loop it lanches the menu functionality in a separate thread. Both threads have access to the same shared variables. The following subsections describe the functionalities of the individual threads/loops.

### Synthesis Loop

### Menu Loop


