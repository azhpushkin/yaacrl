#include <string>
#include <set>
#include <vector>
#include <map>

#include "fingerprint.h"
#include "spectrogram.h"


namespace yaacrl {

struct WAVFile {
    WAVFile(std::string path);
    WAVFile(std::string path, std::string name);
    std::string path;
    std::string name;
};

struct MP3File {
    MP3File(std::string path);
    MP3File(std::string path, std::string name);
    std::string path;
    std::string name;
};

class Fingerprint {
public:
    Fingerprint(const WAVFile& file);
    Fingerprint(const MP3File& file);
    std::string name;
    Spectrogram spec;
    std::vector<Peak> peaks;
    std::vector<Hash> hashes;
private:
    void process(std::string path);
};


class Storage {
public:
    Storage(std::string filepath);
    ~Storage();
    void store_fingerprint(Fingerprint&& fp);
    void store_fingerprint(Fingerprint& fp);
    std::map<std::string, float> get_matches(Fingerprint& fp);
private:
    void* db;
};

}  // end namespace
