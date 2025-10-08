#pragma once
#include <string>
#include <memory>

#include "object.hpp"

namespace core
{
    class AssetData : public Object
    {
    public:
        AssetData(const AssetData&) = delete;
        AssetData& operator=(const AssetData&) = delete;
        AssetData(AssetData&&) = delete;
        AssetData& operator=(AssetData&&) = delete;

    protected:
        id_t UUID; // Unique file ID assigned by asset manager
        std::string path; // Located based on file ID
        
        AssetData(id_t newID) : Object(newID) {};
    };
} // namespace core