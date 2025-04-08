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
        // particleBuffer = std::make_shared<Buffer>(sizeof(Particle) * numParticles, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        
        
        // computeShader = std::make_unique<ComputeShader>("shaders/hydraulic_erosion.comp", std::vector<ShaderInput>{
        //     // {"color", ShaderInput::DataType::VEC3},
        // });
        computeShader = std::make_unique<ComputeShader>("internal/compute_shaders/erosion.comp.spv", std::vector<ShaderInput>{}, 1);
        computePipeline = std::make_unique<graphics::ComputePipeline>(*computeShader);

        initializeTexture(resolution);

        computeResource = std::make_unique<ComputeResource>(ComputeResource::instantiate(computeShader.get()));
        computeResource->setTexture(0, heightMap);
        computeResource->setTexture(1, heightMap);
        // computeResource->setTextureImage(1, heightMap);
        computeResource->updateDescriptorSet();
        // heightMapResource = std::make_unique<ShaderResource>(heightMap, 0, 0, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            
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

        ImGui::SliderFloat("Sediment per Height", &erosionProperties.sedimentScale, 0.0f, 0.1f, "%.9f");
        erosionProperties.sedimentScale = glm::clamp(erosionProperties.sedimentScale, 0.0f, 1.0f);
        // ImGui::SliderFloat("Erosion Rate", &erosionProperties.erosionRate, 0.0f, 0.1f, "%.9f");
        // erosionProperties.erosionRate = glm::clamp(erosionProperties.erosionRate, 0.0f, 1.0f);
        // ImGui::SliderFloat("Deposition Rate", &erosionProperties.depositionRate, 0.0f, 0.1f, "%.9f");
        // erosionProperties.depositionRate = glm::clamp(erosionProperties.depositionRate, 0.0f, 1.0f);
        ImGui::SliderFloat("Sediment Capacity", &erosionProperties.sedimentCapacity, 0.0f, 5.f, "%.9f");
        erosionProperties.sedimentCapacity = glm::max(erosionProperties.sedimentCapacity, 0.0f);
        ImGui::SliderFloat("Base Capacity", &erosionProperties.baseCapacity, 0.0f, 1.f, "%.9f");
        erosionProperties.baseCapacity = glm::max(erosionProperties.baseCapacity, 0.0f);
        
        ImGui::SliderFloat("Gravity", &erosionProperties.gravity, -10.0f, 0.0f);
        erosionProperties.gravity = glm::min(erosionProperties.gravity, 0.0f);
        ImGui::SliderFloat("Friction", &erosionProperties.friction, 0.0f, 1.0f);
        erosionProperties.friction = glm::clamp(erosionProperties.friction, 0.0f, 1.0f);
        // ImGui::SliderFloat("Delta Time", &erosionProperties.deltaTime, 0.0f, 0.1f);
        // erosionProperties.deltaTime = glm::max(erosionProperties.deltaTime, 0.0f);
    }

    void HydraulicErosion::initializeTexture(size_t resolution)
    {
        std::cout << "Generating Heightmap" << std::endl;
        TextureProperties properties = TextureProperties().getDefaultProperties();
        SamplerProperties samplerProperties = SamplerProperties().getDefaultProperties();
        properties.format = VK_FORMAT_R32_SFLOAT;
        properties.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        properties.finalLayout = VK_IMAGE_LAYOUT_GENERAL; // Allow usage for reads and writes, no need for shader read
        samplerProperties.magFilter = VK_FILTER_LINEAR;
        samplerProperties.minFilter = VK_FILTER_LINEAR;
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
        for(uint32_t i = 0; i < iterations; i++)
        {
            Particle particle{};
            particle.position = glm::vec2(core::Random::getRandom01(), core::Random::getRandom01()) * (float)heightMap->getWidth();
            particle.velocity = glm::vec2(0.0f, 0.0f);
            particle.sediment = 0.0f;
            bool endErosion = false;
            for(int i = 0; i < erosionProperties.maxLifetime; i++)
            {
                if(particle.position.x < 0.0f || particle.position.y < 0.0f || particle.position.x >= heightMap->getWidth() || particle.position.y >= heightMap->getHeight())
                {
                    break;
                }
                
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

                float speed = glm::length(particle.velocity);

                float newHeight = 0;
                if(i == erosionProperties.maxLifetime - 1 || (speed < 0.001f && i > 5))
                {
                    newHeight += particle.sediment;
                    endErosion = true;
                }
                else
                {
                    float effectiveCapacity = erosionProperties.sedimentCapacity * (speed + erosionProperties.baseCapacity);
                    if(particle.sediment > effectiveCapacity || glm::dot(particle.velocity, gradient) > 0.0f)
                    {
                        float depositionAmount = particle.sediment - effectiveCapacity;
                        depositionAmount = glm::min(depositionAmount, particle.sediment);
                        newHeight += depositionAmount;
                    }
                    else if(particle.sediment < effectiveCapacity)
                    {
                        float erosionAmount = effectiveCapacity - particle.sediment;
                        erosionAmount = glm::min(erosionAmount, height - lowestNeighbor);
                        newHeight -= erosionAmount;
                    }
                }
                
                newHeight *= erosionProperties.sedimentScale;

                float w00 = (1.0f - xFrac) * (1.0f - yFrac);
                float w10 = xFrac * (1.0f - yFrac);
                float w01 = (1.0f - xFrac) * yFrac;
                float w11 = xFrac * yFrac;

                float newHeight00 = glm::max(height00 + newHeight * w00, lowestNeighbor);
                float newHeight10 = glm::max(height10 + newHeight * w10, lowestNeighbor);
                float newHeight01 = glm::max(height01 + newHeight * w01, lowestNeighbor);
                float newHeight11 = glm::max(height11 + newHeight * w11, lowestNeighbor);
                float actualChange = newHeight00 + newHeight10 + newHeight01 + newHeight11 
                                    - height00 - height10 - height01 - height11;

                particle.sediment -= actualChange / erosionProperties.sedimentScale;
                particle.sediment = glm::max(particle.sediment, 0.0f);

                if(particle.sediment < 0.001f)
                    break;
                
                heightMap->setPixel((uint32_t)particle.position.x, (uint32_t)particle.position.y, newHeight00);
                heightMap->setPixel((uint32_t)particle.position.x + 1, (uint32_t)particle.position.y, newHeight10);
                heightMap->setPixel((uint32_t)particle.position.x, (uint32_t)particle.position.y + 1, newHeight01);
                heightMap->setPixel((uint32_t)particle.position.x + 1, (uint32_t)particle.position.y + 1, newHeight11);

                particle.velocity *= (1.0f - erosionProperties.friction); // Apply friction before updating velocity
                particle.velocity += gradient * erosionProperties.gravity;
                if(glm::length(particle.velocity) > 1)
                {
                    particle.velocity = glm::normalize(particle.velocity);
                }
                particle.position += particle.velocity;
            }
        }
        heightMap->updateOnGPU();
    }

    void HydraulicErosion::runIterationsGPU(uint32_t iterations)
    {
        // heightMap->transitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
        // computeResource->updateDescriptorSet();
        
        VkCommandBuffer commandBuffer = graphics::Shared::device->beginSingleTimeCommands();

        computePipeline->bind(commandBuffer);

        std::vector<VkDescriptorSet> descriptorSets = { computeResource->getDescriptorSet() };
        vkCmdBindDescriptorSets(
            commandBuffer, 
            VK_PIPELINE_BIND_POINT_COMPUTE, 
            computePipeline->getPipelineLayout(), 
            0,
            static_cast<uint32_t>(descriptorSets.size()),
            descriptorSets.data(), 
            0,
            nullptr
        );

        graphics::ComputePipeline::ErosionPushConstants push{};
        push.numParticles = iterations;
        push.seed = core::Random::getRandomInt(INT32_MIN, INT32_MAX);
        push.maxLifetime = erosionProperties.maxLifetime;
        push.sedimentScale = erosionProperties.sedimentScale;
        push.sedimentCapacity = erosionProperties.sedimentCapacity;
        push.baseCapacity = erosionProperties.baseCapacity;
        push.gravity = erosionProperties.gravity;
        push.friction = erosionProperties.friction;
        vkCmdPushConstants(
            commandBuffer, 
            computePipeline->getPipelineLayout(),
            VK_SHADER_STAGE_COMPUTE_BIT, 
            0, 
            sizeof(graphics::ComputePipeline::ErosionPushConstants), 
            &push
        );
        
        uint32_t groupsX = (iterations + 1024 - 1) / 1024;
        
        // Run the compute shader for the specified number of iterations
        computePipeline->dispatch(commandBuffer, groupsX, 1, 1);
        
        graphics::Shared::device->endSingleTimeCommands(commandBuffer);

        // heightMap->transitionImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}