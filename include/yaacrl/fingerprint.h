#pragma once

#include <vector>
#include <array>

#include "spectrogram.h"


#define HASH_SIZE 16

struct Peak {
    int window;
    int bin;
    Peak(int window_, int bin_): window(window_), bin(bin_) {};
};

struct Hash {
    std::array<char, HASH_SIZE> hash;
    int offset;
};

std::vector<Peak> find_peaks(Spectrogram& spec);
std::vector<Hash> generate_hashes(std::vector<Peak>& peaks);

