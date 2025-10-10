#pragma once
#include "asset.hpp"
#include "game_object.hpp"
#include "object_manager.hpp"

namespace core
{
// Active scene representation in memory, loaded from SceneAsset
class Scene_t : public Object
{
    public:
        static constexpr const char* className = "Scene";
        const char* GetClassName() const { return className; }

        Scene_t(const Scene_t&) = delete;
        Scene_t& operator=(const Scene_t&) = delete;
        Scene_t(Scene_t&&) = delete;
        Scene_t& operator=(Scene_t&&) = delete;

        void loadScene();
        void update(double deltaTime);

        std::vector<GameObject> &getGameObjects() { return gameObjects; }

        void drawScene();

        std::vector<glm::mat4> transforms{};
        
    protected:
        Scene_t(id_t newID) : Object(newID) {}
        friend class ObjectManager;

        std::vector<GameObject> gameObjects{};
};

class Scene : public SmartRef<Scene_t>
{
    public:
        using SmartRef<Scene_t>::SmartRef;

        Scene(const std::string& sceneName = "New Scene");
};

} // namespace core