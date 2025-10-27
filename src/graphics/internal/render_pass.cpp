#include "render_pass.hpp"
#include "graphics/containers.hpp"

namespace graphics
{
// RenderPass::RenderPass(VkExtent2D _extent) : extent(_extent)
// {

// }

RenderPass::~RenderPass()
{
    destroyBuffers();
}

void RenderPass::destroyBuffers()
{
    if(frameBuffer != VK_NULL_HANDLE)
        vkDestroyFramebuffer(Shared::device->device(), frameBuffer, nullptr);
    if(renderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(Shared::device->device(), renderPass, nullptr);
    frameBuffer = VK_NULL_HANDLE;
    renderPass = VK_NULL_HANDLE;
    vkDeviceWaitIdle(Shared::device->device());
}

void RenderPass::addColorAttachment(VkFormat imageFormat, VkImageLayout imageLayout)
{
    TextureProperties props = TextureProperties::getDefaultProperties();
    props.format = imageFormat;
    props.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    props.finalLayout = imageLayout;

    SamplerProperties sprops = SamplerProperties::getDefaultProperties();
    sprops.magFilter = VK_FILTER_LINEAR;

    addColorAttachment(props, sprops, imageFormat, imageLayout);
}

void RenderPass::addColorAttachment(TextureProperties textureProps, VkFormat imageFormat, VkImageLayout imageLayout)
{
    SamplerProperties sprops = SamplerProperties::getDefaultProperties();
    sprops.magFilter = VK_FILTER_LINEAR;

    addColorAttachment(textureProps, sprops, imageFormat, imageLayout);
}

void RenderPass::addColorAttachment(SamplerProperties samplerProps, VkFormat imageFormat, VkImageLayout imageLayout)
{
    TextureProperties props = TextureProperties::getDefaultProperties();
    props.format = imageFormat;
    props.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    props.finalLayout = imageLayout;

    addColorAttachment(props, samplerProps, imageFormat, imageLayout);
}

void RenderPass::addColorAttachment(TextureProperties textureProps, SamplerProperties samplerProps, VkFormat imageFormat, VkImageLayout imageLayout)
{
    std::unique_ptr<Texture> tex = std::make_unique<Texture>(textureProps, samplerProps, extent.width, extent.height);
    tex->createTextureUninitialized();
    images.push_back(std::move(tex));

    colorImageLayout = imageLayout;
}

void RenderPass::addDepthAttachment(VkImageLayout imageLayout)
{
    TextureProperties props = TextureProperties::getDefaultProperties();
    props.format = findDepthFormat();
    props.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    props.imageSubResourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    props.finalLayout = imageLayout;

    depthImage = std::make_unique<Texture>(props, extent.width, extent.height);
    depthImage->createTextureUninitialized();

    depthImageLayout = imageLayout;
}

void RenderPass::updateImageSizes()
{
    for(std::unique_ptr<Texture> &texture : images)
    {
        TextureProperties props = texture->properties;
        texture.reset();
        texture = std::make_unique<Texture>(props, extent.width, extent.height); // Recreate textures with new size
        texture->createTextureUninitialized();
    }
    if(depthImage)
    {
        TextureProperties props = depthImage->properties;
        depthImage.reset();
        depthImage = std::make_unique<Texture>(props, extent.width, extent.height);
        depthImage->createTextureUninitialized();
    }
}

void RenderPass::resetLayouts()
{
    for(std::unique_ptr<Texture> &texture : images)
    {
        texture->transitionImageLayout(colorImageLayout);
    }
    if(depthImage)
    {
        depthImage->transitionImageLayout(depthImageLayout);
    }
}

void RenderPass::resetLayouts(VkCommandBuffer commandBuffer)
{
    for(std::unique_ptr<Texture> &texture : images)
    {
        texture->transitionImageLayout(colorImageLayout, commandBuffer);
    }
    if(depthImage)
    {
        depthImage->transitionImageLayout(depthImageLayout, commandBuffer);
    }
}

void RenderPass::create(VkExtent2D _extent)
{
    extent = _extent;
    destroyBuffers();
    updateImageSizes();
    createRenderPass();
    createFrameBuffers();
}

void RenderPass::createRenderPass()
{
    std::vector<VkAttachmentReference> colorAttachmentRefs{};
    std::vector<VkAttachmentDescription> attachments{};
    uint32_t indexer = 0;
    for(std::unique_ptr<Texture> &tex : images)
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = tex->properties.format;
        colorAttachment.samples = tex->properties.sampleCount;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = tex->properties.finalLayout;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = indexer++;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        colorAttachmentRefs.push_back(colorAttachmentRef);
        attachments.push_back(colorAttachment);
    }


    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = colorAttachmentRefs.size();
    subpass.pColorAttachments = colorAttachmentRefs.data();
    if(depthImage)
    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = indexer++;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        attachments.push_back(depthAttachment);
    }
    else
    {
        subpass.pDepthStencilAttachment = nullptr;
    }
    

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstSubpass = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(Shared::device->device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderPass::createFrameBuffers() 
{
    std::vector<VkImageView> attachments{};
    for(std::unique_ptr<Texture> &tex : images)
    {
        VkImageView view = tex->getImageView();
        if(view == VK_NULL_HANDLE)
            throw std::runtime_error("Texture image view is invalid!");
        attachments.push_back(view);
    }
    if(depthImage)
    {
        VkImageView depthView = depthImage->getImageView();
        if(depthView == VK_NULL_HANDLE)
            throw std::runtime_error("Depth image view is invalid!");
        attachments.push_back(depthView);
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    if(attachments.size() > 0)
        framebufferInfo.pAttachments = attachments.data();
    else
        framebufferInfo.pAttachments = nullptr;
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(
            Shared::device->device(),
            &framebufferInfo,
            nullptr,
            &frameBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
}

VkFormat RenderPass::findDepthFormat() 
{
    return Shared::device->findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
} // namespace graphics