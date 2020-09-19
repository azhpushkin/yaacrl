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
}