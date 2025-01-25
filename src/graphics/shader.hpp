#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>

#include "device.hpp"

namespace graphics
{
    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    // Container to abstract away shader logic
    class Shader
    {
        public:
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

            std::string vertexPath;
            std::string fragmentPath;

            Shader(Device &_device, const std::string &vPath, const std::string &fPath, std::vector<ShaderInput> inputs);
            ~Shader();

            // Disallow copying of shaders
            Shader(const Shader&) = delete;
            Shader& operator=(const Shader&) = delete;
            // Shader(Shader&&) = default;
            // Shader& operator=(Shader&&) = default;

            PipelineConfigInfo& getConfigInfo() { return configInfo; };
            VkShaderModule& getVertexModule() { return vertShaderModule; }
            VkShaderModule& getFragmentModule() { return fragShaderModule; }
            const std::vector<ShaderInput>& getInputs() const { return inputs; }
            void reloadShader(); // Rereads the shader files and recreates the shader modules

            bool dirty = false;

        private:
            Device &device;
            const std::vector<ShaderInput> inputs{};
            PipelineConfigInfo configInfo{};

            VkShaderModule vertShaderModule;
            VkShaderModule fragShaderModule;

            void initializeDefaultConfigInfo();

            void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
    };
} // namespace graphics