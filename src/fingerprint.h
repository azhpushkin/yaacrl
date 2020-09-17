#include <vector>
#include <tuple>

#include "spectrogram.h"


typedef std::tuple<int, int> Peak;

std::vector<Peak> find_peaks(Spectrogram spec);

