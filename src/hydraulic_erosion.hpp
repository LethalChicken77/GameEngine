#pragma once
#include "graphics/compute_resource.hpp"
#include "graphics/compute_shader.hpp"
#include <glm/glm.hpp>

namespace game
{
    class HydraulicErosion
    {

        using ShaderResource = graphics::ShaderResource;
        using ComputeShader = graphics::ComputeShader;
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
            uint32_t numParticles = 50000;
            float friction;
            // float sedimentPickupRate;
            // float sedimentDepositRate;
            // float minSlope;
            float erosionRate;
            float evaporationRate;
            float gravity;
            float depositThreshold;

            ErosionProperties()
            {
                friction = 0.95f;
                // sedimentPickupRate = 0.1f;
                // sedimentDepositRate = 0.1f;
                // minSlope = 0.1f;
                erosionRate = 0.01f;
                evaporationRate = 0.001f;
                gravity = -0.1f;
                depositThreshold = 5.f;
            }
        };
            HydraulicErosion(uint32_t resolution, ErosionProperties erosionProps);
            ~HydraulicErosion();

            void initializeParticles();

            void runIterationsCPU(uint32_t iterations, float dt);
            void drawImgui();
            // void render();

        private:
            std::shared_ptr<Texture> heightMap;
            std::unique_ptr<ShaderResource> heightMapResource;
            std::unique_ptr<ComputeShader> computeShader;
            std::shared_ptr<Buffer> particleBuffer;

            std::vector<Particle> cpuParticles{};

            void initializeTexture(size_t resolution);
            
            ErosionProperties erosionProperties;
    };
};