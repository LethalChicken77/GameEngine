#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>
#include <variant>

#include "utils/console.hpp"
#include "internal/device.hpp"
#include "internal/descriptors.hpp"

#include "color.hpp"

namespace graphics
{
    typedef std::variant<
        int,
        bool,
        float,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        Color,
        glm::mat2,
        glm::mat3,
        glm::mat4
        // Texture Reference
        // Sampler Reference
        // Whatever else
    > MaterialValue;
            
    struct ShaderInput
    {
        std::string name;
        enum class DataType
        {
            FLOAT = 0,
            VEC2 = 1,
            VEC3 = 2,
            VEC4 = 3,
            COLOR = 4,
            MAT2 = 5,
            MAT3 = 6,
            MAT4 = 7,
            INT = 8,
            BOOL = 9,

            INVALID = -1
        } type;

        static DataType getTypeID(const MaterialValue& value)
        {
            return std::visit([](auto&& val) -> DataType {
                using T = std::decay_t<decltype(val)>;

                if constexpr (std::is_same_v<T, float>)                 return DataType::FLOAT;
                else if constexpr (std::is_same_v<T, glm::vec2>)        return DataType::VEC2;
                else if constexpr (std::is_same_v<T, glm::vec3>)        return DataType::VEC3;
                else if constexpr (std::is_same_v<T, glm::vec4>)        return DataType::VEC4;
                else if constexpr (std::is_same_v<T, graphics::Color>)  return DataType::COLOR;
                else if constexpr (std::is_same_v<T, glm::mat2>)        return DataType::MAT2;
                else if constexpr (std::is_same_v<T, glm::mat3>)        return DataType::MAT3;
                else if constexpr (std::is_same_v<T, glm::mat4>)        return DataType::MAT4;
                else if constexpr (std::is_same_v<T, int>)              return DataType::INT;
                else if constexpr (std::is_same_v<T, bool>)             return DataType::BOOL;
                else
                    throw std::runtime_error("Unsupported MaterialValue type");
            }, value);
        }

        template <typename T> static DataType getTypeID();
        template <> inline constexpr DataType getTypeID<float>() { return DataType::FLOAT; }
        template <> inline constexpr DataType getTypeID<glm::vec2>() { return DataType::VEC2; }
        template <> inline constexpr DataType getTypeID<glm::vec3>() { return DataType::VEC3; }
        template <> inline constexpr DataType getTypeID<glm::vec4>() { return DataType::VEC4; }
        template <> inline constexpr DataType getTypeID<Color>() { return DataType::COLOR; }
        template <> inline constexpr DataType getTypeID<glm::mat2>() { return DataType::MAT2; }
        template <> inline constexpr DataType getTypeID<glm::mat3>() { return DataType::MAT3; }
        template <> inline constexpr DataType getTypeID<glm::mat4>() { return DataType::MAT4; }
        template <> inline constexpr DataType getTypeID<int>() { return DataType::INT; }
        template <> inline constexpr DataType getTypeID<bool>() { return DataType::BOOL; }
    };

    // Container to abstract away shader logic
    class ShaderBase
    {
        public:
            ShaderBase(std::vector<ShaderInput> _inputs) : inputs(_inputs) {}   

            const std::vector<ShaderInput>& getInputs() const { return inputs; }
            virtual void reloadShader() = 0; // Rereads the shader files and recreates the shader modules

            bool dirty = false;

            DescriptorPool* getDescriptorPool() const { return descriptorPool.get(); }
            DescriptorSetLayout* getDescriptorSetLayout() const { return descriptorSetLayout.get(); }

        protected:
            const std::vector<ShaderInput> inputs{};

            // Descriptor Set pool and layout
            std::unique_ptr<DescriptorPool> descriptorPool;
            std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;

            // virtual void initializeDefaultConfigInfo() = 0; // Implement in sub-classes

            void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
            
            static std::vector<uint32_t> SlangToSpirv(const std::vector<char>& shaderData, const char* moduleName, const char* entryPointName, SlangStage slangStage);
    };
} // namespace graphics