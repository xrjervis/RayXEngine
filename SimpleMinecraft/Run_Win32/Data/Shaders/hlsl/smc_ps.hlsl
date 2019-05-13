#include "smc_common.hlsli"

float4 PSMain(in PS_INPUT input) : SV_TARGET{
    float4 diffuseTextureColor = g_texDiffuse.Sample(g_texDiffuseSampler, input.texCoords);
    float3 indoorLightColor = input.color.x * g_defaultIndoorLightColor + float3(0.05, 0.05, 0.05);
    float3 outdoorLightColor = input.color.y * g_defaultOutdoorLightColor * g_skyColor;

    float4 lightColor = float4(max(indoorLightColor.x, outdoorLightColor.x), max(indoorLightColor.y, outdoorLightColor.y), max(indoorLightColor.z, outdoorLightColor.z), 1.0);

    //float4 worldColor = diffuseTextureColor * input.color;

    float4 worldColor = diffuseTextureColor * lightColor;

    // fog color compute
    float dist = distance(g_eyePos, input.worldPosition.xyz);
	float fogFraction = (dist - g_fogStart) / (g_fogEnd - g_fogStart);
	fogFraction = clamp( fogFraction, 0.0, 1.0 );
//	fogFraction *= fogFraction;
	float3 finalColor = lerp( worldColor.xyz, g_skyColor, fogFraction);
    //TODO: sky colored distance based fog

    return float4(finalColor, 1.0);
}