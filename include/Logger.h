#pragma once

enum class LogLevel { Error, Warn, Info, Debug };

class Logger {
public:
    static void log(LogLevel level, const char* tag, const char* message);
};