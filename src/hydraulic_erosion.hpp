#pragma once
#include "graphics/compute_resource.hpp"
#include "graphics/compute_shader.hpp"
#include "graphics/compute_pipeline.hpp"
#include <glm/glm.hpp>

namespace game
{
    class HydraulicErosion
    {
        using ComputeShader = graphics::ComputeShader;
        using ComputeResource = graphics::ComputeResource;
        using Device = graphics::Device;
        using Buffer = graphics::Buffer;
        using Texture = graphics::Texture;
        using TextureProperties = graphics::TextureProperties;
        using SamplerProperties = graphics::SamplerProperties;

        public:
        struct Particle
        {
            glm::vec2 position{};
            glm::vec2 velocity{};
            float sediment = 0.0f;
        };

        struct ErosionProperties // Pass as push constants
        {
            int maxLifetime = 4000;
            
            float sedimentScale = 0.01f;

            float sedimentCapacity = 8.0f;
            float baseCapacity = 0.1f;
            // float erosionRate = 0.1f;
            // float depositionRate = 0.1f;

            float gravity = -10.f;
            float friction = 0.8f;
        };
        
            HydraulicErosion(uint32_t resolution, ErosionProperties erosionProps);
            ~HydraulicErosion();

            void runIterationsCPU(uint32_t iterations);
            void runIterationsGPU(uint32_t iterations);
            void drawImgui();
            // void render();

        private:
            std::shared_ptr<Texture> heightMap;
            std::unique_ptr<ComputeResource> computeResource;
            std::unique_ptr<ComputeShader> computeShader;
            // std::shared_ptr<Buffer> particleBuffer;
            std::unique_ptr<graphics::ComputePipeline> computePipeline;

            std::vector<Particle> cpuParticles{};

            void initializeTexture(size_t resolution);
            
            ErosionProperties erosionProperties;
    };
};