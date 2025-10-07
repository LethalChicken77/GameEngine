#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>

#include "device.hpp"
#include "descriptors.hpp"
#include "shader_base.hpp"
// #include "graphics_pipeline.hpp"

namespace graphics
{
    class GraphicsPipeline;
    
    enum PipelineType
    {
        STANDARD = 0,
        POST_PROCESSING = 1,
        ID_BUFFER = 2
    };

    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;
        // PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        // PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
        PipelineType pipelineType;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        // VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass* renderPass = nullptr;
        uint32_t subpass = 0;
    };

    // Container to abstract away shader logic
    class Shader : public ShaderBase
    {
        public:
            Shader(const std::string &vPath, const std::string &fPath, std::vector<ShaderInput> inputs, uint32_t textureCount, VkRenderPass *renderPass);
            Shader(const std::string &vPath, const std::string &fPath, std::vector<ShaderInput> inputs, uint32_t textureCount, PipelineConfigInfo _configInfo);
            ~Shader();

            // Disallow copying of shaders
            Shader(const Shader&) = delete;
            Shader& operator=(const Shader&) = delete;

            PipelineConfigInfo& getConfigInfo() { return configInfo; };
            VkShaderModule& getVertexModule() { return vertShaderModule; }
            VkShaderModule& getFragmentModule() { return fragShaderModule; }
            const std::vector<ShaderInput>& getInputs() const { return inputs; }
            GraphicsPipeline* getPipeline() const { return parentPipeline; }
            void reloadShader(); // Rereads the shader files and recreates the shader modules

            bool dirty = false;

            PipelineConfigInfo configInfo{};

            GraphicsPipeline* parentPipeline;

            static PipelineConfigInfo getDefaultConfigInfo();
            static PipelineConfigInfo getDefaultTransparentConfigInfo();

        private:
            std::string vertexPath;
            std::string fragmentPath;

            VkShaderModule vertShaderModule{};
            VkShaderModule fragShaderModule{};


            // Descriptor Set pool and layout
            // DescriptorPool descriptorPool;
            // DescriptorSetLayout descriptorSetLayout;


    };
} // namespace graphics