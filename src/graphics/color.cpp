#include "color.hpp"
#include <algorithm>
#include <cmath>
#include "utils/console.hpp"

namespace graphics
{

Color::Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
Color::Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) : r(_r / 255.0f), g(_g / 255.0f), b(_b / 255.0f), a(_a / 255.0f) {}
Color::Color(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
Color::Color(double _r, double _g, double _b, double _a) : r((float)_r), g((float)_g), b((float)_b), a((float)_a) {}
Color::Color(const glm::vec3 &color) : r(color.r), g(color.g), b(color.b), a(1.0f) {}
Color::Color(const glm::vec4 &color) : r(color.r), g(color.g), b(color.b), a(color.a) {}
Color::Color(const std::string &hexValue)
{
    uint32_t stringLength = hexValue.length();
    std::string trimmedString;
    if(hexValue.length() < 6)
    {
        Console::warn("Invalid hex string, defaulting to black", "Color");
        return;
    }
    if(hexValue.starts_with("0x"))
    {
        trimmedString = hexValue.substr(2, stringLength);
    }
    else if(hexValue.starts_with("#"))
    {
        trimmedString = hexValue.substr(1, stringLength);
    }
    if(trimmedString.length() == 6)
    {
        trimmedString += "FF";
    }
    if(trimmedString.length() != 8)
    {
        Console::warn("Invalid hex string, defaulting to black", "Color");
        return;
    }

    uint32_t result = 0;

    for(int i = 0; i < 8; i++)
    {
        result = result << 4;
        char currentChar = trimmedString[i];
        if(currentChar >= '0' && currentChar <= '9')
        {
            result |= (currentChar - '0');
        }
        else if(currentChar >= 'A' && currentChar <= 'F')
        {
            result |= (currentChar - 'A') + 10;
        }
        else if(currentChar >= 'a' && currentChar <= 'f')
        {
            result |= (currentChar - 'a') + 10;
        }
    }
    a = static_cast<float>(result & 0xFF) / 255.0f;
    b = static_cast<float>((result >> 8) & 0xFF) / 255.0f;
    g = static_cast<float>((result >> 16) & 0xFF) / 255.0f;
    r = static_cast<float>((result >> 24) & 0xFF) / 255.0f;
}

Color Color::hex(uint32_t value)
{
    return Color(
        ((value >> 24) & 0xFF) / 255.0f,
        ((value >> 16) & 0xFF) / 255.0f,
        ((value >> 8) & 0xFF) / 255.0f,
        (value & 0xFF) / 255.0f
    );
}

Color Color::hex(const std::string &value)
{
    uint32_t hexValue = std::stoul(value, nullptr, 16);
    return hex(hexValue);
}

Color Color::HSV(float h, float s, float v, float a)
{
    float modH = fmod(h, 360.0f);
    float C = v * s;
    float X = C * (1 - abs(fmod(modH / 60.0f, 2) - 1));
    float m = v - C;
    float r = modH < 60.0f ? C : modH < 120.0f ? X : modH < 180.0f ? 0 : modH < 240.0f ? X : modH < 300.0f ? C : 0;
    float g = modH < 60.0f ? X : modH < 120.0f ? C : modH < 180.0f ? C : modH < 240.0f ? X : modH < 300.0f ? 0 : X;
    float b = modH < 60.0f ? 0 : modH < 120.0f ? 0 : modH < 180.0f ? X : modH < 240.0f ? C : modH < 300.0f ? C : X;
    return Color(r + m, g + m, b + m, a);
}

Color Color::HSV(glm::vec3 hsv)
{
    return HSV(hsv.r, hsv.g, hsv.b);
}

Color Color::HSV(glm::vec4 hsva)
{
    return HSV(hsva.r, hsva.g, hsva.b, hsva.a);
}

Color Color::HSL(float h, float s, float l, float a)
{
    float modH = fmod(h, 360.0f);
    float C = (1 - abs(2 * l - 1)) * s;
    float X = C * (1 - abs(fmod(modH / 60.0f, 2) - 1));
    float m = l - C / 2;
    float r = modH < 60.0f ? C : modH < 120.0f ? X : modH < 180.0f ? 0 : modH < 240.0f ? X : modH < 300.0f ? C : 0;
    float g = modH < 60.0f ? X : modH < 120.0f ? C : modH < 180.0f ? C : modH < 240.0f ? X : modH < 300.0f ? 0 : X;
    float b = modH < 60.0f ? 0 : modH < 120.0f ? 0 : modH < 180.0f ? X : modH < 240.0f ? C : modH < 300.0f ? C : X;
    return Color(r + m, g + m, b + m, a);
}

Color Color::HSL(glm::vec3 hsl)
{
    return HSL(hsl.r, hsl.g, hsl.b);
}

Color Color::HSL(glm::vec4 hsla)
{
    return HSL(hsla.r, hsla.g, hsla.b, hsla.a);
}

glm::vec3 Color::getRGB() const
{
    return glm::vec3(r, g, b);
}

glm::vec3 Color::getHSV() const
{
    float maxVal = std::max({r, g, b});
    float minVal = std::min({r, g, b});
    float delta = maxVal - minVal;
    float h = 0.0f, s = 0.0f, v = maxVal;

    if (delta > 0.00001f) {
        s = delta / maxVal;

        if (maxVal == r) {
            h = 60.0f * (fmod(((g - b) / delta), 6.0f));
        } else if (maxVal == g) {
            h = 60.0f * (((b - r) / delta) + 2.0f);
        } else { // maxVal == b
            h = 60.0f * (((r - g) / delta) + 4.0f);
        }
        
        if (h < 0.0f) {
            h += 360.0f;
        }
    }
    
    return glm::vec3(h, s, v);
}

glm::vec3 Color::getHSL() const
{
    float maxVal = std::max({r, g, b});
    float minVal = std::min({r, g, b});
    float delta = maxVal - minVal;
    float h = 0.0f, s = 0.0f, l = (maxVal + minVal) * 0.5f;

    if (delta > 0.00001f) {
        s = delta / (1.0f - std::abs(2.0f * l - 1.0f));
        
        if (maxVal == r) {
            h = 60.0f * (fmod(((g - b) / delta), 6.0f));
        } else if (maxVal == g) {
            h = 60.0f * (((b - r) / delta) + 2.0f);
        } else { // maxVal == b
            h = 60.0f * (((r - g) / delta) + 4.0f);
        }
        
        if (h < 0.0f) {
            h += 360.0f;
        }
    } else {
        s = 0.0f; // If delta is zero, it's a shade of gray, so no saturation
    }

    return glm::vec3(h, s, l);
}

constexpr float linearToSRGBVal(float value)
{
    return value <= 0.04045 ? value / 12.92 : glm::pow((value + 0.055) / 1.055, 2.4);
}

Color Color::linearToSRGB(Color color)
{
    return Color(linearToSRGBVal(color.r), linearToSRGBVal(color.g), linearToSRGBVal(color.b), color.a);
}

constexpr float sRGBToLinearVal(float value)
{
    return value <= 0.0031308 ? value * 12.92 : 1.055 * glm::pow(value, 1 / 2.4) - 0.055;
}

Color Color::sRGBToLinear(Color color)
{
    return Color(sRGBToLinearVal(color.r), sRGBToLinearVal(color.g), sRGBToLinearVal(color.b), color.a);
}

Color Color::white = Color(1.0f, 1.0f, 1.0f, 1.0f);
Color Color::gray = Color(0.5f, 0.5f, 0.5f, 1.0f);
Color Color::black = Color(0.0f, 0.0f, 0.0f, 1.0f);
Color Color::red = Color(1.0f, 0.0f, 0.0f, 1.0f);
Color Color::yellow = Color(1.0f, 1.0f, 0.0f, 1.0f);
Color Color::green = Color(0.0f, 1.0f, 0.0f, 1.0f);
Color Color::cyan = Color(0.0f, 1.0f, 1.0f, 1.0f);
Color Color::blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
Color Color::magenta = Color(1.0f, 0.0f, 1.0f, 1.0f);
Color Color::clear = Color(1.0f, 1.0f, 1.0f, 0.0f);
Color Color::zero = Color(0.0f, 0.0f, 0.0f, 0.0f);
} // namespace graphics