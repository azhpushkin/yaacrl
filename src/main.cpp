#include <iostream>
#include <string>

#include "lib.h"


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


    std::string path2("/home/maqquettex/projects/yaacrl/songs/ara_1.wav");

    Fingerprint to_match = Fingerprint::fromWAV(path2);
    std::cout << "Fingers amount: " << to_match.hashes.size() << std::endl;

    storage.get_matches(to_match);
    

    // return 0;
}