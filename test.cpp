#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

#include "lib.h"


namespace fs = std::filesystem;

int main() {
    std::vector<fs::path> songs;
    std::vector<fs::path> test_fragments;
    
    // TODO: CLI arguments needed instead of hardcode
    auto home_path = std::string(getenv("HOME"));
    const fs::path workdir = home_path + "/projects/yaacrl/songs";
    
    for(auto& p: fs::directory_iterator(workdir)) {
        std::string filename = p.path().filename();
        if (filename.find("skip_") == 0) {
            continue;
        }
        else if (filename.find("test_") == 0) {
            test_fragments.push_back(p);
        } else {
            songs.push_back(p);
        }
    }

    Storage storage;
    std::cout << "### Upload songs" << std::endl;
    for (auto& song: songs) {
        std::cout << "Processing " << song.filename() << std::endl;
        auto fprint = Fingerprint::fromWAV(song);
        std::cout << "  -> " << fprint.hashes.size() << " hashes created" << std::endl;
        storage.store_fingerprint(fprint);
        std::cout << "  -> fingeprint saved" << std::endl;
    }

    std::cout << "### Test fragments" << std::endl;
    for (auto& item: test_fragments) {
        std::cout << "Check " << item.filename() << std::endl;
        auto fprint = Fingerprint::fromWAV(item);
        std::cout << "  -> " << fprint.hashes.size() << " hashes detected" << std::endl;
        auto matches = storage.get_matches(fprint);
        std::cout << "  -> " << matches.size() << " matches found" << std::endl;
    }
    

    return 0;

   
    return 0;
    // Match


    // std::string path2("/home/maqquettex/projects/yaacrl/songs/a1.wav");

    // Fingerprint to_match = 
    // std::cout << "Fingers amount: " << to_match.hashes.size() << std::endl;

    // 
    

    // return 0;
}