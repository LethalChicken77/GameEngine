#pragma once
#include "asset.hpp"
#include "game_object.hpp"
#include "object_manager.hpp"

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

        void loadScene();
        void update(double deltaTime);

        std::vector<GameObject> &getGameObjects() { return gameObjects; }
        
    private:
        Scene_t(id_t newID) : Object(newID) {}

        std::vector<GameObject> gameObjects{};
};

class Scene : public SmartRef<Scene_t>
{
    public:
        using SmartRef<Scene_t>::SmartRef;
};

} // namespace core