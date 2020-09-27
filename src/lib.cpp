#include <string>
#include <map>
#include <utility>
#include <iostream>
#include <cstring>
#include <algorithm>

#include "hiredis/hiredis.h"
#include "vendor/AudioFile.h"

#include "fingerprint.h"
#include "spectrogram.h"
#include "lib.h"

#define CONFIDENCE 2  // at least 2 simult for a match

Fingerprint Fingerprint::fromWAV(std::string path) {
    return Fingerprint::fromWAV(path, path);
}

Fingerprint Fingerprint::fromWAV(std::string path, std::string name) {
    Fingerprint fp;
    fp.name = name;
    AudioFile<float> audioFile;

    audioFile.load (path);

    if (1 != audioFile.getNumChannels()) {
        std::cerr << "Detected number of channels is not supported!" << std::endl;
        std::abort();
    }
    
    fp.spec = gen_spectrogram(audioFile.samples[0]);
    fp.peaks = find_peaks(fp.spec);
    fp.hashes = generate_hashes(fp.peaks);
    return fp;
}

Storage::Storage() {
    redis = redisConnect("127.0.0.1", 6379);
    if (redis == NULL || ((redisContext*)redis)->err) {
        std::cerr << "Error connecting to redis" << std::endl;
        std::abort();
    }
}

Storage::~Storage() {
    redisFree((redisContext*)redis);
}

void Storage::store_fingerprint(Fingerprint&& fp) {
    store_fingerprint(fp);
}

void Storage::store_fingerprint(Fingerprint& fp) {
    redisReply* reply;
    for (auto const& hash:  fp.hashes) {
        redisAppendCommand(
            (redisContext*)redis,
            "SADD hash:%b %b", HASH_DATA(hash), HASH_SIZE, fp.name.c_str(), fp.name.length()
        );
        redisAppendCommand(
            (redisContext*)redis,
            "SET hash:%b:offset %d", HASH_DATA(hash), HASH_SIZE, HASH_OFFSET(hash)
        );
        redisGetReply((redisContext*)redis, (void **) &reply);
        freeReplyObject(reply);
        redisGetReply((redisContext*)redis, (void **) &reply);
        freeReplyObject(reply);
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

    redisReply* reply;
    for (auto const& hash:  fp.hashes) {

        reply = (redisReply*)redisCommand(
            (redisContext*)redis,
            "SMEMBERS hash:%b", HASH_DATA(hash), HASH_SIZE
        );
        if (reply == NULL) {
            continue;
        }
        if (reply->type != REDIS_REPLY_ARRAY) {
            continue;
        }
        for (int i = 0; i < reply->elements; i++) {
            auto r = reply->element[i];
            Match new_match;
            new_match.song_name = std::string(r->str, r->len);

            auto offset_reply = (redisReply*)redisCommand(
                (redisContext*)redis,
                "GET hash:%b:offset", HASH_DATA(hash), HASH_SIZE
            );
            int original_offset = std::stoi(std::string(offset_reply->str, offset_reply->len));
            int current_offset = HASH_OFFSET(hash);
            freeReplyObject(offset_reply);

            new_match.offset = original_offset - current_offset;
            matches.push_back(new_match);
        }

        freeReplyObject(reply);
        
    }
    
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
