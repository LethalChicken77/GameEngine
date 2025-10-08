#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <map>

#include "engine_types.hpp"
#include "utils/console.hpp"
#include "window.hpp"
#include "pipeline_manager.hpp"
#include "graphics_pipeline.hpp"
#include "renderer.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "texture.hpp"
#include "descriptors.hpp"
#include "frame_info.hpp"
#include "render_pass.hpp"

#include "graphics_mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "compute_shader.hpp"
#include "shader_resource.hpp"
#include "material.hpp"
#include "core/game_object.hpp" // TODO: Remove dependencies on core
#include "core/scene.hpp"
#include "core/mesh.hpp"
#include "containers.hpp"


namespace graphics
{

class Graphics
{
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    Graphics() = default; // Maybe add some functionality here later
    ~Graphics();

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    void init(const std::string& name, const std::string& engine_name);
    void cleanup();
    bool isOpen() const { return window.isOpen(); }

    void drawFrame();
    void setCamera(Camera* _camera) { camera = _camera; }

    void waitForDevice() { vkDeviceWaitIdle(device.device()); }

    Window *getWindow() { return &window; }
    Device *getDevice() { return &device; }
    void bindCameraDescriptor(FrameInfo& frameInfo, GraphicsPipeline* pipeline);
    void bindGlobalDescriptor(FrameInfo& frameInfo, GraphicsPipeline* pipeline);
    void renderSkybox(FrameInfo& frameInfo);
    void renderGameObjects(FrameInfo& frameInfo);
    void renderGameObjectIDs(FrameInfo& frameInfo);

    void graphicsInitImgui();

    void reloadShaders();

    int getClickedObjID(uint32_t x, uint32_t y);

    // Mesh management
    void setGraphicsMesh(const core::Mesh& mesh); // Create and update meshes
    void destroyGraphicsMeshes();
    void drawMesh(const core::Mesh& mesh, uint32_t materialIndex /* TODO: Pass material smart reference*/, const glm::mat4 &transform); // Draw to scene

private:
    struct MeshRenderData
    {
        MeshRenderData(id_t id, uint32_t materialID = 0, const glm::mat4& _transform = glm::mat4(1))
            : meshID(id), transforms{_transform}, materialIndex{materialID} {}
        id_t meshID;
        uint32_t materialIndex;
        std::vector<glm::mat4> transforms{};
    };
    std::vector<MeshRenderData> sceneRenderQueue{};

    void createRenderPasses();
    void loadTextures();
    void loadShaders();
    void loadMaterials();

    static void windowRefreshCallback(GLFWwindow *window);

    VkApplicationInfo appInfo{};

    Window window{WIDTH, HEIGHT, "VEngine"};
    Device device{window};
    Renderer renderer{window, device};
    // Containers containers{&device};

    std::unique_ptr<PipelineManager> pipelineManager;
    PipelineConfigInfo configInfo;

    std::unique_ptr<Buffer> globalUboBuffer;
    std::vector<std::unique_ptr<Buffer>> cameraUboBuffers;
    std::vector<std::shared_ptr<Texture>> textures;

    Camera* camera = nullptr;

    std::unique_ptr<RenderPass> idBufferRenderPass{};
    std::unique_ptr<RenderPass> sceneRenderPass{};
    std::unique_ptr<RenderPass> imguiRenderPass{};
    std::unique_ptr<RenderPass> finalRenderPass{};
    std::unique_ptr<Material> ppMaterial{};
    std::unique_ptr<Material> imguiMaterial{};
    std::unique_ptr<Material> outputMaterial{};
    std::unique_ptr<Material> skyboxMaterial{};
    std::unique_ptr<Material> idBufferMaterial{};
    core::Mesh skyboxMesh{};
    std::shared_ptr<GraphicsMesh> skyboxGraphicsMesh{};
    Texture *idTexture = nullptr;

    // Store graphics meshes based on instance ID
    std::unordered_map<id_t, std::unique_ptr<GraphicsMesh>> graphicsMeshes{};

    VkClearColorValue defaultClearColor{0.04f, 0.08f, 0.2f, 1.0f};
};

} // namespace graphics
