#include "game_object.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace core
{

glm::mat4 Transform::getTransform() const
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), {position.x, position.y, position.z});
    glm::mat4 rotationM = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0));
    rotationM = glm::rotate(rotationM, rotation.x, glm::vec3(1, 0, 0));
    rotationM = glm::rotate(rotationM, rotation.z, glm::vec3(0, 0, 1));
    glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);

    return translation * rotationM * scaling;
}

// void Transform::setPosition(glm::vec3 pos)
// {
//     position = pos;
// }


} // namespace core