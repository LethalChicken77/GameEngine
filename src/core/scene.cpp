#include "scene.hpp"
#include "../graphics/mesh.hpp"

namespace core
{
void Scene_t::loadScene()
{
    std::cout << "Loading scene" << std::endl;
    // Triangle
    // std::vector<Mesh::Vertex> vertices {
    //     {{0.0f, -0.5f, 0.0f}, {0.5f, 1.0f}},
    //     {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}},
    //     {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}}
    // };


    std::unique_ptr<GameObject_t> obj = GameObject_t::Instantiate();
    std::unique_ptr<GameObject_t> obj2 = GameObject_t::Instantiate();
    std::unique_ptr<GameObject_t> obj3 = GameObject_t::Instantiate();
    // std::cout << "Creating Grid" << std::endl;
    // obj.mesh = Mesh::createGrid(512, 512, {50.0f, 50.0f});
    // obj.materialID = 0;
    obj->mesh = graphics::Mesh::loadObj("internal/models/monkey_high_res.obj");
    // obj->mesh = graphics::Mesh::loadObj("internal/models/Nefertiti.obj");
    // obj->mesh->generateNormals();
    // obj->mesh->createBuffers();
    obj->materialID = 0;
    // obj->transform.scale = glm::vec3(0.01f);
    // obj->transform.scale = glm::vec3(-0.01f, 0.01f, 0.01f); // TODO: Make sure negative scaling doesn't turn models inside out
    // obj->transform.rotation.x = glm::radians(-90.0f);

    // obj2.mesh = Mesh::loadObj("internal/models/monkey_high_res.obj");
    obj2->mesh = graphics::Mesh::createGrid(16,16, {50.0f, 50.0f});
    obj2->materialID = 2;
    obj2->transform.position = glm::vec3(0, -3, 0);

    obj3->mesh = graphics::Mesh::loadObj("internal/models/monkey_high_res.obj");
    obj3->materialID = 1;
    obj3->transform.position = glm::vec3(-3, 0, 0);
    // obj3.mesh = Mesh::createSierpinskiPyramid(12.0f, 8);
    // obj3.materialID = 2;
    gameObjects.push_back(std::move(obj));
    gameObjects.push_back(std::move(obj2));
    gameObjects.push_back(std::move(obj3));
    std::cout << "Loaded game objects" << std::endl;
}

void Scene_t::update(double deltaTime)
{
    int counter = 0;
    for(std::unique_ptr<core::GameObject_t> &obj : gameObjects)
    {
        // if(obj->get_id() == 0)
        //     obj->transform.position = camera.transform.position; // Skybox
        // if(obj.get_id() == 2) break;
        // if(obj.get_id() == 1)
        // {
        //     obj.transform.rotation.y += 0.5f * deltaTime;
        // }
        // obj.transform.position = glm::vec3(glm::sin(glfwGetTime()) * (counter % 2 ? 1 : -1));
        // obj.transform.position = glm::vec3(glm::sin(glm::radians(324.f)) * (counter % 2 ? 1 : -1));
        // obj.transform.rotation = glm::vec3(glm::radians(glfwGetTime() * 90.0f) * (counter % 2 ? 1 : -1));
        // obj.transform.rotation = glm::vec3(glm::radians(324.f) * (counter % 2 ? 1 : -1));
        // obj.transform.rotation.x = glm::radians(-90.0f);
        counter++;
    }
}
} // namespace core