#include "hydraulic_erosion.hpp"
#include "procedural/noise.hpp"
#include "graphics/containers.hpp"
#include "graphics/material.hpp"
#include "imgui.h"

namespace game
{
    using graphics::ShaderResource;
    using graphics::ShaderInput;
    using graphics::ComputeShader;

    HydraulicErosion::HydraulicErosion(uint32_t resolution, ErosionProperties erosionProps) : erosionProperties(erosionProps)
    {
        erosionProperties = erosionProps;
        // Initialize heightMap, computeShader, and particleBuffer here
        // For example:
        // heightMap = std::make_shared<Texture>(width, height);
        // computeShader = std::make_shared<ComputeShader>("path/to/shader.spv");
        // particleBuffer = std::make_shared<Buffer>(sizeof(Particle) * numParticles, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);


        // computeShader = std::make_unique<ComputeShader>("shaders/hydraulic_erosion.comp", std::vector<ShaderInput>{
        //     // {"color", ShaderInput::DataType::VEC3},
        // });
        initializeTexture(resolution);
        // heightMapResource = std::make_unique<ShaderResource>(heightMap, 0, 0, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        
        
        initializeParticles();
    }

    HydraulicErosion::~HydraulicErosion()
    {
        // Cleanup resources if necessary
    }

    void HydraulicErosion::drawImgui()
    {
        ImGui::Text("Erosion Properties:");
        ImGui::SliderFloat("Erosion Rate", &erosionProperties.erosionRate, 0.0f, 0.1f);
        erosionProperties.erosionRate = glm::clamp(erosionProperties.erosionRate, 0.0f, 1.0f);
        // ImGui::SliderFloat("Evaporation Rate", &erosionProperties.evaporationRate, 0.0f, 1.0f);
        ImGui::SliderFloat("Gravity", &erosionProperties.gravity, -1.0f, 1.0f);
        ImGui::SliderFloat("Deposit Threshold", &erosionProperties.depositThreshold, 0.0f, 20.0f);
        ImGui::SliderFloat("Friction", &erosionProperties.friction, 0.0f, 1.0f);
        erosionProperties.friction = glm::clamp(erosionProperties.friction, 0.0f, 1.0f);
    }

    void HydraulicErosion::initializeParticles()
    {
        cpuParticles.resize(erosionProperties.numParticles);
        for(uint32_t i = 0; i < erosionProperties.numParticles; i++)
        {
            cpuParticles[i].position = glm::vec2(core::Random::getRandom01(), core::Random::getRandom01()) * (float)heightMap->getWidth();
            cpuParticles[i].velocity = glm::vec2(0.0f, 0.0f);
            cpuParticles[i].sediment = 0.0f;
        }
    }

    void HydraulicErosion::initializeTexture(size_t resolution)
    {
        std::cout << "Generating Heightmap" << std::endl;
        TextureProperties properties = TextureProperties().getDefaultProperties();
        SamplerProperties samplerProperties = SamplerProperties().getDefaultProperties();
        properties.format = VK_FORMAT_R32_SFLOAT;
        properties.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        properties.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // Allow usage for reads and writes, no need for shader read
        samplerProperties.magFilter = VK_FILTER_LINEAR;
        samplerProperties.addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        heightMap = std::make_shared<Texture>(properties, samplerProperties, resolution, resolution);
        
        for(int i = 0; i < resolution; i++)
        {
            for(int j = 0; j < resolution; j++)
            {
                // float height = core::Random::getRandom01();
                float height1 = procedural::simplex2D(i / (resolution * 0.5f), j / (resolution * 0.5f), 69);
                float height2 = procedural::simplex2D(i / (resolution * 0.25f), j / (resolution * 0.25f), 21) * 0.5f;
                float height3 = procedural::simplex2D(i / (resolution * 0.125f), j / (resolution * 0.125f), 420) * 0.25f;
                float height4 = procedural::simplex2D(i / (resolution * 0.0625f), j / (resolution * 0.0625f), 1) * 0.125f;
                float height5 = procedural::simplex2D(i / (resolution * 0.03125f), j / (resolution * 0.03125f), 2) * 0.0625f;
                heightMap->setPixel(i, j, (height1 + height2 + height3 + height4 + height5) * 30.f);
            }
        }
        heightMap->createTexture();
        graphics::Shared::materials[0].setTexture(3, heightMap);
        graphics::Shared::materials[0].updateDescriptorSet();
    }
    
    void HydraulicErosion::runIterationsCPU(uint32_t iterations, float dt)
    {
        float ds = 0.0001f;
        dt = 0.01f;
        for(Particle &particle : cpuParticles)
        {
            particle.position = glm::clamp(particle.position, glm::vec2(0.0f), glm::vec2(heightMap->getWidth() - 1.0f));
            
            glm::vec2 texturePos = particle.position / (float)heightMap->getWidth();
            float height = heightMap->getPixelFloat((uint32_t)particle.position.x, (uint32_t)particle.position.y);
            float heightX = heightMap->getPixelFloat((uint32_t)particle.position.x + 1, (uint32_t)particle.position.y);
            float heightY = heightMap->getPixelFloat((uint32_t)particle.position.x, (uint32_t)particle.position.y + 1);
            float heightNX = heightMap->getPixelFloat((uint32_t)particle.position.x - 1, (uint32_t)particle.position.y);
            float heightNY = heightMap->getPixelFloat((uint32_t)particle.position.x, (uint32_t)particle.position.y - 1);
            glm::vec2 gradient = glm::vec2(heightX - height, heightY - height) / ds;
            
            float lowestNeighbor = std::min(std::min(heightX, heightY), std::min(heightNX, heightNY));

            float newHeight = height;
            float velocityLength = glm::length(particle.velocity);
            float pickupRate = ((velocityLength / erosionProperties.depositThreshold) - 1.0f) * erosionProperties.erosionRate * dt;
            if(glm::dot(particle.velocity, gradient) < 0.0f || true)
            {
                if(&particle == &cpuParticles[0])
                {
                    std::cout << velocityLength << std::endl;
                    std::cout << pickupRate << std::endl;
                }
                if(pickupRate > 0.0f) // erosion
                {
                    newHeight = glm::max(newHeight - pickupRate, lowestNeighbor);
                    particle.sediment += pickupRate;
                }
                else // deposition
                {
                    float depositAmount = glm::min(particle.sediment, -pickupRate);
                    newHeight += depositAmount;
                    particle.sediment -= depositAmount;
                }
            }
            else if(particle.sediment > 0.0f)
            {
                float newSediment = glm::max(particle.sediment - erosionProperties.erosionRate * dt * (1 - glm::length(particle.velocity)), 0.0f);
                float sedimentDelta = newSediment - particle.sediment;
                newHeight += sedimentDelta;
            }
            
            // particle.sediment += newHeight - height;
            heightMap->setPixel((uint32_t)particle.position.x, (uint32_t)particle.position.y, newHeight);
            // heightMap->setPixel(0, 0, pixelHeight - 0.01f * dt);
            particle.velocity += gradient * dt * erosionProperties.gravity;
            particle.velocity *= erosionProperties.friction * (1.0f - dt);
            if(glm::length(particle.velocity) * dt > 1)
            {
                particle.velocity = glm::normalize(particle.velocity) / dt;
            }
            particle.position += particle.velocity * dt;
        }
        // float pixelHeight = heightMap->getPixelFloat(0, 0);
        // heightMap->setPixel(0, 0, pixelHeight - 0.1f * dt);
        heightMap->updateOnGPU();
    }
}