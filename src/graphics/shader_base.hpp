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

#include "device.hpp"
#include "descriptors.hpp"

namespace graphics
{    
    struct ShaderInput
    {
        std::string name;
        enum class DataType
        {
            FLOAT,
            VEC2,
            VEC3,
            VEC4,
            MAT2,
            MAT3,
            MAT4,
            INT,
            BOOL
        } type;
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

            virtual void initializeDefaultConfigInfo() = 0; // Implement in sub-classes

            void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
            
            static std::vector<uint32_t> SlangToSpirv(const std::vector<char>& shaderData, const char* moduleName, const char* entryPointName, SlangStage slangStage);
    };
} // namespace graphics