#include "skybox_common.hlsli"

float4 PSMain(in PS_INPUT input) : SV_TARGET{
    return g_texCube.Sample(g_texCubeSampler, input.worldPosition.xyz);
}