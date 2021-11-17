#include <vector>

#include "spectrogram.h"
#include "fingerprint.h"

#include "vendor/MurmurHash3.h"


#define LOC_BINS 10
#define LOC_WINDOWS 10
#define MIN_AMPLITUDE -5
#define MAX_FAN 20
#define MAX_WINDOW_DISTANCE 400


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
    bool* x = new bool[BINS_AMOUNT * spec.size()];
    std::fill_n(x, BINS_AMOUNT * spec.size(), true);

    for (int window = 0; window < spec.size(); window++) {
        for (int bin = 0; bin < BINS_AMOUNT; bin++) {
            float value = spec[window][bin];
            // if (!x[window*BINS_AMOUNT + bin]) {
            //     continue;
            // } 
            if (value < MIN_AMPLITUDE) {
                continue;
            }
            
            if (is_local_maximum(spec, window, bin) ) {
                peaks.emplace_back(window, bin);

                // for (int w_shift = -LOC_WINDOWS; w_shift < LOC_WINDOWS; w_shift++) {
                //     if (window + w_shift < 0 || window + w_shift >= spec.size()) 
                //     { continue; }

                //     for (int b_shift = -LOC_BINS; b_shift < LOC_BINS; b_shift++) {
                //         if (bin + b_shift < 0 || bin + b_shift >= BINS_AMOUNT) 
                //         { continue; }

                //         if (w_shift == 0 && b_shift == 0)
                //         { continue; }

                //         x[(window+w_shift)*BINS_AMOUNT + bin+b_shift] = false;
                //     }
                // }
            }
        }
    }
    delete x;
    return peaks;
}

std::vector<Hash> generate_hashes(std::vector<Peak>& peaks) {
    std::vector<Hash> hashes;

    auto format_key = new int[3];
    for(int i = 0; i < peaks.size(); i++) {
        int c = 0;
        for (int j = i+1; (j < i+1+MAX_FAN) && j < peaks.size(); j++) {
            auto distance = peaks[j].window - peaks[i].window;
            if (distance < 0 || distance > MAX_WINDOW_DISTANCE) continue;

            format_key[0] = peaks[i].bin;  // freq 1
            format_key[1] = peaks[j].bin;  // freq 2
            format_key[2] = distance;  // window2 - window1
            
            Hash new_hash;
            MurmurHash3_x64_128(
                format_key,
                sizeof(int) * 3,
                0,
                new_hash.hash_data  // pointer to std::array
            );
            // offset from beginning of the track, thus earlier peak taken
            new_hash.offset = peaks[i].window;

            hashes.push_back(new_hash);
            c++;
        }
    }
    
    delete format_key;
    return hashes;
}