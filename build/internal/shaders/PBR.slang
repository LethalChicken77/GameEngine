import utils;
import shading_models;
import colorspace; // TODO: Perform colorspace transforms in post-processing

import shaderInputs;
import "vertex/vertex_condensed.slang";
import "vertex/instance.slang";

// Material Descriptor Set
struct MaterialInfo
{
    float4 color;
    float normalMapStrength;
    float2 UVOffset = float2(0,0);
    float2 UVScale = float2(1,1);
};
[[vk::binding(0, 2)]] ConstantBuffer<MaterialInfo> materialInfo;
[[vk::binding(1, 2)]] Sampler2D albedoMap; // SRGB
[[vk::binding(2, 2)]] Sampler2D roughnessMap; // Linear
[[vk::binding(3, 2)]] Sampler2D metallicMap; // Linear
[[vk::binding(4, 2)]] Sampler2D specularMap; // Linear
[[vk::binding(5, 2)]] Sampler2D normalMap; // Linear
[[vk::binding(6, 2)]] Sampler2D skyboxTexture; // SRGB (Temporary)

[shader("vertex")]
VOut vsMain(VertexData vertex, InstanceData instance)
{
    VOut output;
    // float4x4 model = transpose(float4x4(instance.Model0, instance.Model1, instance.Model2, instance.Model3));
    float4x4 model = transpose(instance.Model);
    output.FragPosition = mul(cameraData.viewProj, mul(model, float4(vertex.Position, 1.0)));
    output.Position = mul(model, float4(vertex.Position, 1.0)).xyz;

    float3 rawNormal = UnpackOctahedral(vertex.Normal);
    float3 rawTangent = UnpackOctahedral(vertex.Tangent.xy);
    float3x3 normalMatrix = transpose(adjugate(float3x3(model)));
    output.Normal = normalize(mul(normalMatrix, rawNormal));
    output.Tangent = float4(normalize(mul(normalMatrix, rawTangent.xyz)), vertex.Tangent.z);
    output.Bitangent = normalize(mul(normalMatrix, cross(rawNormal.xyz, rawTangent.xyz)));
    output.Color = vertex.Color;
    output.UV = vertex.UV * materialInfo.UVScale + materialInfo.UVOffset;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(FIn input)
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
    float2 UV = input.UV;

    // float3 albedo = SRGBToLinear(input.Color * materialInfo.color.xyz * albedoMap.Sample(UV).xyz);
    float3 albedo = albedoMap.Sample(UV).xyz;
    float roughness = roughnessMap.Sample(UV).x; // Assume linear texture
    float metallic = saturate(metallicMap.Sample(UV).x * 1.2);
    // float specular = 0.05;//specularMap.Sample(UV).x;
    float specular = specularMap.Sample(UV).x;
    // specular = specular * specular;
    specular = specular * 0.1;
    float3 normalMapSample = normalMap.Sample(UV).xyz * 2.0 - 1.0;

    normalMapSample *= float3(1, 1, 1 / materialInfo.normalMapStrength);
    normalMapSample = normalize(normalMapSample);
    normal = normalize(
        normalMapSample.x * tangent + 
        normalMapSample.y * bitangent * input.Tangent.w + 
        normalMapSample.z * normal);
        
    // if (dot(normal, viewDir) > 0)
    // {
    //     normal = normalize(normal - viewDir * dot(normal, viewDir));
    // }

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
    // outColor = PhongBRDF(albedo, 80, lightColor, ambientColor, normal, lightDir, viewDir);

    float depth = length(input.Position - cameraPos);
    float fog = pow(2.0, -depth * 0.0001);

    float3 fogColor = ambientColor * 4;
    outColor = lerp(fogColor, outColor, fog);

    // float3 luma = luminance(outColor);
    // float3 chroma = outColor / luma;
    // outColor = round(outColor * 16) / 16;
    // outColor *= chroma;

    // float luma = luminance(outColor);
    // float3 chroma = outColor / luma;
    // float steps = 8;
    // luma = pow(round(sqrt(luma) * steps) / steps, 2.0);
    // float chromaSteps = 3;
    // chroma = pow(round(sqrt(chroma) * chromaSteps) / chromaSteps, 2.0);
    // outColor = chroma * luma;

    // outColor = chroma * 0.25;

    // outColor = outColor / luminance(outColor) * 0.25;
    // outColor = outColor / (outColor.x + outColor.y + outColor.z) * 0.25;
    // outColor = float3(luminance(outColor));
    // outColor = albedo;
    return float4(outColor, 1.0);
    // return float4(normalMap.Sample(UV).xyz, 1.0);
}