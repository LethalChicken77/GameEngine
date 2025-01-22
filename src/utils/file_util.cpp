#include "file_util.hpp"
#include "string"
#include "fstream"
#include "iostream"
#include <vector>

namespace file_util
{
    std::string readFileToString(const std::string& path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << path << std::endl;
            return "";
        }

        size_t fileSize = static_cast<size_t>(file.tellg());

        std::string content(fileSize, '\0'); // Create a string of the appropriate size
        
        file.seekg(0, std::ios::beg);  // Move back to the beginning of the file
        file.read(&content[0], fileSize);   // Read the file content into the string

        if (!file)
        {
            std::cerr << "Failed to read file: " << path << std::endl;
            return "";
        }

        file.close();

        return content;
    }

    std::vector<char> readFileToCharVector(const std::string& path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + path);
        }

        
        size_t fileSize = static_cast<size_t>(file.tellg());

        std::vector<char> content(fileSize); // Create a string of the appropriate size
        
        file.seekg(0, std::ios::beg);  // Move back to the beginning of the file
        file.read(&content[0], fileSize);   // Read the file content into the string

        if (!file)
        {
            throw std::runtime_error("Failed to read file: " + path);
        }

        file.close();

        return content;
    }
}