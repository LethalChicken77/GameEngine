#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace core
{
    
    struct Transform
    {
        public:
            Transform() : position(0.0f), rotation(glm::quat(1.0, 0.0, 0.0, 0.0)), scale(1.0f) {}

            void setPosition(glm::vec3 pos, bool refreshImmediate = true);
            void setRotation(glm::quat rot, bool refreshImmediate = true);
            void setEulerRotation(glm::vec3 rot, bool refreshImmediate = true);
            void setScale(glm::vec3 scale, bool refreshImmediate = true);

            // Increment transform
            void addPosition(glm::vec3 deltaPos, bool refreshImmediate = true);
            void addScale(glm::vec3 deltaSca, bool refreshImmediate = true);
            void rotateAboutAxis(glm::vec3 axis, float delta, bool refreshImmediate = true);
            void rotateYaw(float delta, bool local = true, bool refreshImmediate = true);
            void rotatePitch(float delta, bool local = true, bool refreshImmediate = true);
            void rotateRoll(float delta, bool local = true, bool refreshImmediate = true);

            glm::vec3 getPosition() const { return position; }
            glm::quat getRotation() const { return rotation; }
            glm::vec3 getRotationEuler() const { return glm::eulerAngles(rotation); }
            glm::vec3 getScale() const { return scale; }
            glm::mat4 getTransform() const 
            {
                if(parent != nullptr && parent != this && !isDescendent(parent))
                {
                    return parent->getTransform() * localTransformationMatrix;
                }
                else
                {
                    return localTransformationMatrix;
                }
            }
            glm::mat4 getLocalTransform() const { return localTransformationMatrix; }


            glm::vec3 forward() const
            {
                return glm::vec3(localTransformationMatrix[2]);
            }
            glm::vec3 right() const
            {
                return glm::vec3(localTransformationMatrix[0]);
            }
            glm::vec3 up() const
            {
                return glm::vec3(localTransformationMatrix[1]);
            }

            void recomputeMatrix();

            // Tree structure
            Transform *parent = nullptr;
            bool isDescendent(Transform* other) const;
        private:
            glm::vec3 position{};
            glm::quat rotation{1.0, 0.0, 0.0, 0.0};
            glm::vec3 scale{1.0, 1.0, 1.0};
            glm::mat4 localTransformationMatrix{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
            // glm::mat4 transformationMatrix{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    };
} // namespace core