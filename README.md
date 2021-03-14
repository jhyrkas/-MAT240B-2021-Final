# -MAT240B-2021-Final

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
- Alloblib (https://github.com/AlloSphere-Research-Group/allolib): downloaded locally, used to compile the application

A version of dr_wav.h from dr_libs (https://github.com/mackron/dr_libs) is included in this repo, but a future version would (should)
depend on this an installation of this library.

Once all prerequisites have been satisfied, the application can be built using Allolib's run.sh script:

>> run.sh -n /path/to/MAT240B-2021-Final/analysis-resynthesis.cpp

(run.sh -n builds an allolib application without launching)


## Usage

TODO

### Usage tips

TODO

## Standalone audio transport interpolation

The separate, standalone application transport_standalone can be built similarly

>> run.sh -n /path/to/MAT240B-2021-Final/transport_standalone.cpp

This application is similar to a standalone audio transport application (https://github.com/sportdeath/audio_transport/blob/master/example/transport.cpp), but produces audio at a fixed interpolation value between the two target audio sources, rather than an interpolation. It can be run as:

>> transport_standalone in1.wav in2.wav interp_amount out.wav

With interp_amount in [0.0-1.0]. All wav files are assumed to be sampled at 48 kHz. This application is very bare-bones and serves a specific purpose to the author, but is free to use on top of the analysis/resynthesis application.
