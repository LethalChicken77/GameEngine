#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "device.hpp"
#include "compute_shader.hpp"

namespace graphics
{
    class ComputePipeline
    {
    public:
        ComputePipeline(Device &_device, ComputeShader& _shader);
        ComputePipeline(Device &_device, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
        ~ComputePipeline();

        ComputePipeline(const ComputePipeline&) = delete;
        ComputePipeline& operator=(const ComputePipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    private:
        void createComputePipeline();
        void createComputePipeline(const std::string& path, const ComputePipelineConfigInfo& configInfo);
        void createPipelineLayout();

        Device &device;
        VkPipeline m_computePipeline;
        ComputeShader &shader;
        VkPipelineCache pipelineCache;
    };
}