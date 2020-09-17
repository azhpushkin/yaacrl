#include <vector>
#include <array>


#define WINDOW_SIZE 2048
#define BINS_AMOUNT 1025 // (WINDOW_SIZE / 2) + 1
#define OVERLAP 1024  // WINDOWS_SIZE / 2


typedef std::vector<std::array<double, BINS_AMOUNT>> Spectrogram;

Spectrogram gen_spectrogram(std::vector<double> samples);
