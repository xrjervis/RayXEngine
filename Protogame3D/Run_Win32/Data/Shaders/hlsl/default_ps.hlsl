#include "common.hlsli"

float4 PSMain(in PS_INPUT input) : SV_TARGET{
    float4 diffuseTextureColor = g_texDiffuse.Sample(g_texDiffuseSampler, input.texCoords);
    float4 final = diffuseTextureColor * input.color;

    return final;
}