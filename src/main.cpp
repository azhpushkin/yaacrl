#include <cstring>
#include <iostream>
#include <vector>

#include "hiredis/hiredis.h"
#include "AudioFile.h"

#include "spectrogram.h"
#include "fingerprint.h"
#include "lib.h"


int main() {
    std::string path("/home/maqquettex/projects/yaacrl/songs/arabella_short.wav");
    std::string name("Arabella");

    Fingerprint fp = Fingerprint::fromWAV(path, name);
    Storage storage;

    storage.store_fingerprint(fp);

    return 0;
}