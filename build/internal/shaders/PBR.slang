import utils;
import shading_models;
import colorspace; // TODO: Perform colorspace transforms in post-processing

import shaderInputs;

// Material Descriptor Set
struct MaterialInfo
{
    float4 color;
    float normalMapStrength;
};
[[vk::binding(0, 2)]] ConstantBuffer<MaterialInfo> materialInfo;
[[vk::binding(1, 2)]] Sampler2D albedoMap;
[[vk::binding(2, 2)]] Sampler2D roughnessMap;
[[vk::binding(3, 2)]] Sampler2D metallicMap;
[[vk::binding(4, 2)]] Sampler2D specularMap;
[[vk::binding(5, 2)]] Sampler2D normalMap;
[[vk::binding(6, 2)]] Sampler2D skyboxTexture;

struct Vertex
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
    float2 UV : TEXCOORD0;
};

struct VOut
{
    float4 FragPosition : SV_POSITION;
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
    float2 UV : TEXCOORD0;
};

[shader("vertex")]
VOut vsMain(Vertex input)
{
    VOut output;
    output.FragPosition = mul(transpose(cameraData.viewProj), mul(transpose(pushConstants.model), float4(input.Position, 1.0)));
    output.Position = mul(transpose(pushConstants.model), float4(input.Position, 1.0)).xyz;
    
    float3x3 normalMatrix = transpose(inverse(float3x3(pushConstants.model)));
    output.Normal = normalize(mul(normalMatrix, input.Normal));
    output.Tangent = float4(normalize(mul(normalMatrix, input.Tangent.xyz)), input.Tangent.w);
    output.Bitangent = normalize(mul(normalMatrix, input.Bitangent));
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    float3 normal = normalize(input.Normal);
    float3 tangent = normalize(input.Tangent.xyz);
    float3 bitangent = normalize(input.Bitangent);

    float3 lightDir = normalize(float3(0.5, 0.4, 1));
    float3 cameraPos = cameraData.view[3].xyz;
    float3 viewDir = normalize(input.Position.xyz - cameraPos);
    float3 halfDir = 0.5 * (lightDir - viewDir);
    float3 lightColor = float3(1.0, 1.0, 1.0) * 4.0;
    float3 ambientColor = globalData.ambientLight;//invLinearToSRGB(float3(0.04f, 0.08f, 0.2f));
    float2 UV = input.UV * 10;

    float3 albedo = SRGBToLinear(input.Color * materialInfo.color.xyz * albedoMap.Sample(UV).xyz);
    float roughness = roughnessMap.Sample(UV).x;
    float metallic = saturate(metallicMap.Sample(UV).x * 1.2);
    float specular = 0.1;//specularMap.Sample(UV).x;
    float3 normalMapSample = normalMap.Sample(UV).xyz * 2.0 - 1.0;

    normalMapSample *= float3(materialInfo.normalMapStrength, materialInfo.normalMapStrength, 1);
    normalMapSample = normalize(normalMapSample);
    normal = normalize(
        normalMapSample.x * tangent + 
        normalMapSample.y * -bitangent * input.Tangent.w + 
        normalMapSample.z * normal);

    float3 reflectDir = reflect(viewDir, normal);
    float2 sphericalCoords = cartesianToSphericalNorm(reflectDir);
    float3 reflColor = skyboxTexture.Sample(sphericalCoords).xyz;

    float3 outColor = float3(0);
    for (int i = 0; i < globalData.numLights; i++)
    {
        Light currentLight = globalData.lights[i];
        float3 lightDir = float3(0);
        float attenuation = 1;
        if (currentLight.type == LightType.DIRECTIONAL)
            lightDir = normalize(currentLight.position);
        else
        {
            lightDir = currentLight.position - input.Position;
            attenuation = 1 / (lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
            lightDir = normalize(lightDir);
        }

        float3 halfDir = 0.5 * (lightDir - viewDir);

        float3 lightColor = currentLight.color * currentLight.intensity * attenuation;
        lightColor = lightColor;
        outColor += pbrBRDF(albedo, roughness, metallic, specular, lightColor, normal, lightDir, viewDir);
    }
    outColor += pbrAmbient(albedo, roughness, metallic, specular, ambientColor, ambientColor, normal, viewDir);
    // float3 outColor = PhongBRDF(albedo, 80, lightColor, ambientColor, normal, lightDir, viewDir);

    return float4(outColor, 1.0);
}