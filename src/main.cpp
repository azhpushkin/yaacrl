#include <cstring>
#include <iostream>
#include <vector>

#include "hiredis/hiredis.h"
#include "AudioFile.h"

#include "spectrogram.h"
#include "fingerprint.h"
#include "lib.h"


int main() {
    std::string path("/home/maqquettex/projects/yaacrl/songs/arabella.wav");
    std::string name("Arabella");

    Fingerprint fp = Fingerprint::fromWAV(path, name);
    Storage storage;

    storage.store_fingerprint(fp);


    // Match
    std::string path2("/home/maqquettex/projects/yaacrl/songs/arabella_short.wav");

    Fingerprint to_match = Fingerprint::fromWAV(path2);

    auto res = storage.get_matches(to_match);
    std::cout << "found matches: " << res.size() << std::endl;

    return 0;
}