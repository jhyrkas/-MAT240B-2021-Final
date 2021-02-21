#include <vector>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

// loading audio for stft peaks analysis
void load(std::vector<float> &input, const char *filePath) {
  unsigned int channels;
  unsigned int sampleRate;
  drwav_uint64 totalPCMFrameCount;
  float *pSampleData = drwav_open_file_and_read_pcm_frames_f32(
      filePath, &channels, &sampleRate, &totalPCMFrameCount, NULL);
  if (pSampleData == NULL) {
    printf("failed to load %s\n", filePath);
    exit(1);
  }

  //
  if (channels == 1)
    for (int i = 0; i < totalPCMFrameCount; i++) {
      input.push_back(pSampleData[i]);
    }
  else if (channels == 2) {
    for (int i = 0; i < totalPCMFrameCount; i++) {
      input.push_back((pSampleData[2 * i] + pSampleData[2 * i + 1]) / 2);
    }
  } else {
    printf("can't handle %d channels\n", channels);
    exit(1);
  }

  drwav_free(pSampleData, NULL);
}

// convert audio from load() to a format that audio transport wants
std::vector<double> audio_transport_input(std::vector<float> data) {
    std::vector<double> outp;
    for (int i = 0; i < data.size(); i++) {
        outp.push_back(data[i]);
    }
    return outp;
}
