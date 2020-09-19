#include <iostream>
#include <vector>
#include <array>
#include <math.h>
#include <cstdlib>

#define kiss_fft_scalar float
#include "kissfft/kiss_fft.h"

#include "spectrogram.h"


Spectrogram gen_spectrogram(std::vector<float> samples) {

    std::vector<float> window(WINDOW_SIZE, 0.0);
    // hanning function applied
    for (int i = 0; i < WINDOW_SIZE; ++i) {
        window[i] = (1 - cos((2 * M_PI * i) / (WINDOW_SIZE - 1) )) / 2;
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
            cpx_from[i].i = 0;
            if (chunk_start + i < samples.size()) {
                cpx_from[i].r = samples[chunk_start + i] * window[i];  // Do windowing
            } else {
                cpx_from[i].r = 0;  // we have read beyond the signal, so zero-pad it!
            }
        }
        
        // Perform the FFT on our chunk
        kiss_fft(cfg, cpx_from, cpx_to);

        std::array<float, BINS_AMOUNT> new_bin;
        for (int i = 0; i < BINS_AMOUNT; i++) {
            new_bin[i] = 10*log10(cpx_to[i].i * cpx_to[i].i + cpx_to[i].r*cpx_to[i].r);
            if (!isfinite(new_bin[i])){
                new_bin[i] = -9999.;
            }
        }
        specgram.push_back(new_bin);
    }

    free(cpx_from);
    free(cpx_to);
    kiss_fft_free(cfg);
    return specgram;
}
