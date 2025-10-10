import utils;
import shading_models;
import colorspace;

import shaderInputs;

// Material Descriptor Set
struct MaterialInfo
{
    float filler;
};
[[vk::binding(0, 2)]] ConstantBuffer<MaterialInfo> materialInfo;

[shader("vertex")]
VOut vsMain(VertexData vertex, InstanceData instance)
{
    VOut output;
    float4x4 model = transpose(float4x4(instance.Model0, instance.Model1, instance.Model2, instance.Model3));
    output.FragPosition = mul(cameraData.viewProj, mul(model, float4(vertex.Position, 1.0)));
    output.Position = mul(model, float4(vertex.Position, 1.0)).xyz;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    return float4(1.0, 0.0, 0.0, 0.0);
}