import colorspace;


static const float2 vertices[6] = {
    float2(-1, 1),
    float2(-1, -1),
    float2(1, -1),
    float2(-1, 1),
    float2(1, -1),
    float2(1, 1),
};

struct MaterialInfo // Herp derp this shouldn't be here
{
    float filler;
};
[[vk::binding(0, 0)]] ConstantBuffer<MaterialInfo> materialInfo;
[[vk::binding(1, 0)]] Sampler2D base;

struct VIn
{
    uint vertexID : SV_VertexID;
};

struct VOut
{
    float4 Position : SV_Position;
    float2 RealPosition : Position;
    float2 UV : TEXCOORD0;
};

[shader("vertex")]
VOut vsMain(VIn input)
{
    VOut output;
    output.Position = float4(vertices[input.vertexID], 0.5, 1.0);
    output.RealPosition = vertices[input.vertexID];
    output.UV = vertices[input.vertexID] * 0.5 + float2(0.5);
    // output.UV.y = 1 - output.UV.y;
    // output.UV -= float2(0.5);

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    float3 outColor = base.Sample(input.UV).xyz;
    outColor *= 0.5;
    float exposure = 0.0;
    outColor *= pow(2, exposure);
    // outColor = (round(outColor) + round(outColor - 0.25) + round(outColor + 0.25)) / 4;
    outColor = LinearToACEScg(outColor);
    outColor = ACESFilmCurve(outColor);
    // outColor = gamma_srgb(outColor);
    float gamma = 1.2;
    outColor = pow(outColor, gamma);
    return float4(outColor, 1.0);
    // return float4(input.UV.xy, 0.0, 1.0);
}