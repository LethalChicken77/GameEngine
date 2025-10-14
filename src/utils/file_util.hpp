#pragma once
#include <string>
#include <vector>

class FileUtil
{
    public:
        static std::string readFileToString(const std::string& path);
        static std::vector<char> readFileToCharVector(const std::string& path);
        static std::vector<std::string> getFiles(const std::string& directoryPath);
        static std::vector<std::string> getFilesRecursive(const std::string& directoryPath);
        static std::vector<std::string> getSubdirectories(const std::string& directoryPath);
        static inline bool fileExists(const std::string& path);
        static inline bool directoryExists(const std::string& directoryPath);
        static inline bool folderExists(const std::string& directoryPath); // Alias for directoryExists
};