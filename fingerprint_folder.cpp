#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

#include "yaacrl.h"


namespace fs = std::filesystem;


// Simplest possible logger
void custom_logger(yaacrl::LogLevel lvl, std::string msg) {
    std::cout << "[" << static_cast<int>(lvl) << "] " << msg << std::endl;
}



int main(int argc, char* argv[]) {
    yaacrl::set_logger(custom_logger);
    
    if (argc < 3) {
        std::cout << "Please specify folder to fingerprint and path to the output yaacrl database " << std::endl;
        return 1;
    }

    const fs::path songs_dir = fs::path(argv[1]);
    const fs::path library_path = fs::path(argv[2]);

    yaacrl::Storage storage(library_path);
    
    std::cout << "## Starting.." << std::endl;
    std::vector<yaacrl::StoredSong> songs;
    for(auto& p: fs::directory_iterator(songs_dir)) {
        std::string song_filename = p.path().filename();
        // std::cout <<   << std::endl;
        auto fprint = yaacrl::Fingerprint(yaacrl::WAVFile(p.path()));
        auto song = storage.store_fingerprint(fprint, p.path());
        songs.emplace_back(song);
    }

    std::cout << "Renaming "<< songs[0].id << " " << songs[0].name << std::endl;
    storage.rename_stored_song(songs[0], "Very First song!");


    storage.delete_stored_song(songs[1]);

    return 0;
    
}