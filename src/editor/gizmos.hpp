#include "core/mesh.hpp"

namespace editor
{
    class Gizmos
    {
        public:
            // Visuals for transformation gizmos
            static std::unique_ptr<core::MeshData> createMoveHandleMesh();
            static std::unique_ptr<core::MeshData> createScaleHandleMesh();
            static std::unique_ptr<core::MeshData> createRotateHandleMesh();
            static std::unique_ptr<core::MeshData> createUniformHandleMesh();
            static std::unique_ptr<core::MeshData> createPlanarHandleMesh(); // Uniform planar transformation
            static std::unique_ptr<core::MeshData> createRotateHandleOccluder(); // Sphere to hide back side of rotate handles
            
            // Hitboxes for handles, bigger than the visuals
            static std::unique_ptr<core::MeshData> createBarHandleHitbox(); // Shared between move and scale handles
            static std::unique_ptr<core::MeshData> createUniformHandleHitbox();
            static std::unique_ptr<core::MeshData> createPlanarHandleHitbox();
            static std::unique_ptr<core::MeshData> createRotateHandleHitbox();

    };
} // namespace editor