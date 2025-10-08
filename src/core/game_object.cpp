#include "game_object.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace core
{

// void Transform::setPosition(glm::vec3 pos)
// {
//     position = pos;
// }

void GameObject_t::createGraphicsMesh()
{
    graphicsMesh = std::make_shared<graphics::GraphicsMesh>(mesh.get());
}

} // namespace core