# FingerSynth

A fun and portable synthesizer that can be played with the use of a single finger, and which allows a variety of controls for the synthesized sound.

![Alt text](images/fingersynth.png?raw=true "FingerSynth Menu")

## Introduction
Interestingly enough, similarly to my MUMT306 project, the inspiration for this project also has ties to Daft Punk's music. I was scrolling through Instagram one day when I stumbled upon someone suggesting to check out a video of someone recreating some of Daft Punk's tracks using this small pocket synthesizer called a 'Stylophone'. The link to the video can be found [here](https://www.youtube.com/watch?v=dllfRLUX5xk). I found the stylophone to be an expremely cool invention, and in particular I thoroughly enjoyed what the creator *maromaro1337* did with the invention.

Consequently, while I was initially settled on creating some sort of plug in for a DAW of some kind, I quickly saw that with the tools presented in MUMT307 throughout the semester I could attempt to recreate a simiar and perhaps enhanced version of instrument in pure digital format. I found that the Synthesis ToolKit provides more than enough functionality to make the implementation of such a tool possible, and therefore I quickly decided to change my topic idea. The following sections describe the project's technical details, gives insight into the development process and highlights areas of improvement and paths further exploration and development of the project.

## Project Overview and Development
The entirety of the program runs in a single C++ program. The very first element that needed to be implemented was the ability to read some input data from the user, and transform this data into a synthesized sound. The most intuitive way to do so at first was to try to read keyboard inputs, in order to synthesize sounds. This did not go too well, as there are not many good C++ libraries that allow for processing of keystroked in a way that is optimal for simulation of an instrument. Namely, we wanted to have insight into exactly when a key was hit/released in real time, and this proved to be a more difficult task in C++ than I first anticipated. 

I proceeded to explore the possibilities of using the keyboard or trackpad to generate midi messages, which would allow for an effective use of the RtMidi class. It was in this process that I stumbled upon a software called [AudioSwift](https://audioswiftapp.com/). This software truly helped bridge the gap between the input data from the trackpad, and the receival of data by FingerSynth. In order to be properly operate with FingerSynth, AudioSwift is configured to operate in XY mode (on of its five modes of operation). While this mode is initially designed to send Control Change messages to MIDI devices, I decided that the nature of the messages was the most applicable to FingerSynth. There are three types of messages sent based on trackpad gestures, labelled by their message number 1, 2 or 3:

1. Massage type 1: This indicates a change in the position of the finger in the X direction on the trackpad, and the values sent vary between 0 and 127. If we slide the finger horizontally from left to right on the trackpad at the same height, a series of type 1 messages will be sent in increasing order from 0 to 127. 
1. Massage type 2: This message function is analogous to Message number 1, except that it works vertically, with the increase happening from the bottom to the top of the trackpad. 
1. Message type 3: This message is sent to indicate the presence of the finger on the trackpad. If the user lays their finger on the trackpad, a type 3 message with value 127 will be sent. If the user releases their finger, a type message will be sent with value 0.

As it can probably be seen, at this point the interpretation of these messages was left to the developer, and is done within the program.

## Program Structure

Now that the mode of input is clarified, it is important to give a good overview of the way the program is structured. There are two main components of FingerSynth that need to be described. Firstly, there is the *backend* synthesis program, that reads MIDI messages from AudioSwift and transforms them into corresponding data sent to the DAC. Secondly there is the *frontend* menu functionality which provides the user with a visual interface, as well as the possibility to change certain features of FingerSynth.

Since both of these features require continuous monitoring of user input, yet have completely different functionalities, a two threaded structure was required for the program. The main thread takes care of the synthesis, and operates in a perpetual while loop reading and interpreting user data, but before entering the while loop it lanches the menu functionality in a separate thread. Both threads have access to the same shared variables, the inclusion of mutual exclusion in this case seemed like a bit of an overkill, since synthesis, and choice selection never happen simultaneously. The following subsections describe the functionalities of the individual threads/loops.

### Synthesis/Main Loop
The program goes through the usual set up for standard STK program, with a default sampling rate set to 48000Hz. For this project, a decision was made to (virtually) divide the trackpad into 12 even sections from left to right, such that 12 separate tones could be generated. Consequently, for any set up, 12 separate notes can be played using the FingerSynth. This led me to use an array of 12 oscillators to generate sounds, with each oscillator set to a given frequency. Since the synthesis happens in real time, it was vital to keep the number of computations per iteration of the synthesis loop to a minimum, and therefore I chose to avoid having to use `setFrequency()` each time a note number was changed. Consequenly 12 separate oscillators are initialized each to a given frequency corresponding to a note on the given scale. There is a choice of 3 different oscillators (Sine, SawTooth and Square), and therefore a 12 element array is initialized for each. A global variable called `oscillator_number`, set by default to zero (Sine), defines which oscillator is currently in use.

Subsequently, the array is initialized with the frequencies that we choose for each of the notes. This is based on the choice of the key, as well as the choice of a scale. There are 6 options for the scale type, namely:
1. Major Scale 
1. Minor Scale
1. Major Pentatonic Scale
1. Minor Pentatonic Scale
1. Minor Blues Scale
1. Chromatic Scale
For each scale, and array of 12 integers is defined to identify the difference between the base note, and 12 consecutive notes, following that scale starting at the base note. The default scale is the Major scale.

The key is defined by the MIDI note number of the note that we wish our scale to start on. It is a value that can vary between 20 and 100. The default note number is 57, which correspnds to the A just below middle C. Based on the note number, and the array of note differences, each oscillator in the array of oscillators can be initialized to the desired frequencies. 

For each oscillator, a separate envelope was also created, as well as a global envelope to control the overall output of the oscillators. For simplicity, all the envelopes were initialized to have a fixed ramp time of 0.01 seconds, and their goal is mainly to provide smooth cross fading between sine tones, which helps avoid 'ticks' commonly heard when a signal onset is too instantaneous. 

When the user presses their finger on the trackpad, the global envelope is turned on using the `keyOn()` function. The note number is calculated based on the horizontal position of the finger, and its corresponding envelope is also turned on. Finally the velocity of the note is calculated based on the height of the finger on the trackpad. A single sample is calculated by taking the sum over all the frequencies of the product of `oscillator[i].tick()` and `envelope[i].tick()`, and multiplying everything by the global envelope, as well as the velocity. We have that at any time only one envelope will have a note on, yet the sum needs to be taken over all tones, to allow the simultaneous onset and decay of two consecutive tones. Here `oscillator` refers to one of the three possible arrays of oscillators, and `envelope` to the corresponding array of envelopes for each oscillator. 

If a note changes, which happens when the user slides the finger to a different horizontal position, the envelope for the previous note is told to ramp down using `keyOff()`, and for the new note the envelope is turned on. This allows for smooth cross-fading between notes, without the unpleasant ticks. 

Finally at each iteration, the loop checks if the shared variable `change` was motified to 1. This signals that the user made changes to the synthesis configuration using the menu, and that the settings of the synthesizer need to be changed. The program proceeds to update its oscillators to new frequencies, reflecting said changes.

Despite the fact that the main loop uses the blocking functionality of the RtAudio class, thanks to the fact that the number of operations withing each loop iteration was kept to a minimum, the resulting sound is smooth and very fun to play around with.

### Menu Loop

The main menu loop incorporates features which are designed to provide an optimal user experience. It runs in parallel to the loop that synthesizes the sounds, which allowed me to be more free with regards to the amount of computations that occur within this branch of the program. The main element that this function incorporates, which allows us to display text onto the screen in such a free manner, is the `ncurses` library. Through this, the program interprets the actions of the user from the keypad, highlights the correct areas of the screen and makes the appropriate changes based on the selection selection. The details of the implementations of the display are left out since they are beyond the scope of the main focus of this project. 

The central functionality of the menu is to adjust the variables (shared with the synthesis thread) which define the synthesis configuration in accordance to the selections the user makes (using up-down-left-right arrow keys and `enter`). Once a user presses enter, it first makes the changes the variable defining to the type of oscillator, the base note or the scale type, following which it changes a global variable called `change` which will make the main synthesizing process realize that a change has been made, and in turn make it reconfigure the synthesizer.

## Areas for further Research and Development

While the project at the moment provides a very well rounded synthesizer with a lot of functionalities and possibilities, it is important to note that there are certain aspects that I wish to be able to explore more and for which I unfortunately didn't have the time to develop in enough depth. The main function of course is the addition of filters and effects to the synthesized sounds. When I initially started out on the project I envisioned the menu containing the filter/effect element, which allows for the choice from a variety of options, and the possibility to adjust the given filter/effect parameters. While the current menu reflects the desire to do so, for now it is purely visual, and unfortunately the synthesizer doesn't incorporate any possibility right now to create filtered sounds. 
