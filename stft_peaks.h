#include <algorithm>  // std::sort
#include <complex>
#include <valarray>
#include <vector>

// suggested entry in a table of data resulting from the analysis of the input
// sound.
struct Entry {
  double frequency, amplitude;
};

bool entry_comparator ( const Entry& l, const Entry& r)
   { return l.frequency < r.frequency; } // sort ascending

// from stft-peaks.cpp
// used in fft
// adapted from: https://stackoverflow.com/questions/1577475/c-sorting-and-keeping-track-of-indexes

typedef std::pair<double,double> amp_and_freq;
bool amp_freq_comparator ( const amp_and_freq& l, const amp_and_freq& r)
   { return l.first > r.first; } // sort descending

// higher memory implementation via http://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B
typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;
void fft(CArray& x)
{
    const size_t N = x.size();
    if (N <= 1) return;

    // divide
    CArray even = x[std::slice(0, N/2, 2)];
    CArray  odd = x[std::slice(1, N/2, 2)];

    // conquer
    fft(even);
    fft(odd);

    // combine
    for (size_t k = 0; k < N/2; ++k)
    {
        Complex t = std::polar(1.0, -2 * M_PI * k / N) * odd[k];
        x[k    ] = even[k] + t;
        x[k+N/2] = even[k] - t;
    }
}

// fixed size for now
double* hann_window() {
    static double window[2048];

    for (int i = 0; i < 2048; i++) {
        window[i] = 0.5 * (1.0 - cos(2.0*M_PI*(i+1)/2049.0));
    }

    return window;
}

std::vector<std::vector<Entry>> stft_peaks(std::vector<float> data, int N, double sample_rate) {
    std::vector<std::vector<Entry>> entries;

    double* window = hann_window();
    int hop_size = 1024;
    int nfft = 8192;
    int window_size = 2048;
    int data_length = data.size();

    int nframes = ceil(data_length / float(hop_size));

    CArray fft_buf(nfft);
    int start_index = 0;

    for (int fr = 0; fr < nframes; fr++) {
        // PART 1: create fft buffer

        // should deal with size corner cases
        int end_index = std::min(data_length, start_index+hop_size);

        int j = 0;
        for (int i = start_index; i < end_index; i++) {
            fft_buf[j] = data[i];
            j++;
        }

        // zero-pad what's left
        while (j < nfft) {
            fft_buf[j] = 0.0;
            j++;
        }

        // PART 2: perform FFT

        // apply hann window
        for (int i = 0; i < window_size; i++) {
            fft_buf[i] *= window[i];
        }

        fft(fft_buf);

        // PART 3: find peaks
        double bin_step = double(sample_rate) / nfft;
        std::vector<amp_and_freq> peaks;
        // avoid giving peaks at DC or Nyquist
        // don't bother with negative frequencies
        for (int j = 1; j < nfft/2; j++) {
            double amp = std::abs(fft_buf[j]);
            // making one of these >= so that only one value in a plateau is captured
            if (amp > std::abs(fft_buf[j-1]) && amp >= std::abs(fft_buf[j+1])) {
                peaks.push_back(std::make_pair(std::abs(fft_buf[j]), j * bin_step));
            }
        }

        std::sort(peaks.begin(), peaks.end(), amp_freq_comparator);
        entries.push_back(std::vector<Entry>());
        for (int i = 0; i < N; i++) {
            // this will be 0 amp anyway, setting this high because of
            // how i am assigning voices (i.e. voice 0 always gets the
            // lowest frequency)
            double freq = sample_rate / 2.0;
            double amp = 0.0;
            if (i < peaks.size()) {
                freq = peaks[i].second;
                amp = peaks[i].first;
            }
            Entry e = {freq, amp};
            entries[fr].push_back(e);
        }

        // re-sort so that entries are sorted low to high in frequency
        std::sort(entries[fr].begin(), entries[fr].end(), entry_comparator);

        // next frame
        start_index += hop_size;
    }

    return entries;
}

