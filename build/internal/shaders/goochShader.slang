import utils;
import shading_models;
import colorspace; // TODO: Perform colorspace transforms in post-processing

import shaderInputs;

// Material Descriptor Set
struct MaterialInfo
{
    float4 coolColor;
    float4 warmColor;
    float4 outlineColor;
    float outlinePower;
    float roughness;
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
    float4x4 model = transpose(pushConstants.model);
    output.FragPosition = mul(transpose(cameraData.viewProj), mul(model, float4(input.Position, 1.0)));
    output.Position = mul(model, float4(input.Position, 1.0)).xyz;

    float3x3 normalMatrix = transpose(inverse(float3x3(model)));
    output.Normal = normalize(mul(normalMatrix, input.Normal));

    float3 cameraPos = cameraData.view[3].xyz;
    float3 viewDir = normalize(input.Position.xyz - cameraPos);
    if (dot(output.Normal, viewDir) > 0)
    {
        output.Normal = normalize(output.Normal - viewDir * dot(output.Normal, viewDir));
    }
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    float3 normal = normalize(input.Normal);
    // float3 lightDir = normalize(float3(0.5, 0.4, 1));
    float3 cameraPos = cameraData.view[3].xyz;
    float3 viewDir = normalize(input.Position.xyz - cameraPos);
    // float3 lightColor = float3(1.0, 1.0, 1.0) * 4.0;
    float3 ambientColor = globalData.ambientLight;// = invLinearToSRGB(float3(0.04f, 0.08f, 0.2f));

    float3 F0 = float3(0.05f);

    Light currentLight = globalData.lights[0];
    float3 lightDir = normalize(currentLight.position);
    float3 halfDir = 0.5 * (lightDir - viewDir);

    float3 lightColor = currentLight.color * currentLight.intensity;
    float3 coolColor = SRGBToLinear(materialInfo.coolColor.xyz);
    float3 warmColor = SRGBToLinear(materialInfo.warmColor.xyz);
    float4 outlineColor = float4(SRGBToLinear(materialInfo.outlineColor.xyz), materialInfo.outlineColor.w);//float3(1.0, 0.6, 0.1);
    float outlinePower = materialInfo.outlinePower;
    float3 outColor = goochBRDF(coolColor, warmColor, outlineColor, outlinePower, materialInfo.roughness, F0, lightColor, normal, lightDir, viewDir);
    // float3 outColor = PhongBRDF(albedo, 80, lightColor, ambientColor, normal, lightDir, viewDir);

    // outColor = OklabToLinear(outColor);
    // outColor = pow(outColor, 2.0);
    // outColor = ACEScgToLinear(outColor);
    // outColor = LinearToSRGB(outColor); // Gamma correction
    // outColor *= 0.5;
    // outColor = ACESFilmCurve(outColor);
    return float4(outColor, 1.0);
}