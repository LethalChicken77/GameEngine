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
#include "core/mesh.hpp"

namespace graphics
{
    class GraphicsMesh // TODO: Replace with graphics.draw(Mesh, Material)
    {
    public:
        static std::vector<VkVertexInputBindingDescription> getVertexBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();

        // struct Builder
        // {
        //     std::vector<Vertex> vertices;
        //     std::vector<Triangle> triangles;

        //     void loadModelFromObj(const std::string& filename);
        // };

        GraphicsMesh(core::MeshData* meshptr);
        ~GraphicsMesh();

        GraphicsMesh(const GraphicsMesh&) = delete;
        GraphicsMesh& operator=(const GraphicsMesh&) = delete;

        void bind(VkCommandBuffer commandBuffer); // TODO: Remove in favor of graphics.draw(Mesh)
        void draw(VkCommandBuffer commandBuffer);
        void drawInstanced(VkCommandBuffer commandBuffer, uint32_t instanceCount);

        void createBuffers();

    private:
        std::unique_ptr<Buffer> vertexBuffer{};
        uint32_t vertexCount;
        bool useIndexBuffer = true;
        std::unique_ptr<Buffer> indexBuffer{};
        uint32_t indexCount;

        core::MeshData* meshPtr;

        void createVertexBuffer();
        void createIndexBuffer();
        
        void loadModelFromObj(const std::string& filename);
    };
}