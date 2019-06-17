#include "skybox_common.hlsli"

PS_INPUT VSMain(in VS_INPUT input){
    PS_INPUT output;
    row_major matrix MVP = mul(mul(g_model, g_view), g_projection);
    output.worldPosition = input.position;
    output.position = mul(input.position, MVP);
    return output;
}