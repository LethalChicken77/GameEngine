#pragma once
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <string>
#include <variant>
#include <memory>
#include <stdexcept>
#include "internal/descriptors.hpp"
#include "internal/device.hpp"
#include "buffers/buffer.hpp"
#include "buffers/texture.hpp"
// static_assert(sizeof(graphics::Texture) > 0, "Texture is not defined!");
#include "shader.hpp"
// #include "containers.hpp"

namespace graphics
{
    class ShaderResource
    {
        public:
            using id_t = uint64_t;

            // Determine the size and alignment for a given ShaderInput::DataType
            struct TypeInfo
            {
                size_t size;
                size_t alignment;
            };

            // void setValue(std::string name, Value value);

            // void createShaderInputBuffer();

            VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
            Buffer *getBuffer() const { return buffer.get(); }

        protected:
            std::vector<MaterialValue> inputValues{};
            std::vector<uint8_t> data{};
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            std::unique_ptr<Buffer> buffer;
            std::vector<Texture*> textures; // TODO: Possibly dangerous
            // std::vector<std::shared_ptr<Texture>> images;

            bool initialized = false;

            size_t alignTo(size_t offset, size_t alignment) const
            {
                // size_t minAlignment = Shared::device->properties.limits.minUniformBufferOffsetAlignment;
                // alignment = std::max(alignment, minAlignment);
                return (offset + alignment - 1) & ~(alignment - 1);
            }

            TypeInfo getTypeInfo(ShaderInput::DataType type) const
            {
                switch (type)
                {
                case ShaderInput::DataType::FLOAT: return {sizeof(float), 4};
                case ShaderInput::DataType::VEC2:  return {sizeof(glm::vec2), 8};
                case ShaderInput::DataType::VEC3:  return {sizeof(glm::vec3), 16}; // Vec3 uses Vec4 alignment
                case ShaderInput::DataType::VEC4:  return {sizeof(glm::vec4), 16};
                case ShaderInput::DataType::COLOR: return {sizeof(Color), 16};
                case ShaderInput::DataType::MAT2:  return {sizeof(glm::mat2), 16}; // Matrices align to vec4
                case ShaderInput::DataType::MAT3:  return {sizeof(glm::mat3), 16};
                case ShaderInput::DataType::MAT4:  return {sizeof(glm::mat4), 16};
                case ShaderInput::DataType::INT:   return {sizeof(int), 4};
                case ShaderInput::DataType::BOOL:  return {sizeof(int), 4}; // bools are treated as 4 bytes in std140
                default: throw std::runtime_error("Unknown ShaderInput::DataType");
                }
            }
    };
} // namespace graphics