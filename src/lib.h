#include <string>
#include <set>
#include <vector>
#include "fingerprint.h"
#include "spectrogram.h"

class Match {
public:
    std::string song_name;
    int offset;
};

class Fingerprint {
public:
    std::string name;
    Spectrogram spec;
    std::vector<Peak> peaks;
    std::vector<Hash> hashes;

    static Fingerprint fromWAV(std::string path);
    static Fingerprint fromWAV(std::string path, std::string name);
};


class Storage {
public:
    Storage();
    ~Storage();
    void store_fingerprint(Fingerprint&& fp);
    void store_fingerprint(Fingerprint& fp);
    std::vector<Match> get_matches(Fingerprint& fp);
private:
    void* redis;
};