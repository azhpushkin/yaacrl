#include <string>
#include <iostream>
#include <cstring>
#include "hiredis/hiredis.h"
#include "AudioFile.h"

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
            "GET hash:%b", hash.data(), hash.size()
        );
        if (reply->type != REDIS_REPLY_NIL) {
            (redisReply*)redisCommand(
                (redisContext*)redis,
                "DEL hash:%b", hash.data(), hash.size()
            );
        }

        reply = (redisReply*)redisCommand(
            (redisContext*)redis,
            "SET hash:%b %b", hash.data(), hash.size(), fp.name.c_str(), fp.name.length()
        );
        if (reply == NULL) {
            return;
        }
        freeReplyObject(reply);
    }
}

std::vector<std::string> Storage::get_matches(Fingerprint& fp) {
    std::vector<std::string> matches;

    redisReply* reply;
    for (auto const& hash:  fp.hashes) {
        reply = (redisReply*)redisCommand(
            (redisContext*)redis,
            "GET hash:%b", hash.data(), hash.size()
        );
        if (reply == NULL) {
            continue;
        }
        if (reply->type == REDIS_REPLY_NIL) {
            continue;
        }

        matches.emplace_back(reply->str, reply->len);

        freeReplyObject(reply);
    }
    return matches;
}