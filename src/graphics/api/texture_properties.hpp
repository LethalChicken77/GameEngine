#pragma once
#include "primitives/color.hpp"
#include "shared_config.hpp"

namespace graphics
{
enum class TextureType : uint8_t
{
    Default = 0, // Color, linear channels
    NormalMap = 1, // Normal maps, perform remapping beforehand for performance
    CubeMap = 2, // Cube maps, divided into 6 textures in the graphics module
    // LIGHT_MAP, SPRITE, HDRI, etc.
};

enum class TextureWrapMode : uint8_t
{
    Repeat = 0,
    MirroredRepeat = 1,
    ClampToEdge = 2,
    ClampToBorder = 3,
    MirrorClampToEdge = 4
};

enum class TextureFilterMode : uint8_t
{
    Nearest = 0,
    Bilinear = 1,
    Cubic = 2
};

enum class MipmapMode : uint8_t
{
    None = 0, // Used to determine whether to create mipmaps. Sets the sample to nearest.
    Nearest = 1,
    Linear = 2
};

enum class BorderColorMode
{
    FTransparentBlack = 0,
    ITransparentBlack = 1,
    FOpaqueBlack = 2,
    IOpaqueBlack = 3,
    FOpaqueWhite = 4,
    IOpaqueWhite = 5,
    FCustom = 1000287003,
    ICustom = 1000287004
};

enum class SwizzleChannel : uint8_t
{
    Red = 0,
    Green = 1,
    Blue = 2,
    Alpha = 3,
    Zero = 4,
    One = 5
};

struct Swizzle
{
    SwizzleChannel r = SwizzleChannel::Red;
    SwizzleChannel g = SwizzleChannel::Green;
    SwizzleChannel b = SwizzleChannel::Blue;
    SwizzleChannel a = SwizzleChannel::Alpha;
};

enum class TextureDataType : uint8_t
{
    Invalid,
    UInt,
    SInt,
    UNorm,
    SNorm,
    Float,

    Packed_B10G11R11_UFloat,
    Packed_A2R10G10B10_UNorm,

    D16_UNorm,
    D24_UNorm,
    D32_SFloat,
    D16_UNorm_S8_UInt,
    D24_UNorm_S8_UInt,
    D32_SFloat_S8_UInt,
    S8_UInt,
};

enum class ChannelOrder : uint8_t
{
    RGBA = 0,
    BGRA = 1,
    ABGR = 2
};

enum class TextureShape : uint8_t
{
    s1D,
    s2D,
    s3D,
    Cube,
    Array1D,
    Array2D,
    Array3D,
    ArrayCube
};

/// @brief Defines an image format as a set of properties.
/// @note Treat packed formats as single channel, non-srgb, RGBA, Color images. The channel size should match the packed size.
/// These types can be specified in TextureDataType as needed.
struct ImageFormat
{
    bool isSRGB = false;
    uint8_t channelCount = 4;
    uint8_t channelSize = 1;
    TextureDataType dataType = TextureDataType::UInt;
    ChannelOrder channelOrder = ChannelOrder::RGBA;

    uint32_t PixelSize() const { return channelCount * channelSize; }

    std::string ToString() const;

    bool operator==(const ImageFormat& other) const
    {
        return isSRGB == other.isSRGB
            && channelCount == other.channelCount
            && channelSize == other.channelSize
            && dataType == other.dataType
            && channelOrder == other.channelOrder;
    }

    struct Hash
    {
        size_t operator()(const ImageFormat& f) const noexcept
        {
            size_t h = std::hash<bool>()(f.isSRGB);
            h ^= std::hash<uint32_t>()(f.channelCount) + 0x9e3779b9 + (h<<6) + (h>>2);
            h ^= std::hash<uint32_t>()(f.channelSize) + 0x9e3779b9 + (h<<6) + (h>>2);
            h ^= std::hash<int>()(static_cast<int>(f.dataType)) + 0x9e3779b9 + (h<<6) + (h>>2);
            h ^= std::hash<int>()(static_cast<int>(f.channelOrder)) + 0x9e3779b9 + (h<<6) + (h>>2);
            return h;
        }
    };
};

struct TextureConfig
{
    // Preprocessing properties
    TextureType type = TextureType::Default; // Perform preprocessing based on type
    bool generateMipmaps = true;
    
    // Texture properties
    ImageFormat format{};
    Swizzle swizzle{};
    TextureShape shape = TextureShape::s2D;

    // Sampler properties
    TextureWrapMode wrapU = TextureWrapMode::Repeat;
    TextureWrapMode wrapV = TextureWrapMode::Repeat;
    TextureWrapMode wrapW = TextureWrapMode::Repeat;

    TextureFilterMode magFilter = TextureFilterMode::Bilinear;
    TextureFilterMode minFilter = TextureFilterMode::Bilinear;

    MipmapMode mipmapMode = MipmapMode::Linear;
    float mipLodBias = 0;

    BorderColorMode borderColorMode = BorderColorMode::FOpaqueBlack;
    Color borderColor = Color(0.f,0.f,0.f,1.f);

    bool anisotropicFiltering = true;
    float maxAnisotropy = 8.0f;

    bool compareEnable = false; // Used for shadow maps
    CompareOp compareOp = CompareOp::Always; // Use LessEqual for shadow maps

    float minLod = 0.0f;
    float maxLod = 1000.0f;

    bool srgb = true; // Color textures
    bool readWrite = false; // Storage textures, default false for sampled textures
};
} // namespace graphics