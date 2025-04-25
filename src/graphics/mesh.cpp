#include "mesh.hpp"

#include <cassert>
#include <cstring>
#define TINYOBJLOADER_IMPLEMENTATION

#include <tiny_obj_loader.h>

namespace graphics
{
    // Mesh::Mesh(Device& _device, const Builder& builder) : device(_device)
    // {
    //     createVertexBuffer(builder.vertices);
    //     createIndexBuffer(builder.triangles);
    // }
    
    Mesh::Mesh()
    {
        // createBuffers();
    }

    Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices)
    {
        vertices = _vertices;
        for(uint32_t i = 0; i < _indices.size(); i += 3)
        {
            triangles.push_back({_indices[i], _indices[i + 1], _indices[i + 2]});
        }
        // createVertexBuffer();
        // if(useIndexBuffer)
        //     createIndexBuffer();
    }

    Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<Triangle>& _triangles)
    {
        vertices = _vertices;
        triangles = _triangles;
        // createVertexBuffer();
        // if(useIndexBuffer)
        //     createIndexBuffer();
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

    void Mesh::createVertexBuffer()
    {
        vertexCount = static_cast<uint32_t>(vertices.size());
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

    void Mesh::createIndexBuffer()
    {
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

    void Mesh::createBuffers()
    {
        createVertexBuffer();
        createIndexBuffer();
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

    float angleBetween(glm::vec3 v1, glm::vec3 v2)
    {
        return glm::acos(glm::dot(v1, v2) / (glm::length(v1) * glm::length(v2)));
    }

    void Mesh::generateNormals()
    {
        if(triangles.size() == 0)
        {
            return;
        }

        for(Vertex& vertex : vertices)
        {
            vertex.normal = glm::vec3(0.0f);
        }
        for(Triangle& triangle : triangles)
        {
            Vertex& v0 = vertices[triangle.v0];
            Vertex& v1 = vertices[triangle.v1];
            Vertex& v2 = vertices[triangle.v2];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;
            glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
            float angle0 = angleBetween(v1.position - v0.position, v2.position - v0.position);
            float angle1 = angleBetween(v0.position - v1.position, v2.position - v1.position);
            float angle2 = angleBetween(v0.position - v2.position, v1.position - v2.position);

            v0.normal += normal * angle0;
            v1.normal += normal * angle1;
            v2.normal += normal * angle2;
        }
        for(Vertex& vertex : vertices)
        {
            vertex.normal = glm::normalize(vertex.normal);
        }
    }
    
    std::shared_ptr<Mesh> Mesh::createCube(float edgeLength)
    {
        edgeLength *= 0.5f;
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

        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(vertices, indices);
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

    std::shared_ptr<Mesh> Mesh::createSierpinskiPyramid(float edgeLength, int depth)
    {
        std::vector<Vertex> vertices = generateSierpinski(edgeLength, depth);
        
        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(vertices, std::vector<uint32_t>());
        return mesh;
    }

    std::shared_ptr<Mesh> Mesh::createGrid(int width, int length, glm::vec2 dimensions)
    {
        width = std::max(1, width);
        length = std::max(1, length);
        std::vector<Vertex> vertices{};
        std::vector<Triangle> triangles{};
        float xScale = dimensions.x / (float)width;
        float zScale = dimensions.y / (float)length;
        float minX = -dimensions.x / 2.0f;
        float minZ = -dimensions.y / 2.0f;

        for(int x = 0; x <= width; x++)
        {
            for(int z = 0; z <= length; z++)
            {
                float x0 = minX + x * xScale;
                float z0 = minZ + z * zScale;
                Vertex v = {
                    // {x0, glm::sin(x0) + glm::sin(z0), z0},
                    {x0, 0, z0},
                    {0, 1, 0},
                    {1, 1, 1},
                    {x / (float)width, z / (float)length}
                };
                vertices.push_back(v);

                if(x > 0 && z > 0)
                {
                    uint32_t i0 = (x - 1) * (length + 1) + z - 1;
                    uint32_t i1 = (x - 1) * (length + 1) + z;
                    uint32_t i2 = x * (length + 1) + z;
                    uint32_t i3 = x * (length + 1) + z - 1;
                    triangles.push_back({i0, i1, i2});
                    triangles.push_back({i0, i2, i3});
                }
            }
        }
        
        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(vertices, triangles);
        mesh->generateNormals();
        mesh->createBuffers();
        return mesh;
    }

    std::unique_ptr<Mesh> Mesh::loadObj(const std::string& filename)
    {
        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
        mesh->loadModelFromObj(filename);

        return mesh;
    }

    void Mesh::loadModelFromObj(const std::string& filename)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        triangles.clear();

        // Define a key to identify unique vertex combinations
        struct VertexKey {
            int posIdx;
            int normIdx;
            int texIdx;
            bool operator==(const VertexKey& other) const {
                return posIdx == other.posIdx && normIdx == other.normIdx && texIdx == other.texIdx;
            }
        };

        // Hash function for VertexKey
        struct VertexKeyHash {
            size_t operator()(const VertexKey& k) const {
                size_t h1 = std::hash<int>{}(k.posIdx);
                size_t h2 = std::hash<int>{}(k.normIdx);
                size_t h3 = std::hash<int>{}(k.texIdx);
                return h1 ^ (h2 << 1) ^ (h3 << 2);
            }
        };

        // Map from VertexKey to index in vertices vector
        std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertexMap;

        // Process each shape
        for (const auto& shape : shapes) {
            // Iterate over indices in steps of 3 (triangles)
            for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
                std::array<uint32_t, 3> triIndices;

                // Process each vertex of the triangle
                for (int k = 0; k < 3; k++) {
                    tinyobj::index_t idx = shape.mesh.indices[i + k];
                    VertexKey key{idx.vertex_index, idx.normal_index, idx.texcoord_index};

                    // Check if this vertex combination exists
                    auto it = vertexMap.find(key);
                    if (it != vertexMap.end()) {
                        triIndices[k] = it->second;
                    } else {
                        // Create a new Vertex
                        Vertex v;
                        int posIdx = idx.vertex_index;
                        v.position = {
                            attrib.vertices[3 * posIdx],
                            attrib.vertices[3 * posIdx + 1],
                            attrib.vertices[3 * posIdx + 2]
                        };

                        int normIdx = idx.normal_index;
                        if (normIdx >= 0) {
                            v.normal = {
                                attrib.normals[3 * normIdx],
                                attrib.normals[3 * normIdx + 1],
                                attrib.normals[3 * normIdx + 2]
                            };
                        }

                        int texIdx = idx.texcoord_index;
                        if (texIdx >= 0) {
                            v.texCoord = {
                                attrib.texcoords[2 * texIdx],
                                attrib.texcoords[2 * texIdx + 1]
                            };
                        }

                        if (3 * posIdx + 2 < attrib.colors.size()) {
                            v.color = {
                                attrib.colors[3 * posIdx],
                                attrib.colors[3 * posIdx + 1],
                                attrib.colors[3 * posIdx + 2]
                            };
                        }

                        // Add to vertices and update map
                        vertices.push_back(v);
                        uint32_t newIndex = static_cast<uint32_t>(vertices.size() - 1);
                        vertexMap[key] = newIndex;
                        triIndices[k] = newIndex;
                    }
                }

                // Add the triangle
                triangles.push_back({triIndices[0], triIndices[1], triIndices[2]});
            }
        }

        std::cout << "Loaded " << vertices.size() << " vertices and " << triangles.size() << " triangles\n";

        createBuffers();
    }
} // namespace graphics
