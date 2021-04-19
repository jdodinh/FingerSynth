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

### Synthesis/Main Loop
The program goes through the usual set up for standard STK program, with a default sampling rate set to 48000Hz. For this project, a decision was made to (virtually) divide the trackpad into 12 even sections from left to right, such that 12 separate tones could be generated. Consequently, for any set up, separate notes can be played using the FingerSynth. This led me to use an array of 12 oscillators to generate sounds, with each oscillator set to a given frequency. Since the synthesis happens in real time, it was vital to keep the number of computations per iteration of the loop to a minimum, and therefore I chose to avoid having to use `setFrequency()` each time a note number was changed. There is a choice of 3 different oscillators (Sine, SawTooth and Square), and therefore a 12 element array is initialized for each. A global variable called `oscillator_number`, set by default to zero, defines which oscillator is currently in use.

Subsequently, the array is initialized with the frequencies that we choose for each of the notes. This is based on the choice of the key, as well as the choice of a scale. There are 6 options for the scale type, namely:
1. Major Scale 
1. Minor Scale
1. Major Pentatonic Scale
1. Minor Pentatonic Scale
1. Minor Blues Scale
1. Chromatic Scale
For each scale, and array of 12 number is defined to identify the difference between the base note, and 12 subsequent notes starting at the base note, following that scale. The default scale is the Major scale. 

The key is defined by the midi note number of the note that we wish for our scales to start on. It is a value that can vary between 20 and 100. The default note number is 57, which correspnds to the A just below middle C. Based on the note number, and the array of note differences, each oscillator in the array of oscillators can be initialized to the desired frequencies. 

For each oscillator, a separate envelope was also created, as well as a global envelope to control the overall output of the oscillators. For simplicity, all the envelopes were nitialized to have a fixed ramp time of 0.01 seconds, and their goal is mainly to provide smooth cross fading between sine tones, which helps avoid 'ticks' commonly heard when a signal onset is too brutal. 

When the user presses their finger on the trackpad, the global envelope is turned on using the `keyOn()` function. The note number is calculated based on the horizontal position of the finger, and its envelope is also turned on. Finally the velocity of the note is calculated based on the height of the finger on the trackpad. A single sample is calculated by taking the sum over all the frequencies of the product of `oscillator[i].tick()` and `envelope[i].tick()`, and multiplying everything by the global envelope, as well as the velocity. Here `oscillator` refers one of the three possible arrays of oscillators, and `envelope` to the corresponding array of envelopes for each oscillator. 

If a note changes, which happens when the user slides the finger to a different horizontal position, the envelope for the previous note is told to ramp down using `keyOff()`, and for the new note the envelope is turned on. This allows for smooth cross-fading between notes, without the unpleasant ticks. 

Finally at each iteration, the loop checks if a shared variable valled `change` was motified to 1. This indicated that the used made changes using the menu, and that the setting of the synthesizer need to be changed. The program proceeds to update its oscillators to new frequencies, since both the key or the scale could've been changed. 

### Menu Loop


