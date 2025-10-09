import utils;
import constants;
import colorspace;

import shaderInputs;

// Material Descriptor Set
struct MaterialInfo
{
    float4 color;
};
[[vk::binding(0, 2)]] ConstantBuffer<MaterialInfo> materialInfo;

[shader("vertex")]
VOut vsMain(VertexData input, InstanceData instance)
{
    VOut output;
    float4x4 model = transpose(float4x4(instance.Model0, instance.Model1, instance.Model2, instance.Model3));
    output.FragPosition = mul(cameraData.viewProj, mul(model, float4(input.Position, 1.0)));
    output.Position = input.Position.xyz;

    // float3x3 normalMatrix = inverse(transpose(float3x3(pushConstants.model)));
    // output.Normal = normalize(mul(normalMatrix, input.Normal));
    output.Color = input.Color;
    // output.UV = input.UV;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    float3 sunDir = normalize(globalData.lights[0].position);
    float3 skyDir = normalize(input.Position);

    float sunAngle = angleBetween(sunDir, skyDir);
    float3 sunGlow = (1 - saturate(pow(sunAngle / PI, 0.005))) * 30.0 * float3(0.9, 0.9, 0.6);
    float sunDisk = (1 - saturate(pow(sunAngle * 60, 2.0))) * 10.0;

    float2 sphericalCoords = cartesianToSphericalNorm(normalize(input.Position));
    float fudge = 0.01 * Hash(sphericalCoords) - 0.005;

    float3 zenithColor = float3(0.1, 0.15, 0.5);
    float3 horizonColor = float3(0.2, 0.4, 0.8);
    float3 hazeColor = float3(0.85, 0.9, 1);
    float3 groundColor = float3(0.1, 0.1, 0.1);
    float zenithToHorizon = saturate(pow(sphericalCoords.y * 2.0, 2)) + fudge;
    float zenithToHorizonHaze = saturate(pow(sphericalCoords.y * 2.0, 20)) + fudge;
    float downToHorizon = saturate(pow((sphericalCoords.y - 0.5) * 2.0, 0.1)) + fudge;
    float3 result = lerp(lerp(lerp(zenithColor, horizonColor, zenithToHorizon), hazeColor, zenithToHorizonHaze) + sunGlow + float3(sunDisk), groundColor, downToHorizon);

    float3 outColor = result;// LinearToACEScg(result);//gamma_srgb(result);
    return float4(outColor, 1.0);
}