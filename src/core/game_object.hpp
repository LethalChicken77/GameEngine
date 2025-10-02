#pragma once
#include "../graphics/mesh.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "object.hpp"
#include "transform.hpp"

namespace core
{
    class GameObject : Object
    {
    public:

        GameObject() = delete;
        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject&&) = default;

        static GameObject Instantiate()
        {
            Object parent = Object::Instantiate();
            static id_t next_id = 0;
            return GameObject(next_id++);
        }

        id_t get_id() const { return localID; }
        std::shared_ptr<graphics::Mesh> mesh{};

        Transform transform;

        id_t materialID{};
    private:
        id_t localID; // ID local to scene/prefab
    };
}