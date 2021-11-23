#pragma once

#include <vector>
#include <array>


#define WINDOW_SIZE 4096
#define BINS_AMOUNT (WINDOW_SIZE / 2 + 1) 
#define OVERLAP (WINDOW_SIZE / 2)


typedef std::vector<std::array <float, BINS_AMOUNT> > Spectrogram;

Spectrogram gen_spectrogram(std::vector<float> samples);
