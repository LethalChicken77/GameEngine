#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "graphics_pipeline.hpp"

namespace graphics
{
    class Shader
    {
        public:
            struct ShaderInput
            {
                std::string name;
                enum class DataType
                {
                    FLOAT,
                    VEC2,
                    VEC3,
                    VEC4,
                    MAT2,
                    MAT3,
                    MAT4,
                    INT,
                    BOOL
                } type;
            };

            std::string path;

            Shader(const std::string& path, const PipelineConfigInfo& configInfo, std::vector<ShaderInput> inputs);
            ~Shader();
        private:
            PipelineConfigInfo configInfo;
            std::vector<ShaderInput> inputs{};
    };
} // namespace graphics