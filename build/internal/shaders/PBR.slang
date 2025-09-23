import utils;
import shading_models;
import colorspace; // TODO: Perform colorspace transforms in post-processing

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
[[vk::binding(0, 0)]] ConstantBuffer<CameraUbo> cameraData;

// --- Descriptor set 1: Material ---
struct MaterialInfo
{
    float3 color;
};
[[vk::binding(0, 1)]] ConstantBuffer<MaterialInfo> materialInfo;
[[vk::binding(1, 1)]] Sampler2D albedoMap;
[[vk::binding(2, 1)]] Sampler2D roughnessMap;
[[vk::binding(3, 1)]] Sampler2D metallicMap;
[[vk::binding(4, 1)]] Sampler2D specularMap;
[[vk::binding(5, 1)]] Sampler2D normalMap;
[[vk::binding(6, 1)]] Sampler2D skyboxTexture;


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
    float3 normal = normalize(input.Normal);
    float3 tangent = normalize(float3(1, 0, 0));
    float3 bitangent = normalize(float3(0, 1, 0));

    float3 lightDir = normalize(float3(1, 1, 1));
    float3 cameraPos = cameraData.view[3].xyz;
    float3 viewDir = normalize(input.Position.xyz - cameraPos);
    float3 halfDir = 0.5 * (lightDir - viewDir);
    float3 lightColor = float3(1.0, 1.0, 1.0) * 4.0;
    float3 ambientColor = invGamma_srgb(float3(0.04f, 0.08f, 0.2f));
    float2 UV = input.UV * 3;

    float3 albedo = invGamma_srgb(input.Color * materialInfo.color * albedoMap.Sample(UV).xyz);
    float roughness = roughnessMap.Sample(UV).x;
    float metallic = metallicMap.Sample(UV).x;
    float specular = specularMap.Sample(UV).x;
    float3 normalMapSample = normalMap.Sample(UV).xyz * 2.0 - 1.0;
    normal = normalize(normalMapSample.x * tangent + normalMapSample.y * bitangent + normalMapSample.z * normal);

    float3 reflectDir = reflect(viewDir, normal);
    float theta = -atan2(reflectDir.z, reflectDir.x) / (2.0 * PI) + 0.5;
    float phi = acos(reflectDir.y) / PI;
    float3 reflColor = skyboxTexture.Sample(float2(theta, phi)).xyz;

    float3 outColor = pbrBRDF(albedo, roughness, metallic, 0.1, lightColor, ambientColor, reflColor, normal, lightDir, viewDir);
    // float3 outColor = PhongBRDF(albedo, 80, lightColor, ambientColor, normal, lightDir, viewDir);



    outColor = gamma_srgb(outColor); // Gamma correction
    return float4(outColor, 1.0);
}