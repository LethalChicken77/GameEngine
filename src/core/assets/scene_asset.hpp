#pragma once
#include "../asset.hpp"
#include "../game_object.hpp"

namespace core
{
class SceneAsset : Asset
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
        SceneAsset(id_t newID) : Asset(newID) {}
        friend std::unique_ptr<SceneAsset> Object::Instantiate<SceneAsset>(std::string);

        // std::vector<GameObject> gameObjects;
};
} // namespace core