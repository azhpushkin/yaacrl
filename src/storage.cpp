#include <string>
#include "hiredis/hiredis.h"
#include "fingerprint.h"




void store_hash(redisContext* c, Hash h, std::string s) {
    auto reply = redisCommand(c, "SET hash:%b %b", h.data(), h.size(), s.c_str(), s.length());
    if (reply == NULL) {
        return;
    }
    freeReplyObject(reply);
}

