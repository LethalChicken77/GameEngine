#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vulkan/vulkan.h>

namespace debug
{
    extern void log(const std::string& message);
    extern void logerr(const std::string& message);

    extern std::string vec2ToString(const glm::vec2& v);
    extern std::string vec3ToString(const glm::vec3& v);
    extern std::string vec4ToString(const glm::vec4& v);

    extern std::string mat2ToString(const glm::mat2& m);
    extern std::string mat3ToString(const glm::mat3& m);
    extern std::string mat4ToString(const glm::mat4& m);
}