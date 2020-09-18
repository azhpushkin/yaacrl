#include <cstring>
#include <iostream>
#include <vector>

#include "hiredis/hiredis.h"
#include "AudioFile.h"

#include "spectrogram.h"
#include "fingerprint.h"
#include "storage.h"


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
    std::cout << "Hashes generated: " << hashes.size()
              << ". Example:" << std::string(hashes[0].data(), 16)
              << std::endl;
    

    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err) {
        std::cerr << "Error connecting to redis" << std::endl;
    }

    for (auto const& hash:  hashes) {
        store_hash(c, hash, std::string("arabella"));
    }
    redisFree(c);

    return 0;
}