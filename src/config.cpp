#include <string>

#include "config.h"

using namespace yaacrl;

void (*log_function)(LogLevel,std::string);

void yaacrl::yaacrl_log_message(LogLevel lvl ,std::string msg) {
    if (log_function != nullptr) {
        log_function(lvl, msg);
    }
}


void yaacrl::set_logger( void (*new_logger) (LogLevel,std::string)) {
    log_function = new_logger;
}