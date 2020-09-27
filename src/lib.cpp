#include <string>
#include <iostream>
#include <cstring>
#include "hiredis/hiredis.h"
#include "vendor/AudioFile.h"
#include <algorithm>

#include "fingerprint.h"
#include "spectrogram.h"

#include "lib.h"

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

std::vector<Match> Storage::get_matches(Fingerprint& fp) {
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
        if (reply->type == REDIS_REPLY_NIL) {
            continue;
        }
        Match new_match;
        new_match.song_name = std::string(reply->str, reply->len);
        
        freeReplyObject(reply);
        reply = (redisReply*)redisCommand(
            (redisContext*)redis,
            "GET hash:%b:offset", std::get<0>(hash).data(), std::get<0>(hash).size()
        );
        int original_offset = std::stoi(std::string(reply->str, reply->len));
        int current_offset = std::get<1>(hash);
        freeReplyObject(reply);

        new_match.offset = original_offset - current_offset;
        matches.push_back(new_match);
    }
    return matches;

}