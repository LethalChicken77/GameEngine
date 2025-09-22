import utils;

// --- Push constants ---
struct PushConstants
{
    float4x4 model;
};
[[vk::push_constant]]
PushConstants pushConstants;

// --- Descriptor set 0: Camera ---
[[vk::binding(0, 0)]]
struct CameraUbo
{
    float4x4 view;
    float4x4 invView;
    float4x4 projection;
    float4x4 viewProj;
};
uniform CameraUbo cameraData;

// --- Descriptor set 1: Material ---
[[vk::binding(0, 1)]]
struct MaterialInfo
{
    float3 color;
};
uniform MaterialInfo materialInfo;

// Constants
static const float PI      = 3.14159265359;
static const float INV_PI  = 1.0 / PI;
static const float EPSILON = 0.0001;

struct Vertex
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
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

    float3x3 normalMatrix = transpose(inverse(float3x3(pushConstants.model)));
    output.Normal = normalize(mul(normalMatrix, input.Normal));
    output.Normal = input.Normal;
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    float3 outColor = input.Color * materialInfo.color;
    return float4(outColor, 1.0);
}