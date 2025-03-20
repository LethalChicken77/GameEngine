#include "noise.hpp"

namespace procedural
{

static constexpr float F2 = 0.366025403f; // (sqrt(3) - 1) / 2
static constexpr float G2 = 0.211324865f; // (3 - sqrt(3)) / 6

float calculateCornerContribution(float x, float y, int gridX, int gridY, int seed) 
{
    float t = 0.5f - x*x - y*y;
    if (t < 0) {
        return 0.0f;
    }

    // Generate pseudo-random gradient
    int rngSeed = core::Random::SeedFrom2D(gridX, gridY, seed);
    float angle = static_cast<float>(core::Random::getRandomFloat(0, 2.0f * glm::pi<float>(), rngSeed));

    // Gradient vector components using GLM
    float gx = glm::cos(angle);
    float gy = glm::sin(angle);

    // Dot product with smoother falloff
    return t * t * (gx * x + gy * y);
}

float simplex2D(float x, float y, uint32_t seed)
{
    // Skew the input space
    float s = (x + y) * F2;
    int i = static_cast<int>(glm::floor(x + s));
    int j = static_cast<int>(glm::floor(y + s));

    // Unskew the cell origin
    float t = (i + j) * G2;
    float x0 = x - (i - t);
    float y0 = y - (j - t);

    // Determine simplex
    int i1 = (x0 > y0) ? 1 : 0;
    int j1 = (x0 > y0) ? 0 : 1;

    // Offsets for other corners
    float x1 = x0 - i1 + G2;
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0f + 2.0f * G2;
    float y2 = y0 - 1.0f + 2.0f * G2;

    // Calculate contributions
    float n0 = calculateCornerContribution(x0, y0, i, j, seed);
    float n1 = calculateCornerContribution(x1, y1, i + i1, j + j1, seed);
    float n2 = calculateCornerContribution(x2, y2, i + 1, j + 1, seed);

    // Sum and scale to [0,1] range
    return (n0 + n1 + n2) * 2.f;
}

float simplex3D(float x, float y, float z, uint32_t seed)
{
}

float simplex4D(float xin, float y, float z, float w, uint32_t seed)
{
}

} // namespace procedural