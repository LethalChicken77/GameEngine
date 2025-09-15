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

#include "containers.hpp"

namespace graphics
{
    class Mesh // TODO: Store triangle and vertex vectors, takes more memory but allows easier mesh manipulation
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

        // struct Builder
        // {
        //     std::vector<Vertex> vertices;
        //     std::vector<Triangle> triangles;

        //     void loadModelFromObj(const std::string& filename);
        // };

        Mesh();
        Mesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);
        Mesh(const std::vector<Vertex>& _vertices, const std::vector<Triangle>& _indices);
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        void drawInstanced(VkCommandBuffer commandBuffer, uint32_t instanceCount);

        static std::shared_ptr<Mesh> createCube(float edgeLength);
        static std::shared_ptr<Mesh> createSierpinskiPyramid(float edgeLength, int depth);
        static std::shared_ptr<Mesh> createGrid(int width, int length, glm::vec2 dimensions);
        static std::unique_ptr<Mesh> loadObj(const std::string& filename);

        void createBuffers();
        
        void generateNormals();
        void saveAsObj(const std::string& path);
        
        std::vector<Vertex> vertices{};
        std::vector<Triangle> triangles{};

    private:
        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount;
        bool useIndexBuffer = true;
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount;


        void createVertexBuffer();
        void createIndexBuffer();
        
        void loadModelFromObj(const std::string& filename);
    };
}