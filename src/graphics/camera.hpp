#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "../core/transform.hpp"

namespace graphics
{
    struct CameraProperties
    {
        float near;
        float far;
        // float whereverYouAre;
        float vfov;
    };

    class Camera
    {
    public:
        
        Camera();
        Camera(const CameraProperties properties, const bool orthographic = false);

        // Camera(const Camera&) = delete;
        // Camera& operator=(const Camera&) = delete;

        void setProperties(const CameraProperties properties, const bool orthographic = false);
        void setNear(const float near);
        void setFar(const float far);
        void setVfov(const float vfov);
        void setAspectRatio(const float aspectRatio);

        glm::mat4 getView() const { return transform.getTransform(); }
        glm::mat4 getProjection() const { return projection; }
        glm::mat4 getViewProjection() const { return projection * glm::inverse(transform.getTransform()); }

        core::Transform transform;
    private:
        glm::mat4 perspective;
        glm::mat4 orthographic;

        glm::mat4 projection;
        glm::mat4 view;

        bool isOrthographic;
        CameraProperties properties;
        float aspectRatio; // Horizontal / Vertical

        void updateCamera();
    };
}