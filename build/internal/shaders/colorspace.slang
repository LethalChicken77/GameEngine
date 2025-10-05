module colorspace;

float LinearToSRGB(float value)
{
    if (value <= 0.04045)
        return value / 12.92;
    else
        return pow((value + 0.055) / 1.055, 2.4);
}

public float3 LinearToSRGB(float3 color)
{
    float3 outColor;
    outColor.x = LinearToSRGB(color.x);
    outColor.y = LinearToSRGB(color.y);
    outColor.z = LinearToSRGB(color.z);
    return outColor;
}

float SRGBToLinear(float value)
{
    if (value <= 0.0031308)
        return 12.92 * value;
    else
        return 1.055 * pow(value, 1 / 2.4) - 0.055;
}

public float3 SRGBToLinear(float3 color)
{
    float3 outColor;
    outColor.x = SRGBToLinear(color.x);
    outColor.y = SRGBToLinear(color.y);
    outColor.z = SRGBToLinear(color.z);
    return outColor;
}

// sRGB Linear -> ACEScg
public float3 LinearToACEScg(const float3 c)
{
    const float3x3 M = {
        { 0.59719f, 0.35458f, 0.04823f },
        { 0.07600f, 0.90834f, 0.01566f },
        { 0.02840f, 0.13383f, 0.83777f }
    };
    return float3(
        M[0][0] * c.x + M[0][1] * c.y + M[0][2] * c.z,
        M[1][0] * c.x + M[1][1] * c.y + M[1][2] * c.z,
        M[2][0] * c.x + M[2][1] * c.y + M[2][2] * c.z
    );
}

// ACEScg -> sRGB Linear (for display)
public float3 ACEScgToLinear(const float3 c)
{
    const float3x3 M_inv = {
        { 1.60475f, -0.53108f, -0.07367f },
        { -0.10208f, 1.10813f, -0.00605f },
        { -0.00327f, -0.07276f, 1.07603f }
    };
    return float3(
        M_inv[0][0] * c.x + M_inv[0][1] * c.y + M_inv[0][2] * c.z,
        M_inv[1][0] * c.x + M_inv[1][1] * c.y + M_inv[1][2] * c.z,
        M_inv[2][0] * c.x + M_inv[2][1] * c.y + M_inv[2][2] * c.z
    );
}

// Linear RGB -> Oklab
public float3 LinearToOklab(float3 c)
{
    // LMS conversion
    float l = 0.4121656120 * c.x + 0.5362752080 * c.y + 0.0514575653 * c.z;
    float m = 0.2118591070 * c.x + 0.6807189584 * c.y + 0.1074065790 * c.z;
    float s = 0.0883097947 * c.x + 0.2818474174 * c.y + 0.6294548110 * c.z;

    // Cube root
    float l_ = pow(l, 1.0 / 3.0);
    float m_ = pow(m, 1.0 / 3.0);
    float s_ = pow(s, 1.0 / 3.0);

    // Oklab
    return float3(
        0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
        1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
        0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_
    );
}

// Oklab -> Linear RGB
public float3 OklabToLinear(float3 c)
{
    // Inverse LMS
    float l_ = c.x + 0.3963377774 * c.y + 0.2158037573 * c.z;
    float m_ = c.x - 0.1055613458 * c.y - 0.0638541728 * c.z;
    float s_ = c.x - 0.0894841775 * c.y - 1.2914855480 * c.z;

    // Cube
    l_ = l_ * l_ * l_;
    m_ = m_ * m_ * m_;
    s_ = s_ * s_ * s_;

    // Back to linear RGB
    return float3(
        4.0767416621 * l_ - 3.3077115913 * m_ + 0.2309699292 * s_,
        -1.2684380046 * l_ + 2.6097574011 * m_ - 0.3413193965 * s_,
        -0.0041960863 * l_ - 0.7034186147 * m_ + 1.7076147010 * s_
    );
}

// Simple ACES Filmic tone mapping approximation
public float3 ACESFilmCurve(float3 color)
{
    // Approximate ACES RRT+ODT using Uncharted 2 / Jim Hejl filmic curve
    float A = 2.51;
    float B = 0.03;
    float C = 2.43;
    float D = 0.59;
    float E = 0.14;

    color = (color * (A * color + B)) / (color * (C * color + D) + E);

    // Clamp to [0,1] for display
    return saturate(color);
}