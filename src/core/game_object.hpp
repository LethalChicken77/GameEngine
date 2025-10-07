#pragma once
#include "../graphics/mesh.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "object.hpp"
#include "transform.hpp"

namespace core
{
    class GameObject_t : public Object
    {
    public:
        GameObject_t(const GameObject_t&) = delete;
        GameObject_t& operator=(const GameObject_t&) = delete;
        GameObject_t(GameObject_t&&) = delete;
        GameObject_t& operator=(GameObject_t&&) = delete;

        static std::unique_ptr<GameObject_t> Instantiate(std::string name = "New Game Object")
        {
            std::unique_ptr<GameObject_t> parent = Object::Instantiate<GameObject_t>(name);
            return std::move(parent); // TODO: Put somewhere
        }

        id_t get_instance_id() const { return instanceID; }
        id_t get_id() const { return localID; }
        std::shared_ptr<graphics::Mesh> mesh{};

        Transform transform;

        id_t materialID{};
    private:
        GameObject_t(id_t newID) : Object(newID) {}
        id_t localID; // ID local to scene/prefab
        friend std::unique_ptr<GameObject_t> Object::Instantiate<GameObject_t>(std::string);
    };
    using GameObject = std::unique_ptr<GameObject_t>;
}