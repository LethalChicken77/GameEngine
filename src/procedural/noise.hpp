#pragma once
#include "utils/random.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace procedural
{
    extern float simplex2D(float xin, float yin, uint32_t seed = 0);
    extern float simplex3D(float xin, float yin, float zin, uint32_t seed = 0);
    extern float simplex4D(float xin, float yin, float zin, float win, uint32_t seed = 0);

} // namespace procedural