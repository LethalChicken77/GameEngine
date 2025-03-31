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
            uint32_t numParticles = 10000;
            float inertia;
            float sedimentCapacity;
            float sedimentDeposit;
            float minSlope;
            float erosionRate;
            float evaporationRate;
            float gravity;

            ErosionProperties()
            {
                inertia = 0.5f;
                sedimentCapacity = 0.1f;
                sedimentDeposit = 0.01f;
                minSlope = 0.1f;
                erosionRate = 0.01f;
                evaporationRate = 0.001f;
                gravity = 9.81f;
            }
        };
            HydraulicErosion(uint32_t resolution, ErosionProperties erosionProps);
            ~HydraulicErosion();

            void initializeParticles();

            void runIterationsCPU(uint32_t iterations, float dt);
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