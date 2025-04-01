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
        ImGui::SliderInt("Max Lifetime", &erosionProperties.maxLifetime, 0, 1000);
        erosionProperties.maxLifetime = glm::max(erosionProperties.maxLifetime, 0);

        ImGui::SliderFloat("Erosion Rate", &erosionProperties.erosionRate, 0.0f, 0.1f, "%.9f");
        erosionProperties.erosionRate = glm::clamp(erosionProperties.erosionRate, 0.0f, 1.0f);
        ImGui::SliderFloat("Deposition Rate", &erosionProperties.depositionRate, 0.0f, 0.1f, "%.9f");
        erosionProperties.depositionRate = glm::clamp(erosionProperties.depositionRate, 0.0f, 1.0f);
        ImGui::SliderFloat("Sediment Capacity", &erosionProperties.sedimentCapacity, 0.0f, 5.f, "%.9f");
        erosionProperties.sedimentCapacity = glm::max(erosionProperties.sedimentCapacity, 0.0f);
        
        ImGui::SliderFloat("Gravity", &erosionProperties.gravity, -10.0f, 0.0f);
        erosionProperties.gravity = glm::min(erosionProperties.gravity, 0.0f);
        ImGui::SliderFloat("Friction", &erosionProperties.friction, 0.0f, 1.0f);
        erosionProperties.friction = glm::clamp(erosionProperties.friction, 0.0f, 1.0f);
        ImGui::SliderFloat("Delta Time", &erosionProperties.deltaTime, 0.0f, 0.1f);
        erosionProperties.deltaTime = glm::max(erosionProperties.deltaTime, 0.0f);
    }

    void HydraulicErosion::initializeParticles()
    {
        // cpuParticles.resize(erosionProperties.numParticles);
        // for(uint32_t i = 0; i < erosionProperties.numParticles; i++)
        // {
        //     cpuParticles[i].position = glm::vec2(core::Random::getRandom01(), core::Random::getRandom01()) * (float)heightMap->getWidth();
        //     cpuParticles[i].velocity = glm::vec2(0.0f, 0.0f);
        //     cpuParticles[i].sediment = 0.0f;
        // }
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
    
    void HydraulicErosion::runIterationsCPU(uint32_t iterations)
    {
        float ds = 0.0001f;
        float dt = erosionProperties.deltaTime;
        for(uint32_t i = 0; i < iterations; i++)
        {
            Particle particle{};
            particle.position = glm::vec2(core::Random::getRandom01(), core::Random::getRandom01()) * (float)heightMap->getWidth();
            particle.velocity = glm::vec2(0.0f, 0.0f);
            particle.sediment = 0.0f;
            for(int i = 0; i < erosionProperties.maxLifetime; i++)
            {
                particle.position = glm::clamp(particle.position, glm::vec2(0.0f), glm::vec2(heightMap->getWidth() - 1.0f));
                
                float xFrac = particle.position.x - (uint32_t)particle.position.x;
                float yFrac = particle.position.y - (uint32_t)particle.position.y;
                
                glm::vec2 texturePos = particle.position / (float)heightMap->getWidth();
                float height = heightMap->sampleBilinear(texturePos.x, texturePos.y);
                float height00 = heightMap->getPixelFloat((uint32_t)particle.position.x, (uint32_t)particle.position.y);
                float height10 = heightMap->getPixelFloat((uint32_t)particle.position.x + 1, (uint32_t)particle.position.y);
                float height01 = heightMap->getPixelFloat((uint32_t)particle.position.x, (uint32_t)particle.position.y + 1);
                float height11 = heightMap->getPixelFloat((uint32_t)particle.position.x + 1, (uint32_t)particle.position.y + 1);

                float dx = (height10 - height00) * (1.0f - yFrac) + (height11 - height01) * yFrac;
                float dy = (height01 - height00) * (1.0f - xFrac) + (height11 - height10) * xFrac;

                glm::vec2 gradient = glm::vec2(dx, dy);
                float lowestNeighbor = std::min(std::min(height00, height10), std::min(height01, height11));

                // glm::vec2 texturePos = particle.position / (float)heightMap->getWidth();
                // float iheight = heightMap->getPixelFloat((uint32_t)particle.position.x, (uint32_t)particle.position.y);
                // float iheightX = heightMap->getPixelFloat((uint32_t)particle.position.x + 1, (uint32_t)particle.position.y);
                // float iheightY = heightMap->getPixelFloat((uint32_t)particle.position.x, (uint32_t)particle.position.y + 1);
                // float iheightNX = heightMap->getPixelFloat((uint32_t)particle.position.x - 1, (uint32_t)particle.position.y);
                // float iheightNY = heightMap->getPixelFloat((uint32_t)particle.position.x, (uint32_t)particle.position.y - 1);
                // glm::vec2 gradient = glm::vec2(heightX - heightNX, heightY - heightNY) / ds;
                
                // float lowestNeighbor = std::min(std::min(heightX, heightY), std::min(iheightNX, iheightNY));
                float speed = glm::length(particle.velocity);

                float newHeight = 0;
                if(i == erosionProperties.maxLifetime - 1)
                {
                    newHeight += particle.sediment;
                }
                else
                {
                    float effectiveCapacity = erosionProperties.sedimentCapacity * speed * dt;
                    if(particle.sediment > effectiveCapacity || glm::dot(particle.velocity, gradient) > 0.0f)
                    {
                        newHeight += erosionProperties.depositionRate * (particle.sediment - effectiveCapacity);
                        particle.sediment -= erosionProperties.depositionRate * (particle.sediment - effectiveCapacity);
                    }
                    else if(particle.sediment < effectiveCapacity)
                    {
                        float erosionAmount = erosionProperties.erosionRate * (effectiveCapacity - particle.sediment);
                        newHeight -= erosionAmount;
                        particle.sediment += erosionAmount;
                    }
                    particle.sediment = glm::max(particle.sediment, 0.0f);
                }
                
                // particle.sediment += newHeight - height;
                
                float w00 = (1.0f - xFrac) * (1.0f - yFrac);
                float w10 = xFrac * (1.0f - yFrac);
                float w01 = (1.0f - xFrac) * yFrac;
                float w11 = xFrac * yFrac;

                float newHeight00 = glm::max(height00 + newHeight * w00, lowestNeighbor);
                float newHeight10 = glm::max(height10 + newHeight * w10, lowestNeighbor);
                float newHeight01 = glm::max(height01 + newHeight * w01, lowestNeighbor);
                float newHeight11 = glm::max(height11 + newHeight * w11, lowestNeighbor);
                
                heightMap->setPixel((uint32_t)particle.position.x, (uint32_t)particle.position.y, newHeight00);
                heightMap->setPixel((uint32_t)particle.position.x + 1, (uint32_t)particle.position.y, newHeight10);
                heightMap->setPixel((uint32_t)particle.position.x, (uint32_t)particle.position.y + 1, newHeight01);
                heightMap->setPixel((uint32_t)particle.position.x + 1, (uint32_t)particle.position.y + 1, newHeight11);

                // heightMap->setPixel(0, 0, pixelHeight - 0.01f * dt);
                particle.velocity += gradient * dt * erosionProperties.gravity;
                particle.velocity *= (1.0f - erosionProperties.friction * dt);
                if(glm::length(particle.velocity) * dt > 1)
                {
                    particle.velocity = glm::normalize(particle.velocity) / dt;
                }
                particle.position += particle.velocity;
            }
        }
        // float pixelHeight = heightMap->getPixelFloat(0, 0);
        // heightMap->setPixel(0, 0, pixelHeight - 0.1f * dt);
        heightMap->updateOnGPU();
    }
}