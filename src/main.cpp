#include <cstring>
#include <iostream>
#include <vector>

#include "AudioFile.h"
#include "spectrogram.h"
#include "fingerprint.h"


int main() {
    AudioFile<double> audioFile;

    audioFile.load ("/home/maqquettex/projects/yaacrl/songs/arabella_short.wav");
    audioFile.printSummary();

    if (1 != audioFile.getNumChannels()) {
        std::cerr << "Detected number of channels is not supported!" << std::endl;
        std::abort();
    }
    
    Spectrogram spec = gen_spectrogram(audioFile.samples[0]);
    std::cout << "Specgram generated, windows amount: " << spec.size() << std::endl;

    auto peaks = find_peaks(spec);
    std::cout << "Detected peaks: " << peaks.size() << std::endl;

    auto hashes = generate_hashes(peaks);
    std::cout << "Hashes generated: " << hashes.size() << std::endl;
    
    return 0;
}