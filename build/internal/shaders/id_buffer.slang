import utils;

struct PushConstants
{
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

struct VertexData
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
    float2 UV : TEXCOORD0;
};

struct InstanceData
{
    float4 Model0 : LOCATION6;
    float4 Model1 : LOCATION7;
    float4 Model2 : LOCATION8;
    float4 Model3 : LOCATION9;
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
VOut vsMain(VertexData vertex, InstanceData instance)
{
    VOut output;
    float4x4 model = transpose(float4x4(instance.Model0, instance.Model1, instance.Model2, instance.Model3));
    output.FragPosition = mul(transpose(cameraData.viewProj), mul(model, float4(vertex.Position, 1.0)));

    return output;
}

// Entry point
[shader("fragment")]
int4 fsMain(VOut input)
{
    return int4(pushConstants.objectID, 0, 0, 0);
}