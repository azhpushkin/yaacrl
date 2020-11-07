#include <vector>
#include <utility>
#include <array>

#include "spectrogram.h"


#define HASH_SIZE 16
#define HASH_DATA(h) std::get<0>(h).data()
#define HASH_OFFSET(h) std::get<1>(h)

typedef std::pair<int, int> Peak;  // <window, bin>
typedef std::pair<std::array<char, HASH_SIZE>, int> Hash;


std::vector<Peak> find_peaks(Spectrogram& spec);
std::vector<Hash> generate_hashes(std::vector<Peak>& peaks);

