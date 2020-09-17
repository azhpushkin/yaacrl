#include <cstring>
#include <iostream>
#include <vector>

#include "AudioFile.h"
#include "spectrogram.h"


int main() {
    AudioFile<double> audioFile;

    audioFile.load ("/home/maqquettex/projects/yaacrl/songs/knee_socks.wav");
    audioFile.printSummary();

    if (1 != audioFile.getNumChannels()) {
        std::cerr << "Detected number of channels is not supported!" << std::endl;
        std::abort();
    }
    
    Spectrogram spec = gen_spectrogram(audioFile.samples[0]);
    std::string asd;
    
    return 0;
}