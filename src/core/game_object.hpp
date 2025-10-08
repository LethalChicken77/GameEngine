#pragma once
#include "graphics/graphics_mesh.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "object.hpp"
#include "components/transform.hpp"
#include "components/mesh_components.hpp"
#include "utils/smart_reference.hpp"

namespace core
{
    class GameObject_t : public Object
    {
    public:
        static constexpr const char* className = "Game Object";
        const char* GetClassName() const { return className; }
        
        using Object::Object;
        GameObject_t(const GameObject_t&) = delete;
        GameObject_t& operator=(const GameObject_t&) = delete;
        GameObject_t(GameObject_t&&) = delete;
        GameObject_t& operator=(GameObject_t&&) = delete;

        id_t get_id() const { return localID; }
        Mesh mesh{};

        Transform transform;
        // MeshRenderer meshRenderer;

        id_t materialID{};
    protected:
        GameObject_t(id_t newID) : Object(newID) {}
        id_t localID; // ID local to scene/prefab
        friend class ObjectManager;
    };

    class GameObject : public SmartRef<GameObject_t>
    {
        public:
            using SmartRef<GameObject_t>::SmartRef;
    };
} // namespace core