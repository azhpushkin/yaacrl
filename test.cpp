#include <iostream>
#include <string>
#include <filesystem>
#include <optional>
#include <vector>

#include "yaacrl.h"


namespace fs = std::filesystem;


// Simplest possible logger
void custom_logger(yaacrl::LogLevel lvl, std::string msg) {
    std::string lvl_pretty;
    if (lvl == yaacrl::LogLevel::DEBUG) lvl_pretty = "DEBUG";
    else if (lvl == yaacrl::LogLevel::INFO) lvl_pretty = "INFO";
    else if (lvl == yaacrl::LogLevel::WARNING) lvl_pretty = "WARNING";
    else if (lvl == yaacrl::LogLevel::ERROR) lvl_pretty = "ERROR";
    else lvl_pretty = "UNKNOWN_LEVEL";
    std::cout << "[" << lvl_pretty << "] " << msg << std::endl;
}

std::optional<yaacrl::Fingerprint> fingerprint_file(fs::path p) {
    std::string filename = p.filename();
    auto ext = filename.substr(filename.find_last_of(".") + 1);
    if (ext == "mp3") {
        return yaacrl::Fingerprint(yaacrl::MP3File(p));
    } else if (ext == "wav") {
        return yaacrl::Fingerprint(yaacrl::MP3File(p));
    } else {
        return std::nullopt;
    }
}


void fingerprint(fs::path library_path, fs::path songs_dir) {
    yaacrl::Storage storage(library_path);
    
    std::cout << "## Starting.." << std::endl;
    std::vector<yaacrl::StoredSong> songs;
    for(auto& p: fs::directory_iterator(songs_dir)) {
        std::string song_filename = p.path().filename();
        auto fprint = fingerprint_file(p.path());
        if (fprint) {
            auto song = storage.store_fingerprint(*fprint, song_filename);
            songs.emplace_back(song);
        }
        
    }

    auto songs_count = storage.list_songs().size();
    std::cout << "Total songs in DB now: " << songs_count << std::endl;
}

void match(fs::path library_path, fs::path clip_path) {
    yaacrl::Storage storage(library_path);

    std::cout << "Check " << clip_path.filename() << std::endl;
    auto fprint = fingerprint_file(clip_path);
    if (!fprint) {
        std::cout << "Not a correct audio file!" << std::endl;
        return;
    }
    auto matches = storage.get_matches(*fprint);
    std::cout << "  -> " << " Match results: " <<  std::endl;
    for (auto& pair: matches) {
        std::cout << "    * " << std::get<0>(pair).name << ": " << std::get<1>(pair) * 100 << "%" << std::endl;
    }

}


int main(int argc, char* argv[]) {
    yaacrl::set_logger(custom_logger);
    
    // operation
    if (argc < 4) {
        std::cout << "Please use following format: ./test operation_type library_path object_path" << std::endl;
        std::cout << "  operation_type: fingerprint OR match" << std::endl;
        std::cout << "  library_path: path to library for storing or matching" << std::endl;
        std::cout << "  object_path: folder to fingerprint OR audio file to match" << std::endl;
        return 1;
    }

    const std::string operation_type(argv[1]);
    const fs::path library_path = fs::path(argv[2]);
    const fs::path obj_path = fs::path(argv[3]);

    if (operation_type == "fingerprint") fingerprint(library_path, obj_path);
    else if (operation_type == "match") match(library_path, obj_path);
    else std::cout << "Unknown operation: "<< operation_type << std::endl;

    
    return 0;
    
}