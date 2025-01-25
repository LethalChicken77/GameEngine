#include "mesh.hpp"

#include <cassert>
#include <cstring>
#define TINYOBJLOADER_IMPLEMENTATION

#include <tiny_obj_loader.h>

namespace graphics
{
    Mesh::Mesh(Device& _device, const Builder& builder) : device(_device)
    {
        createVertexBuffer(builder.vertices);
        createIndexBuffer(builder.indices);
    }
    
    Mesh::Mesh(Device& _device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) : device(_device)
    {
        createVertexBuffer(vertices);
        if(useIndexBuffer)
            createIndexBuffer(indices);
    }

    Mesh::~Mesh(){}

    void Mesh::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if(useIndexBuffer)
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    void Mesh::draw(VkCommandBuffer commandBuffer)
    {
        if(useIndexBuffer)
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        else
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

    void Mesh::drawInstanced(VkCommandBuffer commandBuffer, uint32_t instanceCount)
    {
        // TODO: Implement
        if(useIndexBuffer)
            vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
        else
            vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
    }

    void Mesh::createVertexBuffer(const std::vector<Vertex>& vertices)
    {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        Buffer stagingBuffer{
            device,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        vertexBuffer = std::make_unique<Buffer>(
            device,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void Mesh::createIndexBuffer(const std::vector<uint32_t>& indices)
    {
        indexCount = static_cast<uint32_t>(indices.size());
        useIndexBuffer = indexCount > 0;

        if(!useIndexBuffer)
            return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        
        uint32_t indexSize = sizeof(indices[0]);

        Buffer stagingBuffer{
            device,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)indices.data());

        indexBuffer = std::make_unique<Buffer>(
            device,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    std::vector<VkVertexInputBindingDescription> Mesh::Vertex::getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Mesh::Vertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);
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
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, color);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    
    std::shared_ptr<Mesh> Mesh::createCube(Device& device, float edgeLength)
    {
        edgeLength *= 0.5f;

        // std::vector<Mesh::Vertex> vertices {
        //     // Front and back
        //     {{-edgeLength, -edgeLength, -edgeLength}, {0.0f, 0.0f}},
        //     {{-edgeLength, edgeLength, -edgeLength}, {0.0f, 1.0f}},
        //     {{edgeLength, -edgeLength, -edgeLength}, {1.0f, 0.0f}},
            
        //     {{edgeLength, edgeLength, -edgeLength}, {1.0f, 1.0f}},
        //     {{edgeLength, -edgeLength, -edgeLength}, {1.0f, 0.0f}},
        //     {{-edgeLength, edgeLength, -edgeLength}, {0.0f, 1.0f}},

            
        //     {{edgeLength, -edgeLength, edgeLength}, {0.0f, 0.0f}},
        //     {{edgeLength, edgeLength, edgeLength}, {0.0f, 1.0f}},
        //     {{-edgeLength, -edgeLength, edgeLength}, {1.0f, 0.0f}},
            
        //     {{-edgeLength, edgeLength, edgeLength}, {1.0f, 1.0f}},
        //     {{-edgeLength, -edgeLength, edgeLength}, {1.0f, 0.0f}},
        //     {{edgeLength, edgeLength, edgeLength}, {0.0f, 1.0f}},


        //     // Left and right
        //     {{-edgeLength, -edgeLength, -edgeLength}, {1.0f, 0.0f}},
        //     {{-edgeLength, -edgeLength, edgeLength}, {0.0f, 0.0f}},
        //     {{-edgeLength, edgeLength, -edgeLength}, {1.0f, 1.0f}},
            
        //     {{-edgeLength, edgeLength, edgeLength}, {0.0f, 1.0f}},
        //     {{-edgeLength, edgeLength, -edgeLength}, {1.0f, 1.0f}},
        //     {{-edgeLength, -edgeLength, edgeLength}, {0.0f, 0.0f}},

            
        //     {{edgeLength, -edgeLength, edgeLength}, {1.0f, 0.0f}},
        //     {{edgeLength, -edgeLength, -edgeLength}, {0.0f, 0.0f}},
        //     {{edgeLength, edgeLength, edgeLength}, {1.0f, 1.0f}},
            
        //     {{edgeLength, edgeLength, -edgeLength}, {0.0f, 1.0f}},
        //     {{edgeLength, edgeLength, edgeLength}, {1.0f, 1.0f}},
        //     {{edgeLength, -edgeLength, -edgeLength}, {0.0f, 0.0f}},




        //     // Top and bottom
        //     {{-edgeLength, edgeLength, edgeLength}, {0.0f, 1.0f}},
        //     {{edgeLength, edgeLength, edgeLength}, {1.0f, 1.0f}},
        //     {{-edgeLength, edgeLength, -edgeLength}, {0.0f, 0.0f}},
            
        //     {{edgeLength, edgeLength, -edgeLength}, {1.0f, 0.0f}},
        //     {{-edgeLength, edgeLength, -edgeLength}, {0.0f, 0.0f}},
        //     {{edgeLength, edgeLength, edgeLength}, {1.0f, 1.0f}},

            
        //     {{-edgeLength, -edgeLength, edgeLength}, {1.0f, 1.0f}},
        //     {{-edgeLength, -edgeLength, -edgeLength}, {1.0f, 0.0f}},
        //     {{edgeLength, -edgeLength, edgeLength}, {0.0f, 1.0f}},
            
        //     {{edgeLength, -edgeLength, -edgeLength}, {0.0f, 0.0f}},
        //     {{edgeLength, -edgeLength, edgeLength}, {0.0f, 1.0f}},
        //     {{-edgeLength, -edgeLength, -edgeLength}, {1.0f, 0.0f}},
        // };
        // std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(device, vertices, std::vector<uint32_t>());

        float inverseSqrt3 = 1.0f / glm::sqrt(3.0f);

        std::vector<Mesh::Vertex> vertices {
            {{-1, -1, -1}, {-inverseSqrt3, -inverseSqrt3, -inverseSqrt3}, {1.f, 1.f, 1.f}, {0.0f, 0.0f}},
            {{1, -1, -1}, {inverseSqrt3, -inverseSqrt3, -inverseSqrt3}, {1.f, 1.f, 1.f}, {1.0f, 0.0f}},
            {{-1, 1, -1}, {-inverseSqrt3, inverseSqrt3, -inverseSqrt3}, {1.f, 1.f, 1.f}, {0.0f, 0.0f}},
            {{1, 1, -1}, {inverseSqrt3, inverseSqrt3, -inverseSqrt3}, {1.f, 1.f, 1.f}, {1.0f, 0.0f}},
            {{-1, -1, 1}, {-inverseSqrt3, -inverseSqrt3, inverseSqrt3}, {1.f, 1.f, 1.f}, {0.0f, 1.0f}},
            {{1, -1, 1}, {inverseSqrt3, -inverseSqrt3, inverseSqrt3}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}},
            {{-1, 1, 1}, {-inverseSqrt3, inverseSqrt3, inverseSqrt3}, {1.f, 1.f, 1.f}, {0.0f, 1.0f}},
            {{1, 1, 1}, {inverseSqrt3, inverseSqrt3, inverseSqrt3}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}},
        };

        std::vector<uint32_t> indices{
            0, 2, 1, 
            2, 3, 1,
            4, 5, 6, 
            6, 5, 7,
            0, 4, 2, 
            4, 6, 2,
            1, 3, 5, 
            3, 7, 5,
            0, 1, 4, 
            1, 5, 4,
            2, 6, 3, 
            6, 7, 3,
        };

        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(device, vertices, indices);
        return mesh;
    }

    std::vector<Mesh::Vertex> generateSierpinski(float edgeLength, int depth)
    {
        float root2 = glm::sqrt(2.0f);
        float root3 = glm::sqrt(3.0f);
        glm::vec3 v0 = glm::vec3(0,         0,      root3 / 3.0f) * edgeLength;
        glm::vec3 v1 = glm::vec3(0.5f,      0,      -root3 / 6.0f) * edgeLength;
        glm::vec3 v2 = glm::vec3(-0.5f,     0,      -root3 / 6.0f) * edgeLength;
        glm::vec3 v3 = glm::vec3(0.0f, root3 * 0.5f, 0.0f) * edgeLength;
        if(depth <= 0)
        {
            std::vector<Mesh::Vertex> vertices(12);
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
            
            for(Mesh::Vertex& vertex : vertices)
            {
                vertex.position = vertex.position * 1.5f;
            }

            return vertices;
        }
        else
        {
            std::vector<Mesh::Vertex> vertices0 = generateSierpinski(edgeLength, depth - 1);
            std::vector<Mesh::Vertex> vertices1 = vertices0;
            std::vector<Mesh::Vertex> vertices2 = vertices0;
            std::vector<Mesh::Vertex> vertices3 = vertices0;

            for(Mesh::Vertex& vertex : vertices0)
            {
                vertex.position = (vertex.position + v0) * 0.5f;
            }
            for(Mesh::Vertex& vertex : vertices1)
            {
                vertex.position = (vertex.position + v1) * 0.5f;
            }
            for(Mesh::Vertex& vertex : vertices2)
            {
                vertex.position = (vertex.position + v2) * 0.5f;
            }
            for(Mesh::Vertex& vertex : vertices3)
            {
                vertex.position = (vertex.position + v3) * 0.5f;
            }


            std::vector<Mesh::Vertex> vertices{};
            vertices.insert(vertices.end(), vertices0.begin(), vertices0.end());
            vertices.insert(vertices.end(), vertices1.begin(), vertices1.end());
            vertices.insert(vertices.end(), vertices2.begin(), vertices2.end());
            vertices.insert(vertices.end(), vertices3.begin(), vertices3.end());
            return vertices;
        }
    }

    std::shared_ptr<Mesh> Mesh::createSierpinskiPyramid(Device& device, float edgeLength, int depth)
    {
        std::vector<Vertex> vertices = generateSierpinski(edgeLength, depth);
        
        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(device, vertices, std::vector<uint32_t>());
        return mesh;
    }

    std::unique_ptr<Mesh> Mesh::loadObj(Device& device, const std::string& filename)
    {
        Builder builder{};
        builder.loadModelFromObj(filename);

        std::cout << "Vertex Count: " << builder.vertices.size() << std::endl;

        return std::make_unique<Mesh>(device, builder);
    }

    void Mesh::Builder::loadModelFromObj(const std::string& filename)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        for(const tinyobj::shape_t &shape : shapes)
        {
            for(const tinyobj::index_t &index : shape.mesh.indices)
            {
                Vertex vertex{};

                if(index.vertex_index >= 0)
                {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    uint32_t colorIndex = 3 * index.vertex_index + 3;
                    if(colorIndex < attrib.colors.size())
                    {
                        vertex.color = {
                            attrib.colors[colorIndex + 0],
                            attrib.colors[colorIndex + 1],
                            attrib.colors[colorIndex + 2]
                        };
                    }
                    else
                    {
                        vertex.color = {1.0f, 1.0f, 1.0f};
                    }
                }
                if(index.normal_index >= 0)
                {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }
                if(index.texcoord_index >= 0)
                {
                    vertex.texCoord = {
                        attrib.texcoords[3 * index.texcoord_index + 0],
                        attrib.texcoords[3 * index.texcoord_index + 1]
                    };
                }
                vertices.push_back(vertex);
            }
        }
    }
}