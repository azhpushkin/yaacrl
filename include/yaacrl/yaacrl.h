#pragma once

#include <string>
#include <set>
#include <vector>
#include <map>

#include "fingerprint.h"
#include "spectrogram.h"
#include "config.h"


namespace yaacrl {

typedef std::vector<std::vector <float> > ChannelSamples;


class WAVFile {
    public:
        std::string path;
        ChannelSamples samples;
        
        WAVFile(std::string path);
};

class MP3File {
    public:
        std::string path;
        ChannelSamples samples;
       
        MP3File(std::string path);
};


class Fingerprint {
public:
    Fingerprint(const WAVFile& file);
    Fingerprint(const MP3File& file);
    std::vector<Peak> peaks;
    std::vector<Hash> hashes;
private:
    void process(ChannelSamples samples);
};

class StoredSong {
    public:
        int id;
        std::string name;
};


class Storage {
public:
    Storage(std::string filepath);
    ~Storage();
    StoredSong store_fingerprint(Fingerprint&& fp, std::string name);
    StoredSong store_fingerprint(Fingerprint& fp, std::string name);
    
    void delete_stored_song(StoredSong song);
    StoredSong rename_stored_song(StoredSong song, std::string new_name);
    std::vector<StoredSong> list_songs();

    std::vector<std::pair <StoredSong, float> > get_matches(Fingerprint& fp);
private:
    void* db;
};

}  // end namespace
