#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>

#include "graphics/shader.hpp"
#include "graphics/shader_base.hpp"

namespace graphics
{
    struct ComputePipelineConfigInfo {
        ComputePipelineConfigInfo() = default;
        // ComputePipelineConfigInfo(const ComputePipelineConfigInfo&) = delete;
        // ComputePipelineConfigInfo& operator=(const ComputePipelineConfigInfo&) = delete;
    };

    // Container to abstract away compute shader logic
    class ComputeShader : public ShaderBase
    {
        public:
            std::string path;

            ComputeShader(const std::string &_path, std::vector<ShaderInput> inputs, uint32_t textureCount);
            ~ComputeShader();

            // Disallow copying of shaders
            ComputeShader(const ComputeShader&) = delete;
            ComputeShader& operator=(const ComputeShader&) = delete;
            // Shader(Shader&&) = default;
            // Shader& operator=(Shader&&) = default;

            ComputePipelineConfigInfo& getConfigInfo() { return configInfo; };
            VkShaderModule& getShaderModule() { return computeShaderModule; }
            const std::vector<ShaderInput>& getInputs() const { return inputs; }
            void reloadShader(); // Rereads the shader files and recreates the shader modules

            bool dirty = false;

            static ComputePipelineConfigInfo getDefaultConfigInfo();

        private:
            ComputePipelineConfigInfo configInfo{};

            VkShaderModule computeShaderModule{};
    };
} // namespace graphics