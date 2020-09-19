#include <cstring>
#include <iostream>
#include <vector>
#include <fstream>

#include "hiredis/hiredis.h"
#include "AudioFile.h"

#include "spectrogram.h"
#include "fingerprint.h"
#include "lib.h"


#include <map>


int main() {
    AudioFile<float> audioFile;

    audioFile.load ("/home/maqquettex/projects/yaacrl/songs/arabella.wav");

    Spectrogram s = gen_spectrogram(audioFile.samples[0]);
    std::ofstream file("/home/maqquettex/projects/dejavu-test/spec.txt");

    for (auto const& row: s) {
        for (auto const& v: row){
            file << v << ",";
        }
        file << "\n";
    }
    file.close();


    std::ofstream file2("/home/maqquettex/projects/dejavu-test/peaks.txt");
    
    auto peaks = find_peaks(s);
    std::cout << "Found peaks: " << peaks.size() << std::endl;
    for (auto const& p: peaks) {
        file2 << std::get<0>(p) << "," << std::get<1>(p) << std::endl;
    }
    file2.close();
}