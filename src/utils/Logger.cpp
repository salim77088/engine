// utils/Logger.cpp
#include "Logger.h"
#include <ctime>
#include <cstdio>
#include <cstdarg>
#include <iostream>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace luminus {

Logger& Logger::Get() {
    static Logger instance;
    return instance;
}

std::string Logger::GetTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* tm_info = std::localtime(&now);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
    return std::string(buffer);
}

std::string Logger::LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Info:  return "INFO ";
        case LogLevel::Warn:  return "WARN ";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
    }
    return "?    ";
}

int Logger::LevelToColor(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return 90;  // gray
        case LogLevel::Info:  return 37;  // white
        case LogLevel::Warn:  return 33;  // yellow
        case LogLevel::Error: return 31;  // red
        case LogLevel::Fatal: return 35;  // magenta
    }
    return 37;
}

void Logger::Log(LogLevel level, const std::string& category, const std::string& message) {
    if (level < m_MinLevel) return;
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::string ts = GetTimestamp();
    std::string lvl = LevelToString(level);
    
    LogEntry entry{level, category, message, ts};
    m_Entries.push_back(entry);
    if (m_Entries.size() > 1000) m_Entries.erase(m_Entries.begin());
    
    if (m_ConsoleEnabled) {
#ifdef PLATFORM_WINDOWS
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        WORD color = 7;
        switch (level) {
            case LogLevel::Trace: color = 8;  break;
            case LogLevel::Info:  color = 7;  break;
            case LogLevel::Warn:  color = 14; break;
            case LogLevel::Error: color = 12; break;
            case LogLevel::Fatal: color = 13; break;
        }
        SetConsoleTextAttribute(hConsole, color);
        std::cout << "[" << ts << "] [" << lvl << "] [" << category << "] " << message << std::endl;
        SetConsoleTextAttribute(hConsole, 7);
#else
        std::cout << "\033[" << LevelToColor(level) << "m"
                  << "[" << ts << "] [" << lvl << "] [" << category << "] " << message
                  << "\033[0m" << std::endl;
#endif
    }
    
    if (m_FileStream.is_open()) {
        m_FileStream << "[" << ts << "] [" << lvl << "] [" << category << "] " << message << "\n";
        m_FileStream.flush();
    }
}

void Logger::SetLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_FileStream.is_open()) m_FileStream.close();
    m_FileStream.open(path, std::ios::out | std::ios::app);
    if (m_FileStream.is_open()) {
        m_FileStream << "=== Luminus Engine Log Session ===\n";
    }
}

std::string Logger::FormatArgs(const char* fmt, ...) {
    char buffer[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return std::string(buffer);
}

} // namespace luminus
