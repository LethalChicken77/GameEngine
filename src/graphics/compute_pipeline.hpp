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
        struct ErosionPushConstants
        {
            uint32_t numParticles;
            uint32_t seed;

            int maxLifetime;
            float sedimentScale;
            float sedimentCapacity;
            float baseCapacity;
            float gravity;
            float friction;

            // uint numParticles;
            // uint seed;
        
            // int maxLifetime;
            // float sedimentScale;
            // float sedimentCapacity;
            // float baseCapacity;
            // float gravity;
            // float friction;
        };
        
        ComputePipeline(ComputeShader& _shader);
        ComputePipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
        ~ComputePipeline();

        ComputePipeline(const ComputePipeline&) = delete;
        ComputePipeline& operator=(const ComputePipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void dispatch(VkCommandBuffer cmdBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ, bool wait = false);
        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

    private:
        void createComputePipeline();
        void createComputePipeline(const std::string& path, const ComputePipelineConfigInfo& configInfo);
        void createPipelineLayout();

        VkPipeline m_computePipeline;
        ComputeShader &shader;
        VkPipelineCache pipelineCache;
        VkPipelineLayout pipelineLayout;
    };
}