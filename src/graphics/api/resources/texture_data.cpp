#include "texture_data.hpp"

#include <format>
#include "utils/console.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
#include "file_util.hpp"

namespace graphics
{

TextureData::TextureData(uint32_t _width, uint32_t _height, std::string_view _name, TextureConfig props)
    : TextureData(_width, _height, 1, _name, props) {}

TextureData::TextureData(uint32_t _width, uint32_t _height, uint32_t _depth, std::string_view _name, TextureConfig props)
    : width(_width), height(_height), depth(_depth), name(_name), properties(props)
{
    data.resize(width * height * depth * properties.format.PixelSize());
}
// uint8_t TextureData::GetByte(uint32_t x, uint32_t y, uint32_t z) const
// {
//     #ifndef DISABLE_VALIDATION
//     if(x >= width || y >= height || z >= depth)
//     {
//         Console::warn(std::format("Cannot access pixel outside texture bounds: ({}, {}, {}), Size: ({}, {}, {}). Returning 0", x, y, z, width, height, depth), "TextureData");
//         return 0;
//     }
//     if(GetPixelSize() != 1)
//     {
//         Console::warn(std::format("Pixel size is not one byte. Data may be misinterpreted"), "TextureData");
//     }
//     #endif
//     uint32_t index = (z * width * height + y * width + x) * GetPixelSize();
//     if(index + 3 >= data.size())
//     {
//         throw std::out_of_range("TextureData get() index out of range");
//     }
//     return
//         (data[index] << 24) |
//         (data[index + 1] << 16) |
//         (data[index + 2] << 8) |
//         (data[index + 3]);
// }
std::unique_ptr<TextureData> TextureData::LoadFromFile(const std::string& path)
{
    // if(!std::filesystem::exists(path))
    if(!FileUtil::FileExists(path))
    {
        Console::error("File not found: " + path, "TextureData");
        return nullptr;
    }
    
    stbi_set_flip_vertically_on_load(false); // Flip texture on load to match UV coords

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if(!pixels)
    {
        throw std::runtime_error("Failed to load texture image");
    }
    uint32_t pixelCount = texWidth * texHeight;
    
    // int numChannels = texChannels; // Actual image channels (from STB)
    int numChannels = sizeof(STBI_rgb_alpha); // Force 4 Channels
    TextureConfig texConfig{};
    ImageFormat &format = texConfig.format;
    format.channelCount = numChannels;
    format.channelSize = sizeof(stbi_uc);
    format.isSRGB = true;
    format.dataType = TextureDataType::UNorm;
    format.channelOrder = ChannelOrder::RGBA;
    std::unique_ptr<TextureData> texture = std::make_unique<TextureData>(texWidth, texHeight, std::filesystem::path(path).stem().string(), texConfig);
    memcpy(texture->data.data(), pixels, pixelCount * numChannels * sizeof(stbi_uc));

    texture->width = texWidth;
    texture->height = texHeight;

    return texture;
}

/// @brief Load texture data from a file
/// @param path
/// @param dataType Override for the data type. By default, loads whatever the image specifies. Currently does nothing.
/// @param channelSize Override for the channel size. By default, loads the size of the type stored in the image. Currently does nothing.
/// @return TextureData object
std::unique_ptr<TextureData> TextureData::LoadFromFileEXR(const std::string& path, TextureDataType dataType, uint32_t channelSize)
{
    if(channelSize != 0)
    {
        assert(!(dataType == TextureDataType::Float && channelSize < 2) && "Float types cannot be smaller than 2 bytes");
        assert(!(dataType == TextureDataType::UNorm && channelSize > 2) && "Norm types cannot be larger than 2 bytes");
        assert(channelSize == 1 || channelSize == 2 || channelSize == 4 || channelSize == 8 && "Channel size must be 1, 2, 4, or 8");
    }
    if(!FileUtil::FileExists(path))
    {
        Console::error("File not found: " + path, "TextureData");
        return nullptr;
    }

    const char* err = nullptr;

    // Parse EXR version
    EXRVersion version;
    if (ParseEXRVersionFromFile(&version, path.c_str()) != TINYEXR_SUCCESS) {
        throw std::runtime_error("Failed to parse EXR version");
    }

    // Parse the EXR header
    EXRHeader header;
    InitEXRHeader(&header);
    if (ParseEXRHeaderFromFile(&header, &version, path.c_str(), &err) != TINYEXR_SUCCESS) {
        std::string errorMsg = err ? std::string(err) : "Unknown EXR header error";
        FreeEXRErrorMessage(err);
        throw std::runtime_error("Failed to parse EXR header: " + errorMsg);
    }

    // Request conversion to FLOAT for any HALF channels
    for (int i = 0; i < header.num_channels; ++i) {
        if (header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
            header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
        }
    }
    // Load the EXR image
    EXRImage image;
    if (LoadEXRImageFromFile(&image, &header, path.c_str(), &err) != TINYEXR_SUCCESS) {
        std::string errorMsg = err ? std::string(err) : "Unknown EXR image error";
        FreeEXRErrorMessage(err);
        throw std::runtime_error("Failed to load EXR image: " + errorMsg);
    }
    // Access image data
    int numChannels = image.num_channels;
    int texWidth = image.width;
    int texHeight = image.height;

    size_t rIndex, gIndex, bIndex, aIndex = -1;

    int pixelType = header.pixel_types[0];
    for(int i = 0; i < numChannels; i++)
    {
        if(strcmp(header.channels[i].name, "R") == 0 || strcmp(header.channels[i].name, "Y") == 0) rIndex = i;
        else if(strcmp(header.channels[i].name, "G") == 0) gIndex = i;
        else if(strcmp(header.channels[i].name, "B") == 0) bIndex = i;
        else if(strcmp(header.channels[i].name, "A") == 0) aIndex = i;

        if(header.pixel_types[i] != pixelType)
        {
            Console::errorf("Failed to load EXR image \"{}\": Channels have different pixel sizes.", path);
            return nullptr;
        }
    }

    const uint32_t pixelCount = texWidth * texHeight;

    // Map channels by name
    std::vector<float> chR(pixelCount);
    memcpy(chR.data(), image.images[rIndex], sizeof(float) * pixelCount);
    std::vector<float> chG, chB, chA;
    if (numChannels >= 2) {
        chG.resize(pixelCount);
        memcpy(chG.data(), image.images[gIndex], sizeof(float) * pixelCount);
    }
    if (numChannels >= 3) {
        chB.resize(pixelCount);
        memcpy(chB.data(), image.images[bIndex], sizeof(float) * pixelCount);
    }
    if (numChannels >= 4) {
        chA.resize(pixelCount);
        memcpy(chA.data(), image.images[aIndex], sizeof(float) * pixelCount);
    }

    
    int effectiveNumChannels = (numChannels == 3) ? 4 : numChannels; // Expand RGB to RGBA
    TextureConfig texConfig{};
    ImageFormat& format = texConfig.format;
    format.channelCount = effectiveNumChannels;
    // if(channelSize == 0) // TODO: Add casting step. Probably in importer tbh
    // {
        switch(pixelType)
        {
        case TINYEXR_PIXELTYPE_HALF:
            format.channelSize = 2;
            format.dataType = TextureDataType::Float;
            break;
        case TINYEXR_PIXELTYPE_FLOAT:
            format.channelSize = 4;
            format.dataType = TextureDataType::Float;
            break;
        case TINYEXR_PIXELTYPE_UINT:
            format.channelSize = 4;
            format.dataType = TextureDataType::UInt;
            break;
        default:
            Console::errorf("Failed to load EXR image \"{}\": Invalid pixel type.", path);
            break;
        }
    // }
    format.channelOrder = ChannelOrder::RGBA; // TODO: Allow user specified order
    std::unique_ptr<TextureData> texture = std::make_unique<TextureData>(texWidth, texHeight, std::filesystem::path(path).stem().string(), texConfig);

    texture->data.resize(pixelCount * sizeof(float) * effectiveNumChannels);

    for (uint32_t i = 0; i < pixelCount; i++)
    {
        // std::cout << "Pixel: " << i << " / " << pixelCount << "\n";
        float pixelData[4] = {
            chR[i],
            (numChannels >= 2) ? chG[i] : 0.0f,
            (numChannels >= 3) ? chB[i] : 0.0f,
            (numChannels >= 4) ? chA[i] : 1.0f
        };
        memcpy(
            texture->data.data() + i * sizeof(float) * effectiveNumChannels, 
            pixelData, 
            sizeof(float) * effectiveNumChannels
        );
    }
    
    if(!header.tiled)
    {
        image.num_tiles = 0; // Prevent tinyexr from trying to free tiles
        image.tiles = nullptr;
    }
    if(image.next_level)
    {
        image.next_level = nullptr; // Prevent tinyexr from trying to free mipmaps
    }
    // Free the image data when done
    FreeEXRImage(&image);
    FreeEXRHeader(&header);


    texture->width = texWidth;
    texture->height = texHeight;
    texture->name = std::filesystem::path(path).stem().string();

    return std::move(texture);
}
} // namespace core