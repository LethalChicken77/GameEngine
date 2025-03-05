#include "color.hpp"

namespace graphics
{

Color::Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
Color::Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) : r(_r / 255.0f), g(_g / 255.0f), b(_b / 255.0f), a(_a / 255.0f) {}
Color::Color(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
Color::Color(const glm::vec3 &color) : r(color.r), g(color.g), b(color.b), a(1.0f) {}
Color::Color(const glm::vec4 &color) : r(color.r), g(color.g), b(color.b), a(color.a) {}

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



} // namespace graphics