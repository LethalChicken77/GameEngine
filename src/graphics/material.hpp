#pragma once
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <string>
#include <variant>

#include "shader.hpp"

namespace graphics
{
    class Material
    {
        public:
            typedef std::variant<
                int,
                bool,
                float,
                glm::vec2,
                glm::vec3,
                glm::vec4,
                glm::mat3,
                glm::mat3,
                glm::mat4,
                // Texture Reference
                // Sampler Reference
                // Whatever else
            > Value;

            Material(Shader shader, std::vector<Value> inputValues);
            ~Material();



        private:
            std::vector<Value> inputValues{};
    };
} // namespace graphics