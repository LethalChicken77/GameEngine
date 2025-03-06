#include <glm/glm.hpp>
#include <string>

namespace graphics
{
    struct Color
    {
        Color();
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
        Color(float r, float g, float b, float a = 1.0f);
        Color(const glm::vec3 &color);
        Color(const glm::vec4 &color);
        
        // Values
        float r{0.0f}, g{0.0f}, b{0.0f};
        float a{1.0f};
        
        // Get color from hex
        static Color hex(uint32_t value);
        static Color hex(const std::string &value);

        // HSV and HSL
        static Color HSV(float h, float s, float v, float a = 1.0f);
        static Color HSV(glm::vec3 hsv);
        static Color HSV(glm::vec4 hsva);

        static Color HSL(float h, float s, float l, float a = 1.0f);
        static Color HSL(glm::vec3 hsl);
        static Color HSL(glm::vec4 hsla);
        
        // Physical
        static Color wavelength(float wavelength);
        static Color blackbody(float temperature);
        static Color IOR(float iorR, float iorG, float iorB);
        static Color IOR(glm::vec3 ior);
        
        // Hardcoded colors
        static Color white;
        static Color black;
        static Color gray;
        static Color red;
        static Color yellow;
        static Color green;
        static Color cyan;
        static Color blue;
        static Color magenta;
        static Color clear;
        static Color zero;
        
        // Getters
        glm::vec3 getRGB() const;
        glm::vec3 getHSV() const;
        glm::vec3 getHSL() const;
        
        // Operators
        Color operator+(const Color &other) const { return Color(r + other.r, g + other.g, b + other.b, a + other.a); };
        Color operator-(const Color &other) const { return Color(r - other.r, g - other.g, b - other.b, a - other.a); };
        Color operator*(const Color &other) const { return Color(r * other.r, g * other.g, b * other.b, a * other.a); };
        Color operator/(const Color &other) const { return Color(r / other.r, g / other.g, b / other.b, a / other.a); };
        Color operator*(float scalar) const { return Color(r * scalar, g * scalar, b * scalar, a * scalar); };
        Color operator/(float scalar) const { return Color(r / scalar, g / scalar, b / scalar, a / scalar); };
        Color operator*(const glm::vec4 &other) const { return Color(r * other.r, g * other.g, b * other.b, a * other.a); };
        Color operator/(const glm::vec4 &other) const { return Color(r / other.r, g / other.g, b / other.b, a / other.a); };
        Color operator*(const glm::vec3 &other) const { return Color(r * other.r, g * other.g, b * other.b, a); };
        Color operator/(const glm::vec3 &other) const { return Color(r / other.r, g / other.g, b / other.b, a); };
        bool operator==(const Color &other) const { return r == other.r && g == other.g && b == other.b && a == other.a; };
        bool operator!=(const Color &other) const { return r != other.r || g != other.g || b != other.b || a != other.a; };
    };

} // namespace graphics