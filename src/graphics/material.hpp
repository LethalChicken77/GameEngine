#pragma once
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <string>
#include <variant>
#include <memory>
#include <stdexcept>

#include "shader.hpp"
#include "graphics_containers.hpp"

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
                glm::mat2,
                glm::mat3,
                glm::mat4
                // Texture Reference
                // Sampler Reference
                // Whatever else
            > Value;

            Material() = delete;
            Material(const Material&) = delete;
            Material& operator=(const Material&) = delete;
            Material(Material&&) = default;
            Material& operator=(Material&&) = default;

            static Material instantiate(Containers &_containers, const Shader *_shader)
            {
                static id_t next_id = 0;

                return Material(_containers, next_id++, _shader);
            }

            void assignDescriptorSet(VkDescriptorSet descriptorSet)
            {
                this->descriptorSet = descriptorSet;
            }

            void setValue(std::string name, Value value);

            void createShaderInputBuffer();

            uint32_t getId() const { return id; }
            VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
            Buffer *getBuffer() const { return buffer.get(); }

        private:
            Material(Containers &_contatiners, id_t mat_id, const Shader *_shader);

            uint32_t id;
            std::vector<Value> inputValues{};
            std::vector<uint8_t> data{};
            const Shader *shader;
            VkDescriptorSet descriptorSet;
            std::unique_ptr<Buffer> buffer;
            Containers &containers;

            bool initialized = false;
    };
} // namespace graphics