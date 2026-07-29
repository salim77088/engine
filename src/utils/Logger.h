// utils/Logger.h - Lightweight logging system
#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include <cstdarg>

namespace luminus {

enum class LogLevel {
    Trace,
    Info,
    Warn,
    Error,
    Fatal
};

class Logger {
public:
    static Logger& Get();
    
    void Log(LogLevel level, const std::string& category, const std::string& message);
    void SetLogFile(const std::string& path);
    void SetMinLevel(LogLevel level) { m_MinLevel = level; }
    void EnableConsole(bool enable) { m_ConsoleEnabled = enable; }
    
    struct LogEntry {
        LogLevel level;
        std::string category;
        std::string message;
        std::string timestamp;
    };
    
    const std::vector<LogEntry>& GetEntries() const { return m_Entries; }
    void ClearEntries() { m_Entries.clear(); }
    
    static std::string FormatArgs(const char* fmt, ...);

private:
    Logger() = default;
    std::mutex m_Mutex;
    std::ofstream m_FileStream;
    LogLevel m_MinLevel = LogLevel::Trace;
    bool m_ConsoleEnabled = true;
    std::vector<LogEntry> m_Entries;
    
    std::string GetTimestamp();
    std::string LevelToString(LogLevel level);
    int LevelToColor(LogLevel level);
};

} // namespace luminus

// Convenience macros
#define LM_TRACE(cat, ...)   ::luminus::Logger::Get().Log(::luminus::LogLevel::Trace, cat, ::luminus::Logger::FormatArgs(__VA_ARGS__))
#define LM_INFO(cat, ...)    ::luminus::Logger::Get().Log(::luminus::LogLevel::Info,  cat, ::luminus::Logger::FormatArgs(__VA_ARGS__))
#define LM_WARN(cat, ...)    ::luminus::Logger::Get().Log(::luminus::LogLevel::Warn,  cat, ::luminus::Logger::FormatArgs(__VA_ARGS__))
#define LM_ERROR(cat, ...)   ::luminus::Logger::Get().Log(::luminus::LogLevel::Error, cat, ::luminus::Logger::FormatArgs(__VA_ARGS__))
#define LM_FATAL(cat, ...)   ::luminus::Logger::Get().Log(::luminus::LogLevel::Fatal, cat, ::luminus::Logger::FormatArgs(__VA_ARGS__))
