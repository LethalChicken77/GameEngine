#include <glm/glm.hpp>
#include <vector>
#include "core/object.hpp"

// Reinventing the wheel, for fun

namespace physics
{
    struct HitInfo
    {
        glm::vec3 hitPos{};
        glm::vec3 hitNormal{};
    };

    struct RaycastHit
    {
        glm::vec3 hitPos{};
        glm::vec3 hitNormal{};
        core::Object::id_t hitCollider = -1;
        core::Object::id_t hitObj = -1;
    };
    
    struct RaycastResult
    {
        std::vector<RaycastHit> hitInfo;
    };

    

    RaycastHit Raycast(
        glm::vec3 origin,
        glm::vec3 direction,
        float distance
    );

    RaycastResult RaycastAll(
        glm::vec3 origin,
        glm::vec3 direction,
        float distance
    );
} // namespace physics