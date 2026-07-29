// utils/FileIO.h - File system utilities
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace luminus {

class FileIO {
public:
    // Read entire file as text
    static std::string ReadText(const std::string& path);
    
    // Read entire file as binary
    static std::vector<uint8_t> ReadBinary(const std::string& path);
    
    // Write text to file
    static bool WriteText(const std::string& path, const std::string& content);
    
    // Write binary to file
    static bool WriteBinary(const std::string& path, const std::vector<uint8_t>& data);
    
    // Check if file exists
    static bool Exists(const std::string& path);
    
    // Get file extension (lowercase, no dot)
    static std::string GetExtension(const std::string& path);
    
    // Get filename without extension
    static std::string GetBaseName(const std::string& path);
    
    // Get directory part
    static std::string GetDirectory(const std::string& path);
    
    // List files in directory
    static std::vector<std::string> ListFiles(const std::string& dir, bool recursive = false);
    
    // Create directory (recursive)
    static bool MakeDirs(const std::string& path);
    
    // Get executable directory
    static std::string GetExeDir();
    
    // Join paths
    static std::string Join(const std::string& a, const std::string& b);
};

} // namespace luminus
