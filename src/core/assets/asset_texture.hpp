#pragma once
#include "../asset.hpp"
#include "../../graphics/texture.hpp"

namespace core
{
    class TextureAsset : public Asset
    {
    public:
        TextureAsset(const TextureAsset&) = delete;
        TextureAsset& operator=(const TextureAsset&) = delete;
        TextureAsset(TextureAsset&&) = delete;
        TextureAsset& operator=(TextureAsset&&) = delete;

        static std::unique_ptr<TextureAsset> Instantiate(std::string name = "New Asset")
        {
            std::unique_ptr<TextureAsset> parent = Object::Instantiate<TextureAsset>(name);
            return std::move(parent); // TODO: Put somewhere
        }

        id_t materialID{};
    private:
        TextureAsset(id_t newID) : Asset(newID) {}
        friend std::unique_ptr<TextureAsset> Object::Instantiate<TextureAsset>(std::string);

        std::unique_ptr<graphics::Texture> texture;
    };
}