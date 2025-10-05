#pragma once
#include "asset.hpp"
#include "game_object.hpp"

namespace core
{
// Active scene representation in memory, loaded from SceneAsset
class Scene_t : Object
{
    public:
        Scene_t(const Scene_t&) = delete;
        Scene_t& operator=(const Scene_t&) = delete;
        Scene_t(Scene_t&&) = delete;
        Scene_t& operator=(Scene_t&&) = delete;

        static std::unique_ptr<Scene_t> Instantiate(std::string name = "New Asset") // TODO: Load from SceneAsset data
        {
            std::unique_ptr<Scene_t> parent = Object::Instantiate<Scene_t>(name);
            return std::move(parent); // TODO: Put somewhere
        }

        void loadScene();
        void update(double deltaTime);

        std::vector<GameObject> &getGameObjects() { return gameObjects; }
        
    private:
        Scene_t(id_t newID) : Object(newID) {}
        friend std::unique_ptr<Scene_t> Object::Instantiate<Scene_t>(std::string);

        std::vector<GameObject> gameObjects{};
};
using Scene = std::unique_ptr<Scene_t>;
} // namespace core