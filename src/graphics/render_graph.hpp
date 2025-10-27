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
                std::vector<RenderPassNode*> dependencies;
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
        private:
            RenderGraph newGraph;
        public:
            RenderGraphBuilder &AddRenderPass(std::string_view name);
            RenderGraphBuilder &AddRenderPass(std::string_view name, const std::vector<const std::string> &dependencies);
            // Add an external render pass
            // NOTE: Render graph takes ownership of the render pass
            RenderGraphBuilder &AddRenderPass(std::string_view name, std::unique_ptr<RenderPass> &&renderPass);
            RenderGraphBuilder &AddRenderPass(std::string_view name, std::unique_ptr<RenderPass> &&renderPass, const std::vector<const std::string> &dependencies);
            
            RenderGraphBuilder &Target(const Texture &texture);
            
            std::unique_ptr<RenderGraph> Build();
    };
} // namespace graphics