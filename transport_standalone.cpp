#include <cmath>      // ::sin()
#include <complex>
#include <iostream>
#include <string>
#include <vector>

// audio transport stuff
#include "audio_transport/spectral.hpp"
#include "audio_transport/audio_transport.hpp"
#include <audiorw.hpp> // for writing...can't be bothered to do dr_wav right now

#include "dr_wav.h"
#include "interstrap_util.h" // load()

int main(int argc, char *argv[]) {
    double SAMPLE_RATE = 48000.0; // static for now
    // command line params and setup
    std::vector<float> pSampleData1;
    load(pSampleData1, argv[1]);
    std::vector<float> pSampleData2;
    load(pSampleData2, argv[2]);

    float interp = atof(argv[3]);
    assert(interp >= 0.0f && interp <= 1.0f);

    std::string fileName = argv[4];

    double window_size = 0.05; // seconds
    unsigned int padding = 7; // multiplies window size

    // convert to format audio transport library likes
    std::vector<double> transport_data1 = audio_transport_input(pSampleData1);
    std::vector<double> transport_data2 = audio_transport_input(pSampleData2);

    // get points
    std::vector<std::vector<audio_transport::spectral::point>> points_left = audio_transport::spectral::analysis(transport_data1, SAMPLE_RATE, window_size, padding);
    std::vector<std::vector<audio_transport::spectral::point>> points_right = audio_transport::spectral::analysis(transport_data2, SAMPLE_RATE, window_size, padding);

    // initialize phase
    std::vector<double> phases(points_left[0].size(), 0);
    size_t num_windows = std::min(points_left.size(), points_right.size());
    std::vector<std::vector<audio_transport::spectral::point>> points_interpolated(num_windows);

    // audio transport algorithm
    for (size_t w = 0; w < num_windows; w++) {
        points_interpolated[w] =
        audio_transport::interpolate(points_left[w], points_right[w], phases, window_size, interp);
    }

    std::vector<double> transport_audio = audio_transport::spectral::synthesis(points_interpolated, padding);
    std::vector<std::vector<double>> audio_out;
    audio_out.push_back(transport_audio);

    // write output
    audiorw::write(audio_out, fileName, SAMPLE_RATE);
}
