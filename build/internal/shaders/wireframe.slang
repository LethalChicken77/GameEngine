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
VOut vsMain(VertexData vertex, InstanceData instance)
{
    VOut output;
    float4x4 model = transpose(float4x4(instance.Model0, instance.Model1, instance.Model2, instance.Model3));
    output.FragPosition = mul(cameraData.viewProj, mul(model, float4(vertex.Position, 1.0)));
    output.Position = mul(model, float4(vertex.Position, 1.0)).xyz;

    float3x3 normalMatrix = inverse(transpose(float3x3(model)));
    output.Normal = normalize(mul(normalMatrix, vertex.Normal));
    output.Color = vertex.Color;
    output.UV = vertex.UV;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    float3 outColor = input.Color * materialInfo.color.xyz;
    outColor = SRGBToLinear(outColor);
    return float4(outColor, 1.0);
}