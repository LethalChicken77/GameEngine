#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>

#include "device.hpp"
#include "shader.hpp"
#include "shader_base.hpp"

namespace graphics
{
    struct ComputePipelineConfigInfo {
        ComputePipelineConfigInfo() = default;
        ComputePipelineConfigInfo(const ComputePipelineConfigInfo&) = delete;
        ComputePipelineConfigInfo& operator=(const ComputePipelineConfigInfo&) = delete;

        VkPipelineLayout pipelineLayout = nullptr;
    };

    // Container to abstract away compute shader logic
    class ComputeShader : ShaderBase
    {
        public:
            std::string path;

            ComputeShader(const std::string &_path, std::vector<ShaderInput> inputs);
            ~ComputeShader();

            // Disallow copying of shaders
            ComputeShader(const ComputeShader&) = delete;
            ComputeShader& operator=(const ComputeShader&) = delete;
            // Shader(Shader&&) = default;
            // Shader& operator=(Shader&&) = default;

            ComputePipelineConfigInfo& getConfigInfo() { return configInfo; };
            VkShaderModule& getShaderModule() { return computeShaderModule; }
            void reloadShader(); // Rereads the shader files and recreates the shader modules

            bool dirty = false;

        private:
            ComputePipelineConfigInfo configInfo{};

            VkShaderModule computeShaderModule{};

            void initializeDefaultConfigInfo();
    };
} // namespace graphics