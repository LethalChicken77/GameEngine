#pragma once
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <string>
#include <variant>
#include <memory>
#include <stdexcept>
#include "descriptors.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "texture.hpp"
// static_assert(sizeof(graphics::Texture) > 0, "Texture is not defined!");
#include "shader.hpp"

namespace graphics
{
    class ShaderResource
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

            void assignDescriptorSet(VkDescriptorSet descriptorSet)
            {
                this->descriptorSet = descriptorSet;
            }

            // void setValue(std::string name, Value value);

            // void createShaderInputBuffer();

            VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
            Buffer *getBuffer() const { return buffer.get(); }

        protected:
            std::vector<Value> inputValues{};
            std::vector<uint8_t> data{};
            VkDescriptorSet descriptorSet;
            std::unique_ptr<Buffer> buffer;
            std::vector<std::shared_ptr<Texture>> textures;

            bool initialized = false;
    };
} // namespace graphics