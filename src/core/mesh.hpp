#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "object_manager.hpp"
#include "utils/console.hpp"
#include "utils/smart_reference.hpp"

namespace core
{
    class MeshData : public Object
    {
    public:
    
        struct Vertex
        {
            glm::vec3 position{};
            glm::vec3 normal{};
            glm::vec4 tangent{};
            glm::vec3 bitangent{};
            glm::vec3 color{1.0f, 1.0f, 1.0f};
            glm::vec2 texCoord{};

        };

        struct Triangle
        {
            uint32_t v0;
            uint32_t v1;
            uint32_t v2;
        };

        void SetMesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);
        void SetMesh(const std::vector<Vertex>& _vertices, const std::vector<Triangle>& _indices);
        ~MeshData();

        MeshData(const MeshData&) = delete;
        MeshData& operator=(const MeshData&) = delete;

        std::vector<Vertex> vertices{};
        std::vector<Triangle> triangles{};

    private:
        
        void loadModelFromObj(const std::string& filename); // TODO: Asset importer
    };

    class Mesh : public SmartRef<MeshData>
    {
        public:
            using SmartRef<MeshData>::SmartRef; // Inherit base constructor

            using Vertex = MeshData::Vertex;
            using Triangle = MeshData::Triangle;

            Mesh(std::vector<Vertex> &vertices);
            Mesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
            Mesh(std::vector<Vertex> &vertices, std::vector<Triangle> &triangles);

            void generateNormals();
            void generateTangents();

            void PrintInfo() const
            {
                if(ptr) 
                {
                    Console::log("Mesh " + ptr->name + " has " + std::to_string(ptr->vertices.size()) + " vertices and " + std::to_string(ptr->triangles.size()) + " triangles.", "Mesh");
                }
            }

            // Mesh creation
            static Mesh createCube(float edgeLength);
            static Mesh createSierpinskiPyramid(float edgeLength, int depth);
            static Mesh createGrid(int width, int length, glm::vec2 dimensions);
            static Mesh createSkybox(float size);
            static Mesh loadObj(const std::string& filename); // TODO: replace with loadFromFile
        private:
            void loadModelFromObj(const std::string& filename);
    };
}