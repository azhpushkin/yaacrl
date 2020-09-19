#include <cstring>
#include <iostream>
#include <vector>

#include "hiredis/hiredis.h"
#include "AudioFile.h"

#include "spectrogram.h"
#include "fingerprint.h"
#include "lib.h"

#include <map>


int main() {
    Storage storage;

    // std::string arabella("/home/maqquettex/projects/yaacrl/songs/arabella.wav");
    // storage.store_fingerprint(Fingerprint::fromWAV(arabella));

    // std::string dll("/home/maqquettex/projects/yaacrl/songs/dance_little_liar.wav");
    // storage.store_fingerprint(Fingerprint::fromWAV(dll));

    // std::string knee_socks("/home/maqquettex/projects/yaacrl/songs/knee_socks.wav");
    // storage.store_fingerprint(Fingerprint::fromWAV(knee_socks));

    // std::string one_for("/home/maqquettex/projects/yaacrl/songs/one_for_the_road.wav");
    // storage.store_fingerprint(Fingerprint::fromWAV(one_for));


    // return 0;
    // Match


    std::string path2("/home/maqquettex/projects/yaacrl/songs/ara_3.wav");

    Fingerprint to_match = Fingerprint::fromWAV(path2);
    std::cout << "Fingers amount: " << to_match.hashes.size() << std::endl;

    auto res = storage.get_matches(to_match);
    std::cout << "found matches: " << res.size() << std::endl;

    std::map<std::string, int> what;
    for (auto const& x: res) {
        // std::cout << "song: " << x << std::endl;
        auto q = what.find(x);
        if (q == what.end()) {
            what[x] = 1;
        } else {
            what[x] = q->second + 1;
        }
    }
    for (auto const& pair: what) {
        std::cout << pair.first << " = " << pair.second << std::endl;
    }

    // return 0;
}