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

namespace graphics
{
    struct ComputePipelineConfigInfo {
        ComputePipelineConfigInfo() = default;
        ComputePipelineConfigInfo(const ComputePipelineConfigInfo&) = delete;
        ComputePipelineConfigInfo& operator=(const ComputePipelineConfigInfo&) = delete;

        VkPipelineLayout pipelineLayout = nullptr;
    };

    // Container to abstract away shader logic
    class ComputeShader
    {
        public:
            std::string path;

            ComputeShader(Device &_device, const std::string &_path, std::vector<ShaderInput> inputs);
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

        private:
            Device &device;
            const std::vector<ShaderInput> inputs{};
            ComputePipelineConfigInfo configInfo{};

            VkShaderModule computeShaderModule;

            void initializeDefaultConfigInfo();

            void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
    };
} // namespace graphics