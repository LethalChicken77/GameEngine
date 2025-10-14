#include "file_util.hpp"
#include "string"
#include "fstream"
#include "iostream"
#include <vector>
#include "console.hpp"
#include <filesystem>

std::string FileUtil::readFileToString(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        Console::error("File not found: " + path, "File Util");
        return "";
    }

    size_t fileSize = static_cast<size_t>(file.tellg());

    std::string content(fileSize, '\0'); // Create a string of the appropriate size
    
    file.seekg(0, std::ios::beg);  // Move back to the beginning of the file
    file.read(&content[0], fileSize);   // Read the file content into the string

    if (!file)
    {
        Console::error("Could not read file: " + path, "File Util");
        return "";
    }

    return content;
}

std::vector<char> FileUtil::readFileToCharVector(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        Console::error("File not found: " + path, "File Util");
        return std::vector<char>();
    }

    
    size_t fileSize = static_cast<size_t>(file.tellg());

    std::vector<char> content(fileSize); // Create a string of the appropriate size
    
    file.seekg(0, std::ios::beg);  // Move back to the beginning of the file
    file.read(&content[0], fileSize);   // Read the file content into the string

    if (!file)
    {
        Console::error("Could not read file: " + path, "File Util");
        return std::vector<char>();
    }

    return content;
}

std::vector<std::string> FileUtil::getFiles(const std::string& directoryPath)
{
    std::vector<std::string> paths;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if(entry.is_regular_file())
            paths.push_back(entry.path().string());
    }
}

std::vector<std::string> FileUtil::getFilesRecursive(const std::string& directoryPath)
{
    std::vector<std::string> paths;
    
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if(entry.is_regular_file())
            paths.push_back(entry.path().string());
    }

    std::vector<std::string> subDirs = getSubdirectories(directoryPath);
    for(const std::string &subDir : subDirs)
    {
        std::vector<std::string> subPaths = getFilesRecursive(subDir);
        paths.insert(paths.end(), subPaths.begin(), subPaths.end());
    }
}

std::vector<std::string> FileUtil::getSubdirectories(const std::string& directoryPath)
{
    std::vector<std::string> paths;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if(entry.is_directory())
            paths.push_back(entry.path().string());
    }
}

inline bool FileUtil::fileExists(const std::string& path)
{
    return std::filesystem::is_regular_file(path);
}

inline bool FileUtil::directoryExists(const std::string& directoryPath)
{
    return std::filesystem::is_directory(directoryPath);
}

inline bool FileUtil::folderExists(const std::string& directoryPath)
{
    return directoryExists(directoryPath);
}