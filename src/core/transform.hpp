#pragma once
#include <glm/glm.hpp>

namespace core
{
    
    struct Transform
    {
        Transform() : position(0.0f), rotation(0.0f), scale(1.0f) {}

        glm::vec3 position;
        glm::vec3 rotation; // Rotation in radians
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
} // namespace core