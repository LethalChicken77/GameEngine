#pragma once
#include "../graphics/mesh.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace core
{
    struct Transform
    {
        Transform() : position(0.0f), rotation(0.0f), scale(1.0f) {}

        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

        glm::mat4 getTransform() const;

        glm::vec3 forward() const
        {
            return glm::vec3(getTransform()[2]);
        }
        glm::vec3 right() const
        {
            return glm::vec3(getTransform()[0]);
        }
        glm::vec3 up() const
        {
            return glm::vec3(getTransform()[1]);
        }
    };

    class GameObject
    {
    public:
        using id_t = uint64_t;

        GameObject() = delete;
        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject&&) = default;

        static GameObject instantiate()
        {
            static id_t next_id = 0;
            return GameObject(next_id++);
        }

        id_t get_id() const { return id; }
        std::shared_ptr<graphics::Mesh> mesh{};

        Transform transform;
    private:
        GameObject(id_t obj_id) : id(obj_id) {};
        id_t id;
    };
}