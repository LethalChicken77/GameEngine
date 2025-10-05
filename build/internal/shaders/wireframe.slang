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


struct Vertex
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
    float2 UV : TEXCOORD0;
}

struct VOut
{
    float4 FragPosition : SV_POSITION;
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Color : COLOR;
    float2 UV : TEXCOORD0;
}

[shader("vertex")]
VOut vsMain(Vertex input)
{
    VOut output;
    output.FragPosition = mul(transpose(cameraData.viewProj), mul(transpose(pushConstants.model), float4(input.Position, 1.0)));
    output.Position = mul(transpose(pushConstants.model), float4(input.Position, 1.0)).xyz;

    float3x3 normalMatrix = inverse(transpose(float3x3(pushConstants.model)));
    output.Normal = normalize(mul(normalMatrix, input.Normal));
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    float3 outColor = input.Color * materialInfo.color.xyz;
    outColor = LinearToSRGB(outColor);
    return float4(outColor, 1.0);
}