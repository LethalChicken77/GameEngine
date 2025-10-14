#pragma once
#include "core/asset_data.hpp"
#include "core/game_object.hpp"

namespace core
{
class SceneAsset : AssetData
{
    public:
        SceneAsset(const SceneAsset&) = delete;
        SceneAsset& operator=(const SceneAsset&) = delete;
        SceneAsset(SceneAsset&&) = delete;
        SceneAsset& operator=(SceneAsset&&) = delete;

        // static std::unique_ptr<Scene> Instantiate(std::string name = "New Asset")
        // {
        //     std::unique_ptr<Scene> parent = Object::Instantiate<Scene>(name);
        //     return std::move(parent); // TODO: Put somewhere
        // }

        void drawScene();
        
    private:
        SceneAsset(id_t newID) : AssetData(newID) {}

        // std::vector<GameObject> gameObjects;
};
} // namespace core