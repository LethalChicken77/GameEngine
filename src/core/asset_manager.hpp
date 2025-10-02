#pragma once
#include <string>
#include <unordered_map>
#include "asset.hpp"

namespace core
{
    class AssetManager
    {
    public:
        using id_t = uint64_t;
        AssetManager() = delete;
        ~AssetManager() = delete;
        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;

        static void registerAsset(std::unique_ptr<Asset>);

    private:
        static std::unordered_map<id_t, std::unique_ptr<Asset>> assets;
    };
}