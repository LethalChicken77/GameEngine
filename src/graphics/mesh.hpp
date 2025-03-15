#pragma once

#include "device.hpp"
#include "buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

namespace graphics
{
    class Mesh
    {
    public:
    
        struct Vertex
        {
            glm::vec3 position{};
            glm::vec3 normal{};
            glm::vec3 color{1.0f, 1.0f, 1.0f};
            glm::vec2 texCoord{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        struct Triangle
        {
            uint32_t v0;
            uint32_t v1;
            uint32_t v2;
        };

        struct Builder
        {
            std::vector<Vertex> vertices;
            std::vector<Triangle> triangles;

            void loadModelFromObj(const std::string& filename);
        };

        Mesh(Device& _device, const Builder& builder);
        Mesh(Device& _device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        Mesh(Device& _device, const std::vector<Vertex>& vertices, const std::vector<Triangle>& indices);
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        void drawInstanced(VkCommandBuffer commandBuffer, uint32_t instanceCount);

        static std::shared_ptr<Mesh> createCube(Device& device, float edgeLength);
        static std::shared_ptr<Mesh> createSierpinskiPyramid(Device& device, float edgeLength, int depth);
        static std::shared_ptr<Mesh> createGrid(Device& device, glm::ivec2 dimensions);
        static std::shared_ptr<Mesh> createGrid(Device& device, int width, int length);
        static std::unique_ptr<Mesh> loadObj(Device& device, const std::string& filename);

    private:
        Device& device;
        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount;
        bool useIndexBuffer = true;
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount;

        void createVertexBuffer(const std::vector<Vertex>& vertices);
        void createIndexBuffer(const std::vector<uint32_t>& indices);
        void createIndexBuffer(const std::vector<Triangle>& triangles);
    };
}