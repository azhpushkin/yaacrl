#include <vector>
#include <tuple>

#include "spectrogram.h"
#include "fingerprint.h"

#include "MurmurHash3.h"


#define LOC_BINS 20
#define LOC_WINDOWS 20
#define MIN_AMPLITUDE -30
#define MAX_FAN 50


bool is_local_maximum(Spectrogram& spec, int window, int bin) {
    float value = spec[window][bin];

    for (int w_shift = -LOC_WINDOWS; w_shift < LOC_WINDOWS; w_shift++) {
        if (window + w_shift < 0 || window + w_shift >= spec.size()) 
        { continue; }

        for (int b_shift = -LOC_BINS; b_shift < LOC_BINS; b_shift++) {
            if (bin + b_shift < 0 || bin + b_shift >= BINS_AMOUNT) 
            { continue; }

            if (w_shift == 0 && b_shift == 0)
            { continue; }

            if (spec[window+w_shift][bin+b_shift] >= value) 
            { return false; }
        }
    }

    return true;
}

std::vector<Peak> find_peaks(Spectrogram& spec) {
    std::vector<Peak> peaks;

    for (int window = 0; window < spec.size(); window++) {
        for (int bin = 0; bin < BINS_AMOUNT; bin++) {
            float value = spec[window][bin];
            if (value < MIN_AMPLITUDE || value == 0.0) {
                continue;
            }
            
            if (is_local_maximum(spec, window, bin) ) {
                peaks.emplace_back(window, bin);
            }
        }
    }

    return peaks;
}

std::vector<Hash> generate_hashes(std::vector<Peak>& peaks) {
    std::vector<Hash> hashes;

    auto format_key = new int[3];
    for(int i = 0; i < peaks.size(); i++) {
        int c = 0;
        for (int j = i + 1; (j < peaks.size() && c < MAX_FAN); j++, c++) {
            auto distance = std::get<0>(peaks[j]) - std::get<0>(peaks[i]);

            format_key[0] = std::get<1>(peaks[i]);  // freq 1
            format_key[1] = std::get<1>(peaks[j]);  // freq 2
            format_key[2] = distance;  // window2 - window1
            
            Hash new_hash;
            MurmurHash3_x64_128(
                format_key,
                sizeof(int) * 3,
                0,
                new_hash.data()
            );

            hashes.push_back(new_hash);
        }
    }
    
    delete format_key;
    return hashes;
}