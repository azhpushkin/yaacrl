#include "config.h"


void (*log_function)(LogLevel,const char*);

void log_message(LogLevel lvl ,const char* msg) {
    if (log_function != nullptr) {
        log_function(lvl, msg);
    }
}

void set_logger( void (*new_logger) (LogLevel,const char*)) {
    log_function = new_logger;
}