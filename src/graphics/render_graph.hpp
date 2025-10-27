#pragma once
#include <string>
#include <memory>
#include <map>
#include "frame_info.hpp"
#include "internal/render_pass.hpp"

namespace graphics
{

    class RenderGraph
    {
        private:
            struct RenderPassNode
            {
                std::unique_ptr<RenderPass> renderPass;
                float renderScale;
                std::vector<std::string> dependencies;
            };
        public:
            ~RenderGraph() = default;

            RenderGraph(const RenderGraph&) = delete;
            RenderGraph& operator=(const RenderGraph&) = delete;

            std::unique_ptr<RenderPass> &getFinalRenderPass();
            // std::unique_ptr<RenderPass> &getRenderPass(const std::string& name);

            void execute(FrameInfo& frameInfo);
            Texture *getRenderTexture(std::string_view name);
            Texture *getResult(std::string_view name);
        private:
            RenderGraph() = default;
            std::vector<RenderPassNode> renderPasses;
            std::map<std::string, RenderPassNode*> renderPassMap{};
            VkExtent2D extent;

            friend class RenderGraphBuilder;
    };

    class RenderGraphBuilder
    {
        public:
            RenderGraphBuilder &renderPass(std::string_view name);
            RenderGraphBuilder &renderPass(std::string_view name, const std::vector<const std::string> &dependencies);
            // Add an external render pass
            // NOTE: Render graph takes ownership of the render pass
            RenderGraphBuilder &renderPass(std::string_view name, std::unique_ptr<RenderPass> &&renderPass);
            RenderGraphBuilder &renderPass(std::string_view name, std::unique_ptr<RenderPass> &&renderPass, const std::vector<const std::string> &dependencies);
            
            RenderGraphBuilder &target(const Texture &texture);
            
            RenderGraph *build();
    };
} // namespace graphics