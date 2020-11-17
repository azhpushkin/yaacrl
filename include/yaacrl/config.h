#pragma once



enum class LogLevel {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};


// void log_message(LogLevel lvl, const char* msg); - private
void set_logger(void(*new_logger)(LogLevel,const char*));


