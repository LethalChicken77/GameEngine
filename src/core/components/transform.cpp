#include "transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace core
{

void Transform::setPosition(glm::vec3 pos, bool refreshImmediate)
{
    position = pos;
    if(refreshImmediate) recomputeMatrix();
}

void Transform::setRotation(glm::quat rot, bool refreshImmediate)
{
    rotation = rot;
    if(refreshImmediate) recomputeMatrix();
}

void Transform::setEulerRotation(glm::vec3 rot, bool refreshImmediate)
{
    rotation = glm::quat(rot);
    // glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0));
    // rotationM = glm::rotate(rotationM, rotation.x, glm::vec3(1, 0, 0));
    // rotationM = glm::rotate(rotationM, rotation.z, glm::vec3(0, 0, 1));
    if(refreshImmediate) recomputeMatrix();
}

void Transform::setScale(glm::vec3 sca, bool refreshImmediate)
{
    scale = sca;
    if(refreshImmediate) recomputeMatrix();
}

void Transform::addPosition(glm::vec3 deltaPos, bool refreshImmediate)
{
    position += deltaPos;
    if(refreshImmediate) recomputeMatrix();
}

void Transform::addScale(glm::vec3 deltaSca, bool refreshImmediate)
{
    scale += deltaSca;
    if(refreshImmediate) recomputeMatrix();
}

void Transform::rotateAboutAxis(glm::vec3 axis, float delta, bool refreshImmediate)
{
    glm::quat deltaQuat = glm::angleAxis(delta, glm::normalize(axis));
    rotation = deltaQuat * rotation; // order depends on local vs world
    if(refreshImmediate) recomputeMatrix();
}

void Transform::rotateYaw(float delta, bool local, bool refreshImmediate)
{
    glm::vec3 axis = local ? up() : glm::vec3(0,1,0);
    rotateAboutAxis(axis, delta, refreshImmediate);
}

void Transform::rotatePitch(float delta, bool local, bool refreshImmediate)
{
    glm::vec3 axis = local ? right() : glm::vec3(1,0,0);
    rotateAboutAxis(axis, delta, refreshImmediate);
}

void Transform::rotateRoll(float delta, bool local, bool refreshImmediate)
{
    glm::vec3 axis = local ? forward() : glm::vec3(0,0,1);
    rotateAboutAxis(axis, delta, refreshImmediate);
}



void Transform::recomputeMatrix()
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), {position.x, position.y, position.z});
    glm::mat4 rotationM = glm::toMat4(rotation);
    glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);

    localTransformationMatrix = translation * rotationM * scaling;
}

bool Transform::isDescendent(Transform* other) const
{
    if(other == this) return true;
    if(other == nullptr) return false;
    return isDescendent(other->parent);
}


} // namespace core