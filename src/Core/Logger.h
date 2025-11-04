#pragma once

#include <format>
#include <fstream>
#include <string>

#define LOG_DEBUG(...)                                                         \
    ::Logger::Log(::LogLevel::DEBUG, __FILE__, __LINE__,                       \
                  std::format(__VA_ARGS__))
#define LOG_INFO(...)                                                          \
    ::Logger::Log(::LogLevel::INFO, __FILE__, __LINE__,                        \
                  std::format(__VA_ARGS__))
#define LOG_WARNING(...)                                                       \
    ::Logger::Log(::LogLevel::WARNING, __FILE__, __LINE__,                     \
                  std::format(__VA_ARGS__))
#define LOG_ERROR(...)                                                         \
    ::Logger::Log(::LogLevel::ERROR, __FILE__, __LINE__,                       \
                  std::format(__VA_ARGS__))

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
public:
    static void Create(const std::string &logFilePath = "vulkan-app.log",
                       LogLevel level = LogLevel::DEBUG);
    static void Destroy();
    static Logger *GetInstance();

    static void Log(LogLevel level, const char *file, int line,
                    const std::string &message);

private:
    Logger(const std::string &logFilePath, LogLevel level);
    ~Logger();

    std::ofstream mLogFile;
    LogLevel mLogLevel;
};
