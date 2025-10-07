import utils;

struct PushConstants
{
    float4x4 model;
    int objectID;
};
[[vk::push_constant]]
PushConstants pushConstants;

// Camera Descriptor Set
struct CameraUbo
{
    float4x4 view;
    float4x4 invView;
    float4x4 projection;
    float4x4 viewProj;
};
[[vk::binding(0, 0)]] ConstantBuffer<CameraUbo> cameraData;

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
}

[shader("vertex")]
VOut vsMain(Vertex input)
{
    VOut output;
    float4x4 model = transpose(pushConstants.model);
    output.FragPosition = mul(transpose(cameraData.viewProj), mul(model, float4(input.Position, 1.0)));

    return output;
}

// Entry point
[shader("fragment")]
int4 fsMain(VOut input)
{
    return int4(pushConstants.objectID, 0, 0, 0);
}