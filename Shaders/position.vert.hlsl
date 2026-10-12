cbuffer Uniforms : register(b0, space1)
{
    float scale;
}

struct Input
{
    float3 position : TEXCOORD0;
    float4 color : TEXCOORD1;
};

struct Output
{
    float4 Position : SV_Position;
    float4 Color : TEXCOORD0;
};

Output main(Input input)
{
    Output output;
    output.Position = float4(input.position * scale, 1.0f);
    output.Color = input.color;

    return output;
}