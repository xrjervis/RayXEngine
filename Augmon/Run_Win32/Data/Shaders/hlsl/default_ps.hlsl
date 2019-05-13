#include "common.hlsli"

float4 PSMain(in PS_INPUT input) : SV_TARGET{
    float4 final = float4(1.f, 0.f, 0.f, 1.f);
    float4 diffuseTextureColor = g_texDiffuse.Sample(g_texDiffuseSampler, input.texCoords);
    final = diffuseTextureColor * input.color;
    return final;
}