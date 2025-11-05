#pragma once

#include <format>
#include <fstream>
#include <string>

/**
 * @brief Macro used to log a message with DEBUG level.
 *
 * This macro captures the file name and line number where it is invoked,
 * and formats the message using std::format.
 */
#define LOG_DEBUG(...)                                                         \
    ::Logger::Log(::LogLevel::DEBUG, __FILE__, __LINE__,                       \
                  std::format(__VA_ARGS__))
/**
 * @brief Macro used to log a message with INFO level.
 *
 * This macro captures the file name and line number where it is invoked,
 * and formats the message using std::format.
 */
#define LOG_INFO(...)                                                          \
    ::Logger::Log(::LogLevel::INFO, __FILE__, __LINE__,                        \
                  std::format(__VA_ARGS__))
/**
 * @brief Macro used to log a message with WARNING level.
 *
 * This macro captures the file name and line number where it is invoked,
 * and formats the message using std::format.
 */
#define LOG_WARNING(...)                                                       \
    ::Logger::Log(::LogLevel::WARNING, __FILE__, __LINE__,                     \
                  std::format(__VA_ARGS__))
/**
 * @brief Macro used to log a message with ERROR level.
 *
 * This macro captures the file name and line number where it is invoked,
 * and formats the message using std::format.
 */
#define LOG_ERROR(...)                                                         \
    ::Logger::Log(::LogLevel::ERROR, __FILE__, __LINE__,                       \
                  std::format(__VA_ARGS__))

/**
 * @brief Enumeration of log levels.
 */
enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

/**
 * @brief Logger class for logging messages to a file with different severity
 * levels.
 *
 * This class implements a singleton pattern to ensure a single instance,
 * which can be accessed globally.
 * The user can choose the output log file by calling the Create method before
 * any log operations, by default it logs to "vulkan-app.log".
 */
class Logger {
public:
    /**
     * @brief Creates the singleton Logger instance.
     *
     * If the method is not called, a default log file "vulkan-app.log" is used.
     *
     * @param logFilePath The path to the log file. Defaults to
     * "vulkan-app.log".
     * @param level The minimum log level to record. Defaults to DEBUG.
     */
    static void Create(const std::string &logFilePath = "vulkan-app.log",
                       LogLevel level = LogLevel::DEBUG);
    /**
     * @brief Destroys the singleton Logger instance.
     */
    static void Destroy();

    /**
     * @brief Gets the singleton Logger instance.
     *
     * @return Pointer to the Logger instance.
     */
    static Logger *GetInstance();

    /**
     * @brief Logs a message with the specified log level, file name, and line
     * number.
     *
     * @param level The log level of the message.
     * @param file The name of the source file where the log is generated.
     * @param line The line number in the source file where the log is
     * generated.
     * @param message The log message.
     */
    static void Log(LogLevel level, const char *file, int line,
                    const std::string &message);

private:
    Logger(const std::string &logFilePath, LogLevel level);
    ~Logger();

    std::ofstream mLogFile;
    LogLevel mLogLevel;
};
