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
// #include "containers.hpp"

namespace graphics
{
    class Material : public ShaderResource
    {
        public:
            using id_t = uint64_t;

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
            
            void setValue(std::string name, ShaderResource::Value value);

            void createShaderInputBuffer();

            uint32_t getId() const { return id; }

        private:
            Material(id_t mat_id, const Shader *_shader);

            uint32_t id;
            const Shader *shader;

            bool initialized = false;
    };
} // namespace graphics