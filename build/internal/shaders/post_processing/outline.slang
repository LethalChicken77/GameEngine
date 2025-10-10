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

static const float[] kernel = {0.004433, 0.054006, 0.242036, 0.399050, 0.242036, 0.054006, 0.004433};

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    uint width, height;
    base.GetDimensions(width, height);
    float4 baseSample = base.Sample(input.UV);
    float x = input.Position.x;
    float y = input.Position.y;
    float xSample = 0;
    float ySample = 0;
    for (int i = -3; i < 4; i++)
    {
        float2 xSamplePosition = float2((x + i) / (float)width, y / (float)height);
        float2 ySamplePosition = float2(x / (float)width, (y + i) / (float)height);
        xSample += base.Sample(xSamplePosition).x * kernel[i];
        ySample += base.Sample(ySamplePosition).x * kernel[i];
    }
    float blurredSample = xSample * ySample;
    float alpha = float(saturate(blurredSample) - baseSample.x);
    return float4(1.0, 0.8, 0.0, alpha);
}