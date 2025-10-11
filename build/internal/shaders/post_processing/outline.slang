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

static const float[] kernel = {0.0276305506, 0.0662822453, 0.1238315368, 0.1801738229, 0.2041636887, 0.1801738229, 0.1238315368, 0.0662822453, 0.0276305506};

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    uint width, height;
    base.GetDimensions(width, height);
    float4 baseSample = base.Sample(input.UV);
    float x_base = input.Position.x;
    float y_base = input.Position.y;
    float sample = 0;
    float edgeSample = 0;
    float accum = 0;
    bool nearEdge = false;
    for (int y = -4; y < 5; y++)
    {
        for (int x = -4; x < 5; x++)
        {
            float2 samplePos = float2((x_base + x) / (float)width, (y_base + y) / (float)height);
            float weight = kernel[x + 4] * kernel[y + 4];
            accum += weight;
            if (samplePos.x < 0 || samplePos.x > 1 || samplePos.y < 0 || samplePos.y > 1)
            {
                nearEdge = true;
                edgeSample += weight;
                continue;
            }
            sample += base.Sample(samplePos).x * weight;
            // ySample += base.Sample(ySamplePosition).x;
        }
    }
    float blurredSample = sample / accum; //(xSample + ySample) / accum;
    float edgeBlur = edgeSample / accum;
    // float alpha = blurredSample;
    float alpha = saturate(blurredSample * 5) - baseSample.x;
    if (nearEdge)
        alpha += edgeSample * 5 * baseSample.x;

    return float4(1.0, 0.5, 0.0, alpha);
    // return float4(alpha, 0.0, 0.0, 1.0);
}