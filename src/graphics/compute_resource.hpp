#pragma once
#include "shader_resource.hpp"
#include "compute_shader.hpp"

namespace graphics
{
    class ComputeResource : public ShaderResource
    {
        public:
            using id_t = uint64_t;

            ComputeResource() = delete;
            ComputeResource(const ComputeResource&) = delete;
            ComputeResource& operator=(const ComputeResource&) = delete;
            ComputeResource(ComputeResource&&) = default;
            ComputeResource& operator=(ComputeResource&&) = default;

            static ComputeResource instantiate(const ComputeShader *_shader)
            {
                static id_t next_id = 0;

                return ComputeResource(next_id++, _shader);
            }
            
            void setValue(std::string name, ShaderResource::Value value);
            void setTexture(uint32_t binding, std::shared_ptr<graphics::Texture> texture);

            void createShaderInputBuffer();
            void updateDescriptorSet();

            uint32_t getId() const { return id; }
            
        private:
            ComputeResource(id_t mat_id, const ComputeShader *_shader);

            uint32_t id;
            const ComputeShader *shader;

            bool initialized = false;
    };
} // namespace graphics