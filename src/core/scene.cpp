#include "scene.hpp"
#include "modules.hpp"
#include "temp_data.hpp"

namespace core
{
void Scene_t::loadScene()
{
    Console::log("Loading scene: " + name, "Scene");
    // Original vertex buffer testing code, must keep
    // Triangle
    // std::vector<GraphicsMesh::Vertex> vertices {
    //     {{0.0f, -0.5f, 0.0f}, {0.5f, 1.0f}},
    //     {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}},
    //     {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}}
    // };

    Mesh monkeyMesh = Mesh::loadObj("internal/models/monkey_high_res.obj", "Monkey Mesh");
    Mesh terrainMesh = Mesh::loadObj("internal/models/terrain.obj", "Terrain Mesh");
    Mesh haloMesh = Mesh::loadObj("internal/models/halo_skybox.obj", "Halo Mesh");
    // terrainMesh.generateTangents();
    Mesh cubeMesh = Mesh::createCube(0.1f, "Cube");

    GameObject obj{ObjectManager::Instantiate<GameObject_t>("Basic Monkey")};
    GameObject obj2{ObjectManager::Instantiate<GameObject_t>("Floor")};
    GameObject obj3{ObjectManager::Instantiate<GameObject_t>("Wireframe Monkey")};
    GameObject obj4{ObjectManager::Instantiate<GameObject_t>("Halo")};
    // std::cout << "Creating Grid" << std::endl;
    // obj.mesh = GraphicsMesh::createGrid(512, 512, {50.0f, 50.0f});
    // obj.materialID = 0;
    obj->mesh = monkeyMesh;
    // obj->mesh = graphics::GraphicsMesh::loadObj("internal/models/Nefertiti.obj");
    // obj->mesh->generateNormals();
    // obj->mesh->createBuffers();
    obj->material = gameData->materials.Get(0);
    obj->transform.setPosition(glm::vec3(2, 0, 0));
    // obj->transform.scale = glm::vec3(0.01f);
    // obj->transform.scale = glm::vec3(-0.01f, 0.01f, 0.01f); // TODO: Make sure negative scaling doesn't turn models inside out
    // obj->transform.rotation.x = glm::radians(-90.0f);

    // obj2.mesh = GraphicsMesh::loadObj("internal/models/monkey_high_res.obj");
    obj2->mesh = Mesh::createGrid(250,250, {500.0f, 500.0f});
    // obj2->mesh = terrainMesh;
    // obj2->material = gameData->materials.Get(0);
    obj2->material = gameData->materials.Get(2);
    obj2->transform.setPosition(glm::vec3(0, -2, 0));
    obj2->transform.setScale(glm::vec3(8));

    obj3->mesh = monkeyMesh;
    obj3->material = gameData->materials.Get(1);
    obj3->transform.setPosition(glm::vec3(-4, 0, 0));
    obj3->transform.parent = &obj->transform;

    obj4->mesh = haloMesh;
    obj4->material = gameData->materials.Get(1);
    obj4->transform.setScale(glm::vec3(10000000));
    // obj3.mesh = GraphicsMesh::createSierpinskiPyramid(12.0f, 8);
    // obj3.materialID = 2;
    gameObjects.push_back(std::move(obj));
    gameObjects.push_back(std::move(obj2));
    gameObjects.push_back(std::move(obj3));
    gameObjects.push_back(std::move(obj4));
    Console::log("Scene loaded with " + std::to_string(gameObjects.size()) + " game objects.", "Scene");
}

void Scene_t::update(double deltaTime)
{
    int counter = 0;
    for(GameObject &obj : gameObjects)
    {
        // if(obj->getInstanceID() == 6)
        //     obj->transform.rotateYaw(0.25f * deltaTime * 6.28f);
        // if(obj->getInstanceID() == 8)
        //     obj->transform.rotatePitch(0.6666f * deltaTime * 6.28f);
        // if(obj->getInstanceID() == 6)
        //     obj->transform.rotateAboutAxis(glm::vec3(1,1,1), 0.25f * deltaTime * 6.28f);

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

void Scene_t::drawScene()
{
    for(const GameObject& obj : gameObjects)
    {
        // std::vector<glm::mat4> transforms{};
        // int gridSize = 20;
        // for(int x = 0; x < gridSize; x++)
        // {
        // for(int y = 0; y < gridSize; y++)
        // {
        // for(int z = 0; z < gridSize; z++)
        // {
        //     Transform temp{};
        //     temp.setPosition(obj->transform.getPosition() + glm::vec3(x, y, z));
        //     temp.setRotation(obj->transform.getLocalRotation());
        //     transforms.push_back(temp.getTransform());
        // }
        // }
        // }
        // graphicsModule.DrawMesh(obj->mesh, obj->materialID, transforms);
        graphicsModule.DrawMesh(obj->mesh, *obj->material, obj->transform.getTransform(), obj->getInstanceID());
        // if(obj->getInstanceID() == selectedObject)
        //     graphicsModule.drawMeshOutline(obj->mesh, obj->transform.getTransform());
    }

}

Scene::Scene(const std::string& sceneName)
{
    ptr = ObjectManager::Instantiate<Scene_t>(sceneName);
}
} // namespace core