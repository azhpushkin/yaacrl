/*
This is a small test program that I use for testing YAACRL.

Here is how you can try it:
1. Create a directory with some audio files (WAV, mono)
2. Add `test_` prefix to the files that need to be used for matching
3. Add `skip_` prefix to files you want to exclude from processing
  (so you dont need to worry about removal-restore of those files)
4. Build `yaacrl-test` CMake target
5. Run `build_dir/yaacrl-test /path/to/dir/with/audio`
6. Enjoy!

I honestly do not expect anyone to read this comment or to launch this program,
but as you did it for some reason, here is a cute ASCII cat as an endorsement:
 ,_     _
 |\\_,-~/
 / _  _ |    ,--.
(  @  @ )   / ,-'
 \  _T_/-._( (
 /         `. \
|         _  \ |
 \ \ ,  /      |
  || |-_\__   /
 ((_/`(____,-'
 
*/
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <map>

#include "yaacrl.h"

namespace fs = std::filesystem;


// Simplest possible logger
void custom_logger(yaacrl::LogLevel lvl,const char* msg) {
    std::cout << "[" << static_cast<int>(lvl) << "] " << msg << std::endl;
}




int main(int argc, char* argv[]) {
    yaacrl::set_logger(custom_logger);

    std::vector<fs::path> songs;
    std::vector<fs::path> test_fragments;
    
    if (argc < 2) {
        std::cout << "Please add directory with songs and test samples" << std::endl;
        return 1;
    }

    const fs::path songs_dir = fs::path(argv[1]);
    
    for(auto& p: fs::directory_iterator(songs_dir)) {
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

    yaacrl::Storage storage("database.sqlite");
    std::cout << "### Upload songs" << std::endl;
    for (auto& song: songs) {
        std::cout << "Processing " << song.filename() << std::endl;
        auto fprint = yaacrl::Fingerprint(yaacrl::WAVFile(song));
        std::cout << "  -> " << fprint.hashes.size() << " hashes created" << std::endl;
        storage.store_fingerprint(fprint);
        std::cout << "  -> fingeprint saved" << std::endl;
    }

    std::cout << "### Test fragments" << std::endl;
    for (auto& item: test_fragments) {
        std::cout << "Check " << item.filename() << std::endl;
        auto fprint = yaacrl::Fingerprint(yaacrl::WAVFile(item));
        std::cout << "  -> " << fprint.hashes.size() << " hashes detected" << std::endl;
        auto matches = storage.get_matches(fprint);
        std::cout << "  -> " << " Match results: " <<  std::endl;
        for (auto& pair: matches) {
            std::cout << "    * " << std::get<0>(pair) << ": " << std::get<1>(pair) * 100 << "%" << std::endl;
        }

    }
    

    return 0;
    
}