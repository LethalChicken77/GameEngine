#include "mesh.hpp"
#include "graphics/graphics.hpp"

#include <cassert>
#include <cstring>
#define TINYOBJLOADER_IMPLEMENTATION

#include <tiny_obj_loader.h>
#include <unordered_map>
#include <array>

#include "modules.hpp"

namespace core
{

void MeshData::SetMesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices)
{
    vertices = _vertices;
    for(uint32_t i = 0; i < _indices.size(); i += 3)
    {
        triangles.push_back({_indices[i], _indices[i + 1], _indices[i + 2]});
    }
}

void MeshData::SetMesh(const std::vector<Vertex>& _vertices, const std::vector<Triangle>& _triangles)
{
    vertices = _vertices;
    triangles = _triangles;
    // createVertexBuffer();
    // if(useIndexBuffer)
    //     createIndexBuffer();
}

MeshData::~MeshData(){}

float angleBetween(glm::vec3 v1, glm::vec3 v2)
{
    return glm::acos(glm::clamp(glm::dot(v1, v2) / (glm::length(v1) * glm::length(v2)), -1.0f + 1e-6f, 1.0f - 1e-6f));
}

Mesh::Mesh(std::vector<Vertex> &vertices, const std::string& objectName)
{
    ptr = ObjectManager::Instantiate<MeshData>(objectName);
    ptr->SetMesh(vertices, std::vector<uint32_t>{});
    graphicsModule.setGraphicsMesh(*this);
}

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, const std::string& objectName)
{
    ptr = ObjectManager::Instantiate<MeshData>(objectName);
    ptr->SetMesh(vertices, indices);
    graphicsModule.setGraphicsMesh(*this);
}

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<Triangle> &triangles, const std::string& objectName)
{
    ptr = ObjectManager::Instantiate<MeshData>(objectName);
    ptr->SetMesh(vertices, triangles);
    graphicsModule.setGraphicsMesh(*this);
}

void Mesh::generateNormals()
{
    if(!ptr)
    {
        Console::error("Cannot generate normals as mesh pointer is null", "Mesh");
    }

    std::vector<Vertex> &vertices = ptr->vertices;
    std::vector<Triangle> &triangles = ptr->triangles;
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
        
        // if(glm::length(edge1) < 1e-6f || glm::length(edge2) < 1e-6f)
        //     continue; // Degenerate triangle, just like me :(
            
        glm::vec3 cross = glm::cross(edge1, edge2);

        if(glm::length(cross) < 1e-6f)
            continue; // Degenerate triangle

        glm::vec3 normal = glm::normalize(cross);

        float angle0 = angleBetween(v1.position - v0.position, v2.position - v0.position);
        float angle1 = angleBetween(v0.position - v1.position, v2.position - v1.position);
        float angle2 = angleBetween(v0.position - v2.position, v1.position - v2.position);

        // if(std::isnan(angle0) || std::isnan(angle1) || std::isnan(angle2))
        //     continue;

        v0.normal += normal * angle0;
        v1.normal += normal * angle1;
        v2.normal += normal * angle2;
    }
    for(Vertex& vertex : vertices)
    {
        vertex.normal = glm::normalize(vertex.normal);
    }
}

void Mesh::generateTangents()
{
    if(!ptr)
    {
        Console::error("Cannot generate tangents as mesh pointer is null", "Mesh");
    }

    std::vector<Vertex> &vertices = ptr->vertices;
    std::vector<Triangle> &triangles = ptr->triangles;
    if(triangles.size() == 0)
    {
        Console::error("Cannot generate tangents as there are no triangles defined", "Mesh");
        return;
    }

    for(Vertex& vertex : vertices)
    {
        vertex.tangent = glm::vec4(0.0f);
        vertex.bitangent = glm::vec3(0.0f);
    }
    for(Triangle& triangle : triangles)
    {
        Vertex& v0 = vertices[triangle.v0];
        Vertex& v1 = vertices[triangle.v1];
        Vertex& v2 = vertices[triangle.v2];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;
        

        if(glm::length(edge1) < 1e-6f || glm::length(edge2) < 1e-6f)
            continue; // Degenerate triangle

        glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
        glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

        float f = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent;
        glm::vec3 bitangent;

        if(glm::abs(f) < 1e-9f)
            continue; // Degenerate UVs
            
        tangent = (deltaUV2.y * edge1 - deltaUV1.y * edge2) / f;
        bitangent = (-deltaUV2.x * edge1 + deltaUV1.x * edge2) / f;

        glm::vec4 tempTangent = glm::vec4(tangent, 0.0f);
        v0.tangent += tempTangent;
        v1.tangent += tempTangent;
        v2.tangent += tempTangent;
        v0.bitangent += bitangent;
        v1.bitangent += bitangent;
        v2.bitangent += bitangent;
    }
    for(Vertex& vertex : vertices)
    {
        vertex.tangent = glm::normalize(vertex.tangent);
        vertex.bitangent = glm::normalize(vertex.bitangent);

        // Gram-Schmidt orthogonalize
        glm::vec3 tempTangent = glm::vec3(vertex.tangent);
        vertex.tangent = glm::vec4(glm::normalize(tempTangent - vertex.normal * glm::dot(vertex.normal, tempTangent)), 1.0f);
        vertex.bitangent = glm::normalize(vertex.bitangent - vertex.normal * glm::dot(vertex.normal, vertex.bitangent));

        // Calculate handedness
        tempTangent = glm::vec3(vertex.tangent);
        if(glm::dot(glm::cross(vertex.normal, tempTangent), vertex.bitangent) < 0.0f)
        {
            vertex.bitangent = vertex.bitangent * -1.0f;
            vertex.tangent.w = -1.0f;
        }
    }
}

Mesh Mesh::createCube(float edgeLength, const std::string& objectName)
{
    edgeLength *= 0.5f;
    float inverseSqrt3 = 1.0f / glm::sqrt(3.0f);

    std::vector<MeshData::Vertex> vertices {
        {{-edgeLength, -edgeLength, -edgeLength}, {-inverseSqrt3, -inverseSqrt3, -inverseSqrt3}, {0,0,0,1}, {0,0,0}, {1.f, 1.f, 1.f}, {0.0f, 0.0f}},
        {{edgeLength, -edgeLength, -edgeLength}, {inverseSqrt3, -inverseSqrt3, -inverseSqrt3}, {0,0,0,1}, {0,0,0}, {1.f, 1.f, 1.f}, {1.0f, 0.0f}},
        {{-edgeLength, edgeLength, -edgeLength}, {-inverseSqrt3, inverseSqrt3, -inverseSqrt3}, {0,0,0,1}, {0,0,0}, {1.f, 1.f, 1.f}, {0.0f, 0.0f}},
        {{edgeLength, edgeLength, -edgeLength}, {inverseSqrt3, inverseSqrt3, -inverseSqrt3}, {0,0,0,1}, {0,0,0}, {1.f, 1.f, 1.f}, {1.0f, 0.0f}},
        {{-edgeLength, -edgeLength, edgeLength}, {-inverseSqrt3, -inverseSqrt3, inverseSqrt3}, {0,0,0,1}, {0,0,0}, {1.f, 1.f, 1.f}, {0.0f, 1.0f}},
        {{edgeLength, -edgeLength, edgeLength}, {inverseSqrt3, -inverseSqrt3, inverseSqrt3}, {0,0,0,1}, {0,0,0}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}},
        {{-edgeLength, edgeLength, edgeLength}, {-inverseSqrt3, inverseSqrt3, inverseSqrt3}, {0,0,0,1}, {0,0,0}, {1.f, 1.f, 1.f}, {0.0f, 1.0f}},
        {{edgeLength, edgeLength, edgeLength}, {inverseSqrt3, inverseSqrt3, inverseSqrt3}, {0,0,0,1}, {0,0,0}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}},
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

    return Mesh(vertices, indices, objectName);
}

std::vector<MeshData::Vertex> generateSierpinski(float edgeLength, int depth)
{
    float root2 = glm::sqrt(2.0f);
    float root3 = glm::sqrt(3.0f);
    glm::vec3 v0 = glm::vec3(0,         0,      root3 / 3.0f) * edgeLength;
    glm::vec3 v1 = glm::vec3(0.5f,      0,      -root3 / 6.0f) * edgeLength;
    glm::vec3 v2 = glm::vec3(-0.5f,     0,      -root3 / 6.0f) * edgeLength;
    glm::vec3 v3 = glm::vec3(0.0f, root3 * 0.5f, 0.0f) * edgeLength;
    if(depth <= 0)
    {
        std::vector<MeshData::Vertex> vertices(12);
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
        
        for(MeshData::Vertex& vertex : vertices)
        {
            vertex.position = vertex.position * 1.5f;
        }

        return vertices;
    }
    else
    {
        std::vector<MeshData::Vertex> vertices0 = generateSierpinski(edgeLength, depth - 1);
        std::vector<MeshData::Vertex> vertices1 = vertices0;
        std::vector<MeshData::Vertex> vertices2 = vertices0;
        std::vector<MeshData::Vertex> vertices3 = vertices0;

        for(MeshData::Vertex& vertex : vertices0)
        {
            vertex.position = (vertex.position + v0) * 0.5f;
        }
        for(MeshData::Vertex& vertex : vertices1)
        {
            vertex.position = (vertex.position + v1) * 0.5f;
        }
        for(MeshData::Vertex& vertex : vertices2)
        {
            vertex.position = (vertex.position + v2) * 0.5f;
        }
        for(MeshData::Vertex& vertex : vertices3)
        {
            vertex.position = (vertex.position + v3) * 0.5f;
        }


        std::vector<MeshData::Vertex> vertices{};
        vertices.insert(vertices.end(), vertices0.begin(), vertices0.end());
        vertices.insert(vertices.end(), vertices1.begin(), vertices1.end());
        vertices.insert(vertices.end(), vertices2.begin(), vertices2.end());
        vertices.insert(vertices.end(), vertices3.begin(), vertices3.end());
        return vertices;
    }
}

Mesh Mesh::createSierpinskiPyramid(float edgeLength, int depth, const std::string& objectName)
{
    std::vector<Vertex> vertices = generateSierpinski(edgeLength, depth);
    
    return Mesh(vertices, objectName);
}

Mesh Mesh::createGrid(int width, int length, glm::vec2 dimensions, const std::string& objectName)
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
                {1, 0, 0, 1},
                {0, 0, 1},
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
    
    Mesh mesh = Mesh(vertices, triangles, objectName);
    mesh.generateNormals();
    return mesh;
}

Mesh Mesh::createSkybox(float size, const std::string& objectName)
{
    std::vector<Vertex> vertices{8};
    float halfSize = size / 2.0f;
    vertices[0].position = {-halfSize, -halfSize, -halfSize};
    vertices[1].position = { halfSize, -halfSize, -halfSize};
    vertices[2].position = { halfSize,  halfSize, -halfSize};
    vertices[3].position = {-halfSize,  halfSize, -halfSize};
    vertices[4].position = {-halfSize, -halfSize,  halfSize};
    vertices[5].position = { halfSize, -halfSize,  halfSize};
    vertices[6].position = { halfSize,  halfSize,  halfSize};
    vertices[7].position = {-halfSize,  halfSize,  halfSize};
    std::vector<uint32_t> triangles{
        0, 1, 2, 
        2, 3, 0,
        1, 5, 6, 
        6, 2, 1,

        5, 4, 7, 
        7, 6, 5,
        4, 0, 3, 
        3, 7, 4,
        
        3, 2, 6,
        3, 6, 7,
        4, 5, 1,
        4, 1, 0
    };

    Mesh mesh = Mesh(vertices, triangles, objectName);
    return mesh;
}

Mesh Mesh::loadObj(const std::string& filename, const std::string& objectName)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::vector<Vertex> vertices{};
    std::vector<Triangle> triangles{};

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

    Mesh mesh(vertices, triangles, objectName);
    mesh.generateTangents(); // Not included in OBJ, so we must generate them
    return mesh;
}

} // namespace core
