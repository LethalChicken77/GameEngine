#pragma once
#include <string>
#include <unordered_map>
#include "asset_data.hpp"
#include "object_manager.hpp"
#include <regex>
#include <random>

namespace core
{
    class AssetManager
    {
    public:
        AssetManager() = delete;
        ~AssetManager() = delete;
        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;

        template<class T>
        static T* Instantiate(const std::string &path)
        {
            static_assert(std::is_base_of<AssetData, T>::value, "Instantiated assets must derive from AssetData");
            // Check if file exists
            

            // Extract filename
            std::regex filenameRegex(R"([^/\\]+$)"); // match after last slash
            std::smatch match;
            std::regex_search(path, match, filenameRegex);

            T* obj = ObjectManager::InstantiateInternal<T>(match.str());

            AssetData* assetData = static_cast<AssetData*>(obj);
            id_t newID = createUUID();
            while(assets.contains(newID)) // Should practically never run, but just in case
            {
                newID = createUUID();
            }
            assetData->UUID = newID;
            // assetData->LoadAsset();

            return obj;
        }

    private:
        static std::unordered_map<id_t, AssetData*> assets;

        static id_t createUUID()
        {
            std::random_device rd;
            std::mt19937_64 engine(rd());
            std::uniform_int_distribution<uint64_t> dist;

            return dist(engine);
        }

        friend class AssetData;
    };
}