module colorspace;

float gamma_srgb(float value)
{
    if (value <= 0.04045)
        return value / 12.92;
    else
        return pow((value + 0.055) / 1.055, 2.4);
}

public float3 gamma_srgb(float3 color)
{
    float3 outColor;
    outColor.x = gamma_srgb(color.x);
    outColor.y = gamma_srgb(color.y);
    outColor.z = gamma_srgb(color.z);
    return outColor;
}

float invGamma_srgb(float value)
{
    if (value <= 0.0031308)
        return 12.92 * value;
    else
        return 1.055 * pow(value, 1 / 2.4) - 0.055;
}

public float3 invGamma_srgb(float3 color)
{
    float3 outColor;
    outColor.x = invGamma_srgb(color.x);
    outColor.y = invGamma_srgb(color.y);
    outColor.z = invGamma_srgb(color.z);
    return outColor;
}