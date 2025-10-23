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
                std::vector<std::string> dependencies;
            };
        public:
            RenderGraph() = default;
            ~RenderGraph() = default;

            RenderGraph(const RenderGraph&) = delete;
            RenderGraph& operator=(const RenderGraph&) = delete;

            void addRenderPass(std::string name);
            void addRenderPass(std::string name, std::vector<std::string> &dependencies);
            // Add an external render pass
            // NOTE: Render graph takes ownership of the render pass
            void addRenderPass(std::string name, std::unique_ptr<RenderPass> &renderPass);
            void addRenderPass(std::string name, std::unique_ptr<RenderPass> &renderPass, std::vector<std::string> &dependencies);
            std::unique_ptr<RenderPass> &getFinalRenderPass();
            // std::unique_ptr<RenderPass> &getRenderPass(const std::string& name);

            void execute(FrameInfo& frameInfo);
        private:
            std::vector<RenderPassNode> renderPasses;
            std::map<std::string, RenderPassNode*> renderPassMap{};
    };
} // namespace graphics