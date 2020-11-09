#pragma once

#include <vector>
#include <cstdint>

#include "spectrogram.h"


struct Peak {
    int window;
    int bin;
    Peak() {};  // required by Cython
    Peak(int window_, int bin_): window(window_), bin(bin_) {};
};

struct Hash {
    uint8_t hash_data[16];  // 128 bits entropy
    int offset;
};

std::vector<Peak> find_peaks(Spectrogram& spec);
std::vector<Hash> generate_hashes(std::vector<Peak>& peaks);

