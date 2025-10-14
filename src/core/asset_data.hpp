#pragma once
#include <string>
#include <memory>

#include "object.hpp"
#include "utils/smart_reference.hpp"
// #include "asset_manager.hpp"

namespace core
{
    class AssetData : public Object
    {
    public:
        AssetData(const AssetData&) = delete;
        AssetData& operator=(const AssetData&) = delete;
        AssetData(AssetData&&) = delete;
        AssetData& operator=(AssetData&&) = delete;

        id_t getUUID() const { return UUID; }
        const std::string &getPath() const { return path; }

        void LoadData(); // Implement in children

    protected:
        id_t UUID; // Unique file ID assigned by asset manager. Persistent across sessions
        std::string path; // Located based on file ID
        std::vector<char> data;
        using Object::Object;
        
        // AssetData(id_t newID) : Object(newID) {};
    };
} // namespace core