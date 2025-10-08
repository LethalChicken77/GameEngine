#include <iostream>

#include "camera.hpp"
#include "utils/debug.hpp"

namespace graphics
{
    Camera::Camera()
    {
        CameraProperties properties = {0.01f, 100.0f, 90.0f};
        setProperties(properties);
    }

    Camera::Camera(const CameraProperties properties, const bool _orthographic)
    {
        setProperties(properties, _orthographic);
    }

    void Camera::setProperties(const CameraProperties properties, const bool _orthographic)
    {
        this->properties = properties;
        this->isOrthographic = _orthographic;
        updateCamera();
    }

    void Camera::setNear(const float near)
    {
        properties.near = near;
        updateCamera();
    }

    void Camera::setFar(const float far)
    {
        properties.far = far;
        updateCamera();
    }

    void Camera::setVfov(const float vfov)
    {
        properties.vfov = vfov;
        updateCamera();
    }

    void Camera::setAspectRatio(const float _aspectRatio)
    {
        aspectRatio = _aspectRatio;
        // std::cout << "Aspect ratio: " << aspectRatio << std::endl;
        updateCamera();
    }
    
    void Camera::updateCamera()
    {
        if(isOrthographic)
        {
            float top = properties.vfov * 0.5f;
            float bottom = -top;
            float right = aspectRatio * top;
            float left = -right;

            projection = glm::mat4(
                2.f / (right - left), 0, 0, 0,
                0, 2.f / (bottom - top), 0, 0,
                0, 0, 1.f / (properties.far - properties.near), 0,
                -(right + left) / (right - left), -(bottom + top) / (bottom - top), properties.near / (properties.far - properties.near), 1.f
            );
            // perspective = glm::mat4(1);
        }
        else
        {
            float halfTanFovY = glm::tan(glm::radians(properties.vfov * 0.5f)) * 0.5f;

            projection = glm::mat4(
                1.f / (aspectRatio * halfTanFovY), 0, 0, 0,
                0, -1.f / halfTanFovY, 0, 0,
                0, 0, properties.far / (properties.far - properties.near), 1.f,
                0, 0, -properties.near * properties.far / (properties.far - properties.near), 0
            );
        }

        // projection = orthographic * perspective;
    }
} // namespace graphics