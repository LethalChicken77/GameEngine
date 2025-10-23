#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <map>

#include "engine_types.hpp"
#include "utils/console.hpp"
#include "internal/window.hpp"
#include "internal/pipeline_manager.hpp"
#include "internal/graphics_pipeline.hpp"
#include "internal/renderer.hpp"
#include "internal/device.hpp"
#include "internal/descriptors.hpp"
#include "internal/render_pass.hpp"
#include "buffers/buffer.hpp"
#include "buffers/texture.hpp"
#include "frame_info.hpp"

#include "buffers/graphics_mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "compute/compute_shader.hpp"
#include "shader_resource.hpp"
#include "buffers/material.hpp"
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
    
    void graphicsInitImgui();
    
    void reloadShaders();
    
    int getClickedObjID(uint32_t x, uint32_t y);
    VkDescriptorSet getViewportDescriptorSet() const {
        return viewportDescriptorSet;
    };
    
    // Mesh management
    void setGraphicsMesh(const core::Mesh& mesh); // Create and update meshes
    void destroyGraphicsMeshes();
    void drawMesh(const core::Mesh& mesh, uint32_t materialIndex, const glm::mat4 &transform, uint32_t objectID = -1); // Draw to scene
    void drawMeshInstanced(const core::Mesh& mesh, uint32_t materialIndex, const std::vector<glm::mat4> &transforms); // Draw to scene
    void drawMeshOutline(const core::Mesh& mesh, const glm::mat4 &transform);

    void drawSkybox();

    VkExtent2D viewportSize{};

private:
    struct MeshRenderData
    {
        MeshRenderData(id_t id, const glm::mat4& _transform, uint32_t materialID = 0, id_t objID = -1)
            : meshID(id), objectID(objID), transforms{_transform}, materialIndex{materialID}, instanceBuffer{createInstanceBuffer(transforms)} {}
        MeshRenderData(id_t id, const std::vector<glm::mat4>& _transform, uint32_t materialID = 0, id_t objID = -1)
            : meshID(id), objectID(objID), transforms{_transform}, materialIndex{materialID}, instanceBuffer{createInstanceBuffer(transforms)} {}

        id_t meshID;
        uint32_t objectID;
        uint32_t materialIndex;
        std::vector<glm::mat4> transforms{};
        std::unique_ptr<Buffer> instanceBuffer{};
    };
    std::vector<MeshRenderData> sceneRenderQueue{};
    std::vector<MeshRenderData> outlineRenderQueue{};

    static std::unique_ptr<Buffer> createInstanceBuffer(const std::vector<glm::mat4>& transforms);

    void createRenderPasses();
    void loadTextures(); // TODO: Put in asset manager
    void loadShaders();
    void loadMaterials();


    void renderMeshes(FrameInfo& frameInfo, const std::vector<MeshRenderData> &renderQueue);
    void renderGameObjectIDs(FrameInfo& frameInfo);

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
    std::unique_ptr<RenderPass> outlineBaseRenderPass{};
    std::unique_ptr<RenderPass> outlineRenderPass{};
    std::unique_ptr<RenderPass> imguiRenderPass{};
    std::unique_ptr<RenderPass> finalRenderPass{};
    std::unique_ptr<Material> ppMaterial{};
    std::unique_ptr<Material> imguiMaterial{};
    std::unique_ptr<Material> outputMaterial{};
    std::unique_ptr<Material> idBufferMaterial{};
    std::unique_ptr<Material> outlineMaterial{};
    std::unique_ptr<Material> outlineResultMaterial{};
    core::Mesh skyboxMesh{};
    Texture *idTexture = nullptr;
    Texture *viewportTexture = nullptr;

    // Store graphics meshes based on instance ID
    std::unordered_map<id_t, std::unique_ptr<GraphicsMesh>> graphicsMeshes{};

    VkClearColorValue defaultClearColor{0.04f, 0.08f, 0.2f, 1.0f};

    VkDescriptorSet viewportDescriptorSet;
};

} // namespace graphics
