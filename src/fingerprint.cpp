#include <vector>
#include <tuple>

#include "spectrogram.h"
#include "fingerprint.h"


#define LOC_BINS 20
#define LOC_WINDOWS 20
#define MIN_AMPLITUDE 10

std::vector<Peak> find_peaks(Spectrogram spec) {
    std::vector<Peak> peaks;

    for (int window = 0; window < spec.size(); window++) {
        for (int bin = 0; bin < BINS_AMOUNT; bin++) {
            double value = spec[window][bin];
            bool is_max = true;
            
            
            // Check if dot is a local maximum
            for (int w_shift = -LOC_WINDOWS; is_max && w_shift < LOC_WINDOWS; w_shift++) {
                if (window + w_shift < 0 || window + w_shift >= spec.size()) 
                { continue; }

                for (int b_shift = -LOC_BINS; is_max && b_shift < LOC_BINS; b_shift++) {
                    if (bin + b_shift < 0 || bin + b_shift >= BINS_AMOUNT) 
                    { continue; }

                    if (w_shift == 0 && b_shift == 0)
                    { continue; }

                    if (spec[window+w_shift][bin+b_shift] >= value) 
                    { is_max = false; }
                }

            }

            if (is_max && value && value > MIN_AMPLITUDE) {
                peaks.emplace_back(window, bin);
            }
        }
    }

    return peaks;
}