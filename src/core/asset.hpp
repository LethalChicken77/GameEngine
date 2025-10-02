#pragma once
#include <string>

#include "object.hpp"

namespace core
{
    // Save in .meta files
    class Asset : Object
    {
        public:

        private:
            id_t UUID; // Unique file ID
            std::string path;
    };
} // namespace core