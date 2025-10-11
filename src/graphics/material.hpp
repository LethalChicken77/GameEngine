#pragma once
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <string>
#include <variant>
#include <memory>
#include <stdexcept>
#include "shader.hpp"
#include "shader_resource.hpp"
#include "imgui.h"
#include "color.hpp"
// #include "containers.hpp"

namespace graphics
{
    class Material : public ShaderResource
    {
        public:
            using id_t = uint64_t;

            std::string name;

            Material() = delete;
            Material(const Material&) = delete;
            Material& operator=(const Material&) = delete;
            Material(Material&&) = default;
            Material& operator=(Material&&) = default;

            static Material instantiate(const Shader *_shader)
            {
                static id_t next_id = 0;

                return Material(next_id++, _shader);
            }
            
            void setValue(std::string name, MaterialValue value);
            void setTexture(uint32_t binding, Texture* texture);

            void createShaderInputBuffer();
            void createDescriptorSet();
            void updateValues();

            uint32_t getId() const { return id; }
            const Shader* getShader() { return shader; }

            template <class T>
            T getValue(std::string name);

            void drawImGui();
            // TODO: Handle other types

        private:
            Material(id_t mat_id, const Shader *_shader);

            uint32_t id;
            const Shader *shader;
            std::vector<ShaderInput> shaderInputs;

            bool initialized = false;
    };
} // namespace graphics