#include "uniforms.hlsli"
 
TextureCube<float4> cubemap_texture : register(t0, FRAGMENT_TEXTURE_SPACE);
SamplerState cubemap_sampler : register(s0, FRAGMENT_TEXTURE_SPACE);
 
struct Input
{
    float3 texcoord : TEXCOORD0;
    float4 position : SV_Position;
};

float4 main(Input input) : SV_Target0
{
    return cubemap_texture.Sample(cubemap_sampler, input.texcoord);
}
