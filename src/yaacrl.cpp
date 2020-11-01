#include <string>
#include <map>
#include <utility>
#include <iostream>
#include <cstring>
#include <algorithm>

#include "vendor/sqlite3.h"
#include "vendor/AudioFile.h"

#include "fingerprint.h"
#include "spectrogram.h"
#include "yaacrl.h"

#include "vendor/fmt.h"

#define CONFIDENCE 2  // at least 2 simult for a match
#define DB_CON (sqlite3*)db

using namespace yaacrl;

WAVFile::WAVFile(std::string path_): path(path_), name(path_) {}
WAVFile::WAVFile(std::string path_, std::string name_): path(path_), name(name_) {}

MP3File::MP3File(std::string path_): path(path_), name(path_) {}
MP3File::MP3File(std::string path_, std::string name_): path(path_), name(name_) {}


Fingerprint::Fingerprint(const WAVFile& file) {
    this->name = file.name;
    this->process(file.path);
}

// TODO: throw exception for mp3 file
Fingerprint::Fingerprint(const MP3File& file) {
    this->name = file.name;
    this->process(file.path);
}

void Fingerprint::process(std::string path) {
    AudioFile<float> audioFile;

    audioFile.load (path);

    if (1 != audioFile.getNumChannels()) {
        // TODO: change to logging
        std::cerr << "Detected number of channels is not supported!" << std::endl;
        std::abort();
    }
    
    this->spec = gen_spectrogram(audioFile.samples[0]);
    this->peaks = find_peaks(this->spec);
    this->hashes = generate_hashes(this->peaks);
}

Storage::Storage() {
    int rc;
    char *zErrMsg;
    std::string sql;
    
    rc = sqlite3_open(":memory:", (sqlite3**)(&db));
    if ( rc ) {
        std::cerr << "Error creating SQLite database: " << sqlite3_errmsg(DB_CON) << std::endl;
        std::abort();
    }

    sql = R"(
        create table fingerprints (
            hash TEXT NOT NULL,
            offset INT NOT NULL,
            song TEXT NOT NULL
        );
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cerr << "Error: " << zErrMsg << std::endl;
    }

    sql = R"(
        create index hash_index on fingerprints(hash);
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cerr << "Error: " << zErrMsg << std::endl;
    }
}

Storage::~Storage() {
    sqlite3_close(DB_CON);
}

void Storage::store_fingerprint(Fingerprint&& fp) {
    store_fingerprint(fp);
}

void Storage::store_fingerprint(Fingerprint& fp) {
    int rc;
    char* errMsg;

    for (auto const& hash:  fp.hashes) {
        auto sql_str = fmt::format(
            "insert into fingerprints (hash, offset, song) values ({}, {}, '{}')",
            HASH_DATA(hash), HASH_OFFSET(hash), fp.name
        );

        rc = sqlite3_exec(DB_CON, sql_str.c_str(), NULL, 0, &errMsg);
        if( rc != SQLITE_OK ){
            std::cerr << "Error: " << errMsg << std::endl;
        }
    }
}


// Probably should not be here, IDK
class Match {
public:
    std::string song_name;
    int offset;
};


std::map<std::string, float> Storage::get_matches(Fingerprint& fp) {
    std::vector<Match> matches;

    // redisReply* reply;
    // for (auto const& hash:  fp.hashes) {

    //     reply = (redisReply*)redisCommand(
    //         (redisContext*)redis,
    //         "SMEMBERS hash:%b", HASH_DATA(hash), HASH_SIZE
    //     );
    //     if (reply == NULL) {
    //         continue;
    //     }
    //     if (reply->type != REDIS_REPLY_ARRAY) {
    //         continue;
    //     }
    //     for (int i = 0; i < reply->elements; i++) {
    //         auto r = reply->element[i];
    //         Match new_match;
    //         new_match.song_name = std::string(r->str, r->len);

    //         auto offset_reply = (redisReply*)redisCommand(
    //             (redisContext*)redis,
    //             "GET hash:%b:offset", HASH_DATA(hash), HASH_SIZE
    //         );
    //         int original_offset = std::stoi(std::string(offset_reply->str, offset_reply->len));
    //         int current_offset = HASH_OFFSET(hash);
    //         freeReplyObject(offset_reply);

    //         new_match.offset = original_offset - current_offset;
    //         matches.push_back(new_match);
    //     }

    //     freeReplyObject(reply);
        
    // }
    
    std::map<std::pair<int, std::string>, int> grouped_matches;
    for(auto& match: matches) {
        std::pair<int, std::string> pair = std::make_pair(match.offset, match.song_name);

        grouped_matches[pair]++;
    }

    std::map<std::string, int> songs_matches;
    for (auto& pair: grouped_matches) {
        auto amount = std::get<1>(pair);
        auto song = std::get<1>( std::get<0>(pair) );
        if (amount < CONFIDENCE) {
            continue; 
        }
        
        songs_matches[song] += amount;
    }

    std::map<std::string, float> res;
    for(auto& pair: songs_matches) {
        res[std::get<0>(pair)] = std::get<1>(pair) / (float) matches.size();
    }

    return res;
}
