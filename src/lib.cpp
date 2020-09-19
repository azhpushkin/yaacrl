#include <string>
#include <iostream>
#include <cstring>
#include "hiredis/hiredis.h"
#include "AudioFile.h"
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
    audioFile.printSummary();

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
    std::cout << "Storing " << fp.hashes.size() << " for " << fp.name << std::endl;
    redisReply* reply;
    for (auto const& hash:  fp.hashes) {
        reply = (redisReply*)redisCommand(
            (redisContext*)redis,
            "GET hash:%b", std::get<0>(hash).data(), std::get<0>(hash).size()
        );
        if (reply->type != REDIS_REPLY_NIL) {
            (redisReply*)redisCommand(
                (redisContext*)redis,
                "DEL hash:%b", std::get<0>(hash).data(), std::get<0>(hash).size()
            );
        }

        reply = (redisReply*)redisCommand(
            (redisContext*)redis,
            "SET hash:%b %b", std::get<0>(hash).data(), std::get<0>(hash).size(), fp.name.c_str(), fp.name.length()
        );
        freeReplyObject(reply);
        reply = (redisReply*)redisCommand(
            (redisContext*)redis,
            "SET hash:%b:offset %d", std::get<0>(hash).data(), std::get<0>(hash).size(), std::get<1>(hash)
        );
        freeReplyObject(reply);
    }
}
#define MMATCH std::tuple<std::string, int, int>
void Storage::get_matches(Fingerprint& fp) {
    std::vector<MMATCH> matches;

    redisReply* reply;
    for (auto const& hash:  fp.hashes) {

        reply = (redisReply*)redisCommand(
            (redisContext*)redis,
            "GET hash:%b", std::get<0>(hash).data(), std::get<0>(hash).size()
        );
        if (reply == NULL) {
            continue;
        }
        if (reply->type == REDIS_REPLY_NIL) {
            continue;
        }
        MMATCH match;
        std::get<0>(match) = std::string(reply->str, reply->len);
        
        freeReplyObject(reply);

        reply = (redisReply*)redisCommand(
            (redisContext*)redis,
            "GET hash:%b:offset", std::get<0>(hash).data(), std::get<0>(hash).size()
        );
        std::get<1>(match) = std::stoi(std::string(reply->str, reply->len));
        std::get<2>(match) = std::get<1>(hash);
        freeReplyObject(reply);
        matches.push_back(match);
    }
    std::cout << "Matches: "<< matches.size() << std::endl;
    std::sort(matches.begin(), matches.end(),
        [](const MMATCH& a, MMATCH& b) -> bool
        { return std::get<0>(a) > std::get<0>(b); }
        );
    for (auto const& v: matches) {
        std::cout << std::get<1>(v) - std::get<2>(v) << " || " << std::get<0>(v) << std::endl;
    }

}