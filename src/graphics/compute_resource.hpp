#pragma once
#include "shader_resource.hpp"
#include "compute_shader.hpp"

namespace graphics
{
    class ComputeResources : public ShaderResource
    {
        public:
            static ComputeResources instantiate(const ComputeShader *_shader)
            {
                static id_t next_id = 0;

                return ComputeResources(next_id++, _shader);
            }
        private:
            ComputeResources(id_t mat_id, const ComputeShader *_shader);
            const ComputeShader *shader;
    };
} // namespace graphics