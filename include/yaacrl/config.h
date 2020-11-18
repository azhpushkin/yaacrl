#pragma once


namespace yaacrl {

enum class LogLevel {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};


void yaacrl_log_message(LogLevel lvl, std::string msg);

void set_logger(void(*new_logger)(LogLevel,std::string));

}
