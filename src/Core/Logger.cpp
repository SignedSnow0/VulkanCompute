#include "Logger.h"

#include <iomanip>

static Logger* sInstance = nullptr;

void Logger::Create(const std::string& logFilePath, LogLevel level) {
    if (!sInstance) {
        sInstance = new Logger(logFilePath, level);
    }
}

void Logger::Destroy() {
    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

Logger* Logger::GetInstance() {
    if (!sInstance) {
        Create();
    }
    return sInstance;
}

void Logger::Log(LogLevel level, const char* file, int line,
    const std::string& message) {
    Logger* logger = GetInstance();
    if (level < logger->mLogLevel) {
        return;
    }

    std::string levelStr;
    switch (level) {
    case LogLevel::DEBUG:
        levelStr = "DEBUG";
        break;
    case LogLevel::INFO:
        levelStr = "INFO";
        break;
    case LogLevel::WARNING:
        levelStr = "WARNING";
        break;
    case LogLevel::ERROR:
        levelStr = "ERROR";
        break;
    default:
        levelStr = "UNKNOWN";
        break;
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    logger->mLogFile << "[" << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << "] ["
        << levelStr << "] (" << file << ":" << line << ") "
        << message << std::endl;
}

Logger::Logger(const std::string& logFilePath, LogLevel level) {
    mLogFile = std::ofstream(logFilePath, std::ios::out);
    mLogLevel = level;
}

Logger::~Logger() {
    if (mLogFile.is_open()) {
        mLogFile.close();
    }
}
