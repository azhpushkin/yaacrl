#include <cstring>
#include <iostream>
#include <vector>

#include "AudioFile.h"


int main() {
    AudioFile<double> audioFile;

    audioFile.load ("/home/maqquettex/projects/yaacrl/songs/knee_socks.wav");
    audioFile.printSummary();

    if (1 != audioFile.getNumChannels()) {
        std::cerr << "Detected number of channels is not supported!" << std::endl;
        std::abort();
    }

    std::vector<double> samples = audioFile.samples[0];
    std::cout << "Total samples amount: " << samples.size() << std::endl;
    return 0;
}