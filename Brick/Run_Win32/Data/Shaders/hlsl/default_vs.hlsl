#include "common.hlsli"

PS_INPUT VSMain(in VS_INPUT input){
    PS_INPUT output;
    row_major matrix MVP = mul(mul(g_model, g_view), g_projection);
    output.position = mul(input.position, MVP);
    output.color = input.color;
    output.texCoords = input.texCoords;

    return output;
}