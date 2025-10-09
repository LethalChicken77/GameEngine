#include "graphics_mesh.hpp"

#include <cassert>
#include <cstring>

using Vertex = core::MeshData::Vertex;
using Triangle = core::MeshData::Triangle;

namespace graphics
{
// GraphicsMesh::GraphicsMesh(Device& _device, const Builder& builder) : device(_device)
// {
//     createVertexBuffer(builder.vertices);
//     createIndexBuffer(builder.triangles);
// }

GraphicsMesh::GraphicsMesh(core::MeshData* mesh) : meshPtr(mesh)
{
    vertexCount = mesh->vertices.size();
    indexCount = mesh->triangles.size() * 3;
    createBuffers();
}

GraphicsMesh::~GraphicsMesh(){}

void GraphicsMesh::bind(VkCommandBuffer commandBuffer, const std::unique_ptr<Buffer> &instanceBuffer)
{
    VkBuffer buffers[] = {vertexBuffer->getBuffer(), instanceBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0, 0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);
    if(useIndexBuffer)
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void GraphicsMesh::draw(VkCommandBuffer commandBuffer, uint32_t instanceCount)
{
    if(useIndexBuffer)
        vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
    else
        vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
}

void GraphicsMesh::createVertexBuffer()
{
    if(meshPtr == nullptr)
    {
        Console::error("Cannot create vertex buffer as mesh pointer is null", "GraphicsMesh");
        return;
    }
    
    std::vector<Vertex> &vertices = meshPtr->vertices; // Reference to triangles array for easy access

    assert(vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    uint32_t vertexSize = sizeof(vertices[0]);

    Buffer stagingBuffer{
        *Shared::device,
        vertexSize,
        vertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)vertices.data());

    vertexBuffer = std::make_unique<Buffer>(
        *Shared::device,
        vertexSize,
        vertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    Shared::device->copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
}

void GraphicsMesh::createIndexBuffer()
{
    if(meshPtr == nullptr)
    {
        Console::error("Cannot create index buffer as mesh pointer is null", "GraphicsMesh");
        return;
    }
    
    std::vector<Triangle> &triangles = meshPtr->triangles; // Reference to triangles array for easy access

    indexCount = static_cast<uint32_t>(triangles.size()) * 3;
    useIndexBuffer = indexCount > 0;

    if(!useIndexBuffer)
        return;

    VkDeviceSize bufferSize = sizeof(triangles[0].v0) * indexCount;
    
    uint32_t indexSize = sizeof(triangles[0].v0);

    Buffer stagingBuffer{
        *Shared::device,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)triangles.data());

    indexBuffer = std::make_unique<Buffer>(
        *Shared::device,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    Shared::device->copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}

void GraphicsMesh::createBuffers()
{
    createVertexBuffer();
    createIndexBuffer();
}

std::vector<VkVertexInputBindingDescription> GraphicsMesh::getVertexBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(2);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Instance data
    bindingDescriptions[1].binding = 1;
    bindingDescriptions[1].stride = sizeof(glm::mat4);
    bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> GraphicsMesh::getVertexAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(10);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, tangent);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, bitangent);

    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex, color);

    attributeDescriptions[5].binding = 0;
    attributeDescriptions[5].location = 5;
    attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[5].offset = offsetof(Vertex, texCoord);

    // Instance attributes
    for(int i = 0; i < 4; i++)
    {
        attributeDescriptions[i + 6].binding = 1;
        attributeDescriptions[i + 6].location = i + 6;
        attributeDescriptions[i + 6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[i + 6].offset = sizeof(glm::vec4) * i;
    }

    return attributeDescriptions;
}

std::vector<Vertex> generateSierpinski(float edgeLength, int depth)
{
    float root2 = glm::sqrt(2.0f);
    float root3 = glm::sqrt(3.0f);
    glm::vec3 v0 = glm::vec3(0,         0,      root3 / 3.0f) * edgeLength;
    glm::vec3 v1 = glm::vec3(0.5f,      0,      -root3 / 6.0f) * edgeLength;
    glm::vec3 v2 = glm::vec3(-0.5f,     0,      -root3 / 6.0f) * edgeLength;
    glm::vec3 v3 = glm::vec3(0.0f, root3 * 0.5f, 0.0f) * edgeLength;
    if(depth <= 0)
    {
        std::vector<Vertex> vertices(12);
        vertices[0].position = v0;
        vertices[1].position = v2;
        vertices[2].position = v1;
        vertices[0].normal = vertices[1].normal = vertices[2].normal 
            = glm::normalize(-glm::cross(v1 - v0, v2 - v0));

        vertices[3].position = v0;
        vertices[4].position = v1;
        vertices[5].position = v3;
        vertices[3].normal = vertices[4].normal = vertices[5].normal 
            = glm::normalize(glm::cross(v1 - v0, v3 - v0));

        vertices[6].position = v0;
        vertices[7].position = v3;
        vertices[8].position = v2;
        vertices[6].normal = vertices[7].normal = vertices[8].normal 
            = glm::normalize(glm::cross(v3 - v0, v2 - v0));

        vertices[9].position = v3;
        vertices[10].position = v1;
        vertices[11].position = v2;
        vertices[9].normal = vertices[10].normal = vertices[11].normal 
            = glm::normalize(glm::cross(v1 - v3, v2 - v3));
        
        for(Vertex& vertex : vertices)
        {
            vertex.position = vertex.position * 1.5f;
        }

        return vertices;
    }
    else
    {
        std::vector<Vertex> vertices0 = generateSierpinski(edgeLength, depth - 1);
        std::vector<Vertex> vertices1 = vertices0;
        std::vector<Vertex> vertices2 = vertices0;
        std::vector<Vertex> vertices3 = vertices0;

        for(Vertex& vertex : vertices0)
        {
            vertex.position = (vertex.position + v0) * 0.5f;
        }
        for(Vertex& vertex : vertices1)
        {
            vertex.position = (vertex.position + v1) * 0.5f;
        }
        for(Vertex& vertex : vertices2)
        {
            vertex.position = (vertex.position + v2) * 0.5f;
        }
        for(Vertex& vertex : vertices3)
        {
            vertex.position = (vertex.position + v3) * 0.5f;
        }


        std::vector<Vertex> vertices{};
        vertices.insert(vertices.end(), vertices0.begin(), vertices0.end());
        vertices.insert(vertices.end(), vertices1.begin(), vertices1.end());
        vertices.insert(vertices.end(), vertices2.begin(), vertices2.end());
        vertices.insert(vertices.end(), vertices3.begin(), vertices3.end());
        return vertices;
    }
}
} // namespace graphics
