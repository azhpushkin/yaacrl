#include "config.h"

using namespace yaacrl;

void (*log_function)(LogLevel,const char*);

void yaacrl_log_message(LogLevel lvl ,const char* msg) {
    if (log_function != nullptr) {
        log_function(lvl, msg);
    }
}


void yaacrl::set_logger( void (*new_logger) (LogLevel,const char*)) {
    log_function = new_logger;
}