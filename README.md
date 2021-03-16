# Interstrapolator

This repository contains source code for an Allolib (https://github.com/AlloSphere-Research-Group/allolib) application to 
interpolate and extrapolate sounds using a sinusoidal synthesis method and using the Audio Transport algorithm  (https://github.com/sportdeath/audio_transport).

The source code is currently licensed under the BSD-3 Clause. Sounds created using this library are licensed
under the Creative Commons Attribution-NonCommercial 4.0 International License (CC BY-NC 4.0).

Allolib uses the BSD-3 Clause (https://github.com/AlloSphere-Research-Group/allolib/blob/master/LICENSE)
The Audio Transport library uses the GPL-3.0 License (https://github.com/sportdeath/audio_transport/blob/master/LICENSE).
No modifications to the core Allolib or Audio Transport libraries are present in this application.

## Build instructions and requirements

This application depends on:
- audiorw (https://github.com/sportdeath/audiorw): installed as a library 
- audio_transport (https://github.com/sportdeath/audio_transport): installed as a library
- nativefiledialog (https://github.com/kybr/nativefiledialog): installed locally (run the get-and-build-file-dialog shell script)
- Alloblib (https://github.com/AlloSphere-Research-Group/allolib): downloaded locally, used to compile the application

A version of dr_wav.h from dr_libs (https://github.com/mackron/dr_libs) is included in this repo, but a future version would (should)
depend on this an installation of this library.

Once all prerequisites have been satisfied, the application can be built using Allolib's run.sh script:

> run.sh -n /path/to/MAT240B-2021-Final/analysis-resynthesis.cpp

(run.sh -n builds an allolib application without launching)


## Usage

Once build, the application can be launched from the command line using the following command:

> analysis_resynthesis in1.wav in2.wav num-sinusoids

where num-sinusoids is a positive integer specifying the number of sinusoidal peaks that will be used to resynthesize the target sounds
when using sinusoidal modeling (you may experiment with this number, although it has not been tested about 64). Audio files are assumed to be
sampled at 48 kHz.

Once launched, the UI will allow you to change the interpolation amount, audio gain, synthesis method, files used, and toggle between
a fixed interpolation amount and the use of an interpolation path.

A brief description of audio parameters:
- background: changes the lightness or darkness of the background
- interstrapolation: interpolates or extrapolates between the target audio sounds. 0.5 is 50% between input1 and input 2. 1.5 is 50% past input 2 in the direction away from input1. This can be very abstract, but it's fun to play with. The audio transport algorithm only supports interpolation.
- amplitude: changes the volume of the audio playback
- Pick audio files: pressing this button allows you to change the files you are interstrapolating between. Audio synthesis method is set back to sinusoidal by default.
- Change Synthesis Method: Toggles between sinusoidal modeling and audio transport. Audio transport is not calculated dynamically, so if you change parameters, you will have to toggle back and forth again to change the audio transport sound. (Alternatively, typing '1' changes the synthesis method to sinusoidal and typing '2' changes the method to audio transport and recalculates audio based on the current parameters)
- Use Interpolation Path: clicking outside of the ControlGUI window lets you define an interpolation path. By definition, the lowest point drawn will be scaled to 0.0 and the highest point drawn will be scaled to 1.0. The first and last points horizontally will be scaled to the beginning and end of the audio, respectively. When at least two points are present, toggling the Use Interpolation Path button will change the audio to read from the interpolation path instead of the interpolation slider. NOTE: when changing the interpolation path for audio transport, you will need to type 2 or toggle the Change Synthesis Method button to hear changes.

### Usage tips

- Audio will be shortened to the length of the shortest audio clip. No attempt at onset matching is provided
- Matching like sounds can be interesting (for example, morphing between instruments, voices, instruments and voices, or similar sound effects)
- Extrapolating moves matched sinusoids away from each other and also approximately inverts the difference in amplitude envelopes of sounds
- Have fun and watch your ears!

## Standalone audio transport interpolation

The separate, standalone application transport_standalone can be built similarly

> run.sh -n /path/to/MAT240B-2021-Final/transport_standalone.cpp

This application is similar to a standalone audio transport application (https://github.com/sportdeath/audio_transport/blob/master/example/transport.cpp), but produces audio at a fixed interpolation value between the two target audio sources, rather than a portamento. It can be run as:

> transport_standalone in1.wav in2.wav interp_amount out.wav

With interp_amount in [0.0-1.0]. All wav files are assumed to be sampled at 48 kHz. This application is very bare-bones and serves a specific purpose to the author, but is free to use on top of the analysis/resynthesis application.
