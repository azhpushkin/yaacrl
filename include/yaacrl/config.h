#pragma once


namespace yaacrl {

enum class LogLevel {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};


void yaacrl_log_message(LogLevel lvl, const char* msg);

void set_logger(void(*new_logger)(LogLevel,const char*));

}
