#include <vector>
#include <array>
#include <math.h>
#include <cstdlib>

#define kiss_fft_scalar double
#include "kissfft/kiss_fft.h"

#include "spectrogram.h"


inline double log_transform(kiss_fft_cpx val) {
    return 10*log10(val.i * val.i + val.r * val.r);
}


Spectrogram gen_spectrogram(std::vector<double> samples) {
    std::vector<double> window(WINDOW_SIZE, 0.0);
    // hanning function applied
    for (int i = 0; i < WINDOW_SIZE; ++i) {
        window[i] = (1 - cos((2 * M_PI * i) / (WINDOW_SIZE - 1) ));
    }

    int frames_amount = 1;
    for (int i = WINDOW_SIZE; i < samples.size();) {
        i += WINDOW_SIZE;
        i -= OVERLAP;
        ++frames_amount;
    }

    Spectrogram specgram;

    kiss_fft_cfg cfg = kiss_fft_alloc(WINDOW_SIZE, 0, NULL, NULL);
    kiss_fft_cpx *cpx_from, *cpx_to;


    // Process each chunk of the signal
    cpx_to = (kiss_fft_cpx*) malloc(WINDOW_SIZE * sizeof(kiss_fft_cpx));
    cpx_from = (kiss_fft_cpx*) malloc(WINDOW_SIZE * sizeof(kiss_fft_cpx));

    for (int chunk_start = 0; chunk_start < samples.size(); chunk_start += (WINDOW_SIZE - OVERLAP)) {
        // Copy the chunk into our buffer
        for (int i = 0; i < WINDOW_SIZE; i++) {
            cpx_from[i].i = 0.0;
            if (chunk_start + i < samples.size()) {
                cpx_from[i].r = samples[chunk_start + i] * window[i];  // Do windowing
            } else {
                cpx_from[i].r = 0.0;  // we have read beyond the signal, so zero-pad it!
            }
        }
        
        // Perform the FFT on our chunk
        kiss_fft(cfg, cpx_from, cpx_to);

        std::array<double, BINS_AMOUNT> new_bin;
        for (int i = 0; i < BINS_AMOUNT; i++) {
            new_bin[i] = log_transform(cpx_to[i]);
        }
        specgram.push_back(new_bin);
    }

    free(cpx_from);
    free(cpx_to);
    kiss_fft_free(cfg);
    return specgram;
}
