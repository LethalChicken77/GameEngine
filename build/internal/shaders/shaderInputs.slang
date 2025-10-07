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

// Global Descriptor Set
enum LightType
{
    DIRECTIONAL = 0,
    POINT = 1
};
struct Light
{
    float3 position;
    int type;
    float3 color;
    float intensity;
};
struct GlobalData
{
    float3 ambientLight;
    int numLights;
    Light lights[128];
};
[vk::binding(0, 1)] ConstantBuffer<GlobalData> globalData;