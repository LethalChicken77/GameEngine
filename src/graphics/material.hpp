#pragma once
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <string>
#include <variant>
#include <memory>

#include "shader.hpp"

namespace graphics
{
    class Material
    {
        public:
            using id_t = uint64_t;
            typedef std::variant<
                int,
                bool,
                float,
                glm::vec2,
                glm::vec3,
                glm::vec4,
                glm::mat3,
                glm::mat3,
                glm::mat4
                // Texture Reference
                // Sampler Reference
                // Whatever else
            > Value;

            Material() = delete;

            static Material instantiate(const Shader &_shader, std::vector<Value> inputValues)
            {
                static id_t next_id = 0;

                return Material(next_id++, _shader, inputValues);
            }

            void assignDescriptorSet(VkDescriptorSet *descriptorSet)
            {
                this->descriptorSet = descriptorSet;
            }

        private:
            Material(id_t mat_id, const Shader &_shader, std::vector<Value> inputValues) : id(mat_id), shader(_shader) {};
            uint32_t id;
            std::vector<Value> inputValues{};
            const Shader &shader;
            VkDescriptorSet *descriptorSet;
    };
} // namespace graphics