#include "core/mesh.hpp"
#include "graphics/material.hpp"

namespace core
{
struct MeshRenderer
{
    MeshData mesh;
    graphics::Material* material;
    // Cast shadows

};

struct MeshCollider
{
    MeshData mesh;
    // Physics material
    // Convex hull generation
};

} // namespace core