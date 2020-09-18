#include <string>
#include "fingerprint.h"
#include "hiredis/hiredis.h"



void store_hash(redisContext* c, Hash h, std::string s);