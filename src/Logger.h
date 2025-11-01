#pragma once

#include <format>
#include <fstream>
#include <string>

#define LOG_DEBUG(message)                                                     \
    ::Logger::Log(::LogLevel::DEBUG, __FILE__, __LINE__, message)
#define LOG_INFO(message)                                                      \
    ::Logger::Log(::LogLevel::INFO, __FILE__, __LINE__, message)
#define LOG_WARNING(message)                                                   \
    ::Logger::Log(::LogLevel::WARNING, __FILE__, __LINE__, message)
#define LOG_ERROR(message)                                                     \
    ::Logger::Log(::LogLevel::ERROR, __FILE__, __LINE__, message)

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
