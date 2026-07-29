// utils/FileIO.cpp
#include "FileIO.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace luminus {

std::string FileIO::ReadText(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        LM_ERROR("FileIO", "Cannot open file for reading: %s", path.c_str());
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::vector<uint8_t> FileIO::ReadBinary(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return {};
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)),
                               std::istreambuf_iterator<char>());
    return data;
}

bool FileIO::WriteText(const std::string& path, const std::string& content) {
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    f << content;
    return f.good();
}

bool FileIO::WriteBinary(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    f.write(reinterpret_cast<const char*>(data.data()), data.size());
    return f.good();
}

bool FileIO::Exists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

std::string FileIO::GetExtension(const std::string& path) {
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return "";
    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

std::string FileIO::GetBaseName(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    std::string fname = (slash == std::string::npos) ? path : path.substr(slash + 1);
    size_t dot = fname.find_last_of('.');
    if (dot != std::string::npos) fname = fname.substr(0, dot);
    return fname;
}

std::string FileIO::GetDirectory(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    return (slash == std::string::npos) ? "." : path.substr(0, slash);
}

std::vector<std::string> FileIO::ListFiles(const std::string& dir, bool recursive) {
    std::vector<std::string> result;
    DIR* d = opendir(dir.c_str());
    if (!d) return result;
    
    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        
        std::string full = Join(dir, entry->d_name);
        struct stat info;
        if (stat(full.c_str(), &info) != 0) continue;
        
        if (S_ISDIR(info.st_mode)) {
            if (recursive) {
                auto sub = ListFiles(full, true);
                result.insert(result.end(), sub.begin(), sub.end());
            }
        } else {
            result.push_back(full);
        }
    }
    closedir(d);
    return result;
}

bool FileIO::MakeDirs(const std::string& path) {
    if (path.empty()) return false;
    if (Exists(path)) return true;
    
    std::string parent = GetDirectory(path);
    if (!parent.empty() && parent != path) {
        if (!MakeDirs(parent)) return false;
    }
    
#ifdef PLATFORM_WINDOWS
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

std::string FileIO::GetExeDir() {
#ifdef PLATFORM_WINDOWS
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string p(path);
    return GetDirectory(p);
#else
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len < 0) return ".";
    path[len] = '\0';
    return GetDirectory(std::string(path));
#endif
}

std::string FileIO::Join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    char last = a.back();
    if (last == '/' || last == '\\') return a + b;
    return a + "/" + b;
}

} // namespace luminus
