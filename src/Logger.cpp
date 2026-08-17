#include "Logger.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <cstdio>
#endif

static const char* levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Error: return "ERROR";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Debug: return "DEBUG";
    }
    return "?";
}

void Logger::log(LogLevel level, const char* tag, const char* message) {
#ifdef ARDUINO
    Serial.printf("[%s][%s] %s\n", levelToString(level), tag, message);
#else
    std::printf("[%s][%s] %s\n", levelToString(level), tag, message);
#endif
}