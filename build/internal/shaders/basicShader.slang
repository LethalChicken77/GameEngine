import utils;
import shading_models;
import colorspace; // TODO: Perform colorspace transforms in post-processing
import constants;

// --- Push constants ---
struct PushConstants
{
    float4x4 model;
};
[[vk::push_constant]]
PushConstants pushConstants;

// --- Descriptor set 0: Camera ---
struct CameraUbo
{
    float4x4 view;
    float4x4 invView;
    float4x4 projection;
    float4x4 viewProj;
};
[[vk::binding(0, 0)]]
ConstantBuffer<CameraUbo> cameraData;

// --- Descriptor set 1: Material ---
struct MaterialInfo
{
    float4 color;
    float roughness;
    float metallic;
};
[[vk::binding(0, 1)]] ConstantBuffer<MaterialInfo> materialInfo;

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
    float4x4 model = transpose(pushConstants.model);
    output.FragPosition = mul(transpose(cameraData.viewProj), mul(model, float4(input.Position, 1.0)));
    output.Position = mul(model, float4(input.Position, 1.0)).xyz;

    float3x3 normalMatrix = transpose(inverse(float3x3(model)));
    output.Normal = normalize(mul(normalMatrix, input.Normal));
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(float3(0.5, 0.4, 1));
    float3 cameraPos = cameraData.view[3].xyz;
    float3 viewDir = normalize(input.Position.xyz - cameraPos);
    float3 halfDir = 0.5 * (lightDir - viewDir);
    float3 lightColor = float3(1.0, 1.0, 1.0) * 4.0;
    float3 ambientColor = invGamma_srgb(float3(0.04f, 0.08f, 0.2f));

    float3 albedo = input.Color * materialInfo.color.xyz;
    float3 F0 = float3(0.04f); // Typical for dielectrics

    float3 outColor = pbrBRDF(albedo, materialInfo.roughness, materialInfo.metallic, F0, lightColor, ambientColor, ambientColor, normal, lightDir, viewDir);
    // float3 outColor = PhongBRDF(albedo, 80, lightColor, ambientColor, normal, lightDir, viewDir);

    outColor = gamma_srgb(outColor); // Gamma correction
    return float4(outColor, 1.0);
}