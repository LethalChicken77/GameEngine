#pragma once
#include <string>
#include <vector>

namespace file_util
{
    extern std::string readFileToString(const std::string& path);
    extern std::vector<char> readFileToCharVector(const std::string& path);
}