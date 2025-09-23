#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace graphics
{
class Spline
{
public:
    enum SplineType
    {
        LINEAR = 0,
        HERMITE = 1,
        BSPLINE = 2,
        NURBS = 3,
        PUPS = 4
    };
    SplineType type = LINEAR;
    uint32_t verticesPerSegment = 5;
    std::vector<glm::vec3> controlPoints{};

    glm::vec3 SampleSpline(float u);
    std::vector<glm::vec3> GetVertices();

private:
    std::vector<float> weights;
};
} // namespace graphics