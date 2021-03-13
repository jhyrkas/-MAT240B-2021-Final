// MAT240B FINAL PROJECT
// interpolation and extrapolation app, based heavily on assignment 2
// and Audio Transport (https://github.com/sportdeath/audio_transport)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>  // std::sort
#include <cmath>      // ::sin()
#include <complex>
#include <exception>
#include <iostream>
#include <valarray>
#include <vector>

// audio transport stuff
#include "audio_transport/spectral.hpp"
#include "audio_transport/audio_transport.hpp"

#include "al/app/al_App.hpp"
#include "al/math/al_Functions.hpp"  // al::clip
#include "al/math/al_Random.hpp"
#include "al/math/al_Ray.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "dr_wav.h"
#include "interstrap_util.h" // load()
#include "Max-Function-Like/Function-From-Max.hpp" // drawing interpolation path
#include "nfd.h" // native file dialog
#include "stft_peaks.h" // stft peaks for sinusoidal model

//using namespace al;

// SYNTHESIS FUNCTIONS

// a class that encapsulates data approximating one cycle of a sine wave
//
struct SineTable {
  std::vector<double> data;
  SineTable(int n = 16384) {
    data.resize(n);
    for (int i = 0; i < n; i++) {
      data[i] = ::sin(M_PI * 2.0 * i / n);
      // printf("%lf\n", data[i]);
    }
  }
};

// a function that works with the class above to return the value of a sine
// wave, given a value **in terms of normalized phase**. p should be on (0, 1).
//
double sine(double p) {
  static SineTable table;
  int n = table.data.size();
  int a = p * n;
  int b = 1 + a;
  double t = p * n - a;
  if (b == n)  //
    b = 0;
  // linear interpolation
  return (1 - t) * table.data[a] + t * table.data[b];
}

// a constant global "variable" as an alternative to a pre-processor definition
const double SAMPLE_RATE = 48000.0;
//#define SAMPLE_RATE (48000.0)  // pre-processor definition

// a class using the operator/functor pattern for audio synthesis/processing. a
// Phasor or "ramp" wave goes from 0 to 1 in a upward ramping sawtooth shape. it
// may be used as a phase value in other synths.
//
struct Phasor {
  double phase = 0;
  double increment = 0;
  void frequency(double hz) {  //
    increment = hz / SAMPLE_RATE;
  }
  double operator()() {
    double value = phase;
    phase += increment;
    if (phase >= 1)  //
      phase -= 1;
    return value;
  }
};

// a class that may be used as a Sine oscillator
//
struct Sine : Phasor {
  double operator()() {  //
    return sine(Phasor::operator()());
  }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

using namespace al;

struct MyApp : App {
  Parameter background{"background", "", 0.0, "", 0.0f, 1.0f};
  Parameter interp_p{"interstrapolation", "", 0.0, "", -1.0f, 2.0f};
  Parameter amp{"amplitude", "", 0.5, "", 0.0f, 1.5f};
  ParameterBool pick_files{"Pick audio files", "", 0, "", 0, 1};
  ParameterBool change_playback{"Change Synthesis Method", "", 0, "", 0, 1};
  ControlGUI gui;
  FunctionFromMax functionFromMax{this}; // interpolation path

  int N; // the number of sine oscillators to use
  int s; // sample number
  int s_limit; // length of sample
  int frame_limit; // maximum frames (based on audio input length)
  int playback_mode; // sinusoid, audio transport, silent

  // synthesis params
  std::vector<Sine> sine;
  std::vector<std::vector<Entry>> peaks1;
  std::vector<std::vector<Entry>> peaks2;
  std::vector<std::vector<audio_transport::spectral::point>> points_left;
  std::vector<std::vector<audio_transport::spectral::point>> points_right;
  std::vector<double> transport_audio;

  MyApp(int argc, char *argv[]) {
    // C++ "constructor" called when MyApp is declared
    N = std::atoi(argv[3]);
    calculate_synthesis_features(argv[1], argv[2]);
  }

  // called from constructor and also when calculating new features for new files
  void calculate_synthesis_features(char* path1, char* path2) {
    std::vector<float> pSampleData1;
    load(pSampleData1, path1);
    std::vector<float> pSampleData2;
    load(pSampleData2, path2);

    // data
    peaks1 = stft_peaks(pSampleData1, N, SAMPLE_RATE);
    peaks2 = stft_peaks(pSampleData2, N, SAMPLE_RATE);

    // sinusoid peaks

    // deal with audio being different lengths, just take the min for now
    s = 0;
    frame_limit = std::min(peaks1.size(), peaks2.size());
    s_limit = frame_limit * 1024; // hop size
    playback_mode = 0;

    // need big time gain normalization here
    double max_amp1 = 0.0;
    for (int i = 0; i < peaks1.size(); i++) {
        for (int j = 0; j < N; j++) {
            max_amp1 = std::max(max_amp1, peaks1[i][j].amplitude);
        }
    }
    for (int i = 0; i < peaks1.size(); i++) {
        for (int j = 0; j < N; j++) {
            peaks1[i][j].amplitude /= max_amp1;
        }
    }
    double max_amp2 = 0.0;
    for (int i = 0; i < peaks2.size(); i++) {
        for (int j = 0; j < N; j++) {
            max_amp2 = std::max(max_amp2, peaks2[i][j].amplitude);
        }
    }
    for (int i = 0; i < peaks2.size(); i++) {
        for (int j = 0; j < N; j++) {
            peaks2[i][j].amplitude /= max_amp2;
        }
    }

    // audio transport

    // change?
    double window_size = 0.05; // seconds
    unsigned int padding = 7; // multiplies window size

    std::vector<double> transport_data1 = audio_transport_input(pSampleData1);
    std::vector<double> transport_data2 = audio_transport_input(pSampleData2);

    points_left = audio_transport::spectral::analysis(transport_data1, SAMPLE_RATE, window_size, padding);
    points_right = audio_transport::spectral::analysis(transport_data2, SAMPLE_RATE, window_size, padding);
  }

  void onInit() override {
    // called a single time just after the app is started
    //

    sine.resize(N);

    // remove this code later. it's just here to test
    for (int n = 0; n < N; n++) {
      sine[n].frequency(220.0 * (1 + n));
    }
  }

  void onCreate() override {
    // called a single time (in a graphics context) before onAnimate or onDraw
    //
    functionFromMax.onCreate();
    nav().pos(Vec3d(0, 0, 8));  // Set the camera to view the scene

    gui << background;
    gui << interp_p;
    gui << amp;
    gui << pick_files;
    gui << change_playback;
    gui.init();

    // Disable nav control; So default keyboard and mouse control is disabled
    navControl().active(false);
  }

  void onAnimate(double dt) override {
    // called over and over just before onDraw
    if (change_playback.get()) {
        change_playback.set(false);
        if (playback_mode == 0) {
            playback_mode = 2; // silence
            if (recalculate_audio_transport()) {
                this->s = 0;
                playback_mode = 1; // audio transport
            } else {
                playback_mode = 0; // sinusoidal
            }
        } else if (playback_mode == 1) {
            this->s = 0;
            playback_mode = 0; // sinusoidal
        }
    }
    if (pick_files.get()) {
        pick_files.set(false);
        playback_mode = 2; // silence
        read_new_files();
        playback_mode = 0; // reset to sinusoidal
    }
  }

  void onDraw(Graphics &g) override {
    // called over and over, once per view, per frame. warning! this may be
    // called more than once per frame. for instance, in the context of 3D
    // stereo viewing, this will be called twice per frame. if 6 screens are
    // attached to this system, then onDraw will be called 6 times per frame.
    //
    g.clear(background);
    //
    //
    functionFromMax.onDraw(g);
    // Draw th GUI
    gui.draw(g);
  }

  void onSound(AudioIOData &io) override {
    // don't need to check this sample by sample
    float interp = interp_p.get();
    float ampl = amp.get();
    switch (playback_mode) {
        case 0 : // sinusoidal model
            while (io()) {
                float t_val = float(this->s) / this->s_limit; // time from 0 to 1
                float frac_ind = t_val * frame_limit;
                int low_ind = (int)frac_ind;
                int high_ind = low_ind + 1;
                if (low_ind >= frame_limit) {
                    low_ind = frame_limit - 1;
                }
                if (high_ind >= frame_limit) {
                    high_ind = frame_limit - 1;
                }
                float upper_weight = frac_ind - low_ind;
                float lower_weight = 1.0 - upper_weight;

                // add the next sample from each of the N oscillators
                float f = 0;
                for (int n = 0; n < N; n++) {
                    float freq1 = (lower_weight * peaks1[low_ind][n].frequency) + (upper_weight * peaks1[high_ind][n].frequency);
                    float amp1 = ((lower_weight * peaks1[low_ind][n].amplitude) + (upper_weight * peaks1[high_ind][n].amplitude));
                    float freq2 = (lower_weight * peaks2[low_ind][n].frequency) + (upper_weight * peaks2[high_ind][n].frequency);
                    float amp2 = ((lower_weight * peaks2[low_ind][n].amplitude) + (upper_weight * peaks2[high_ind][n].amplitude));
                    float freq = std::abs((1.0-interp)*freq1 + interp*freq2); // extrapolating can lead to negative frequencies
                    float amp = (1.0-interp)*amp1 + interp*amp2;
                    sine[n].frequency(freq);
                    f += amp*sine[n]();
                }
                // reduce amplitude and limit
                // limiting becomes very important when extrapolating
                f = std::max(std::min(1.0f, f / N), -1.0f);
                io.out(0) = f * ampl;
                io.out(1) = f * ampl;
                this->s += 1;
                // if statement is probably actually faster than a modulo
                if (this->s == this->s_limit) {
                    this->s = 0;
                }
            }
            break;
        case 1 : // audio transport
            while (io()) {
                // will this automatically cast?
                io.out(0) = transport_audio[this->s] * ampl;
                io.out(1) = transport_audio[this->s] * ampl; 

                this->s += 1;
                // TODO: s limit should be the same as above but check this if we start crashing
                if (this->s == transport_audio.size()) {
                    this->s = 0;
                }   
            }
            break;
        default : // silence (usually means that audio transport is being calculated
            this->s = 0;
            // based on convo with karl, better to spit out zeros than exit
            while (io()) {
                io.out(0) = 0.0f;
                io.out(1) = 0.0f;
            }
    }

  }

  bool onKeyDown(const Keyboard &k) override {
    int ascii = k.key();

    // thread safe-ness of this whole thing is questionable
    if (ascii == 49) { // '1'
        this->s = 0;
        playback_mode = 0;
    }
    else if (ascii == 50) { // '2'
        playback_mode = 2; // silence
        if (recalculate_audio_transport()) {
            this->s = 0;
            playback_mode = 1; // audio transport
        } else {
            playback_mode = 0; // sinusoidal
        }
    } else if (ascii == 51) { // '3'
        playback_mode = 2; // silence
        read_new_files();
        playback_mode = 0; // sinusoidal
    }
    return true;
  }

  bool onKeyUp(const Keyboard &k) override {
    return true;
  }

  // bunch of stuff from karl's maxfunction main.cpp
  bool onMouseDrag(const Mouse& m) override {
    functionFromMax.onMouseDrag(m);
    return false;
  }

  bool onMouseDown(const Mouse& m) override {
    functionFromMax.onMouseDown(m);
    return false;
  }

  bool onMouseUp(const Mouse& m) override {
    functionFromMax.onMouseUp(m);

    //
    // Look here; This is how you access the normalized points
    //
    printf("\n========================================\n");
    for (auto p : functionFromMax.points()) {
      p.print();
      printf("\n");
    }
    return false;
  }

  bool onMouseMove(const Mouse& m) override {
    functionFromMax.onMouseMove(m);
    return false;
  }

  // end cp+pst from karl's main

  bool recalculate_audio_transport() {
    float interp = interp_p.get();
    if (interp < 0.0f || interp > 1.0f) {
        std::cerr << "Audio transport cannot extrapolate\n";
        return false;
    }
    
    // largely from audio transport repo examples (i.e. transport_sine.cpp)
    // Initialize phases
    std::vector<double> phases(points_left[0].size(), 0);
    size_t num_windows = std::min(points_left.size(), points_right.size());
    std::vector<std::vector<audio_transport::spectral::point>> points_interpolated(num_windows);

    double window_size = 0.05; // seconds
    unsigned int padding = 7; // multiplies window size

    for (size_t w = 0; w < num_windows; w++) {
        points_interpolated[w] = 
        audio_transport::interpolate(points_left[w], points_right[w], phases, window_size, interp);
    }

    transport_audio = audio_transport::spectral::synthesis(points_interpolated, padding);
    return true;
  }

  // create new synthesis features based on user-selected files
  void read_new_files() {
    nfdchar_t *file1 = NULL;
    nfdresult_t result1 = NFD_OpenDialog(NULL, NULL, &file1);

    if (result1 == NFD_OKAY) {
    } else if (result1 == NFD_CANCEL) {
        return;
    } else {
        printf("Error: %s\n", NFD_GetError());
        return;
    }

    nfdchar_t *file2 = NULL;
    nfdresult_t result2 = NFD_OpenDialog(NULL, NULL, &file2);

    if (result2 == NFD_OKAY) {
    } else if (result2 == NFD_CANCEL) {
        return;
    } else {
        printf("Error: %s\n", NFD_GetError());
        return;
    }

    // only recalculate things if the user has chosen two files
    calculate_synthesis_features(file1, file2);
  }
};

int main(int argc, char *argv[]) {
    
    if (argc < 4) {
        printf("usage: analysis-resynthesis wav-file1 wav-file2 num-oscs\n");
        return 1;
    }

    // MyApp constructor called here, given arguments from the command line
    MyApp app(argc, argv);

    app.configureAudio(48000, 512, 2, 1);

    // Start the AlloLib framework's "app" construct. This blocks until the app is
    // quit (or it crashes).
    app.start();

    return 0;
}

#include "Max-Function-Like/Function-From-Max.cpp"
