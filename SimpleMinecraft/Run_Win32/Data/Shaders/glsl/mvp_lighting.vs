#version 420 core
// Uniforms ==============================================

layout(binding=1, std140) uniform uboCamera{
    mat4 VIEW;
    mat4 PROJECTION;
    vec3 CAMERA_POSITION;
    float pad00;
};
uniform mat4 MODEL;


// Attributes ============================================
in vec3 POSITION;
in vec3 NORMAL;
in vec3 TANGENT;
in vec3 BITANGENT;
in vec4 COLOR;
in vec2 UV;

// Outputs
out vec2 passUV;
out vec4 passColor;
out vec3 passWorldPos;
out mat3 TBN;

// Entry Point ===========================================
void main( void )
{
    vec4 localPos = vec4( POSITION, 1.0f );  

    vec4 worldPos = localPos * MODEL; 
    vec4 cameraPos = worldPos * transpose(VIEW); 
    vec4 clipPos = cameraPos * transpose(PROJECTION); 

    passUV = UV; 
    passColor = COLOR; 

    // new
    passWorldPos = worldPos.xyz;  

    // may use normal matrix to replace model matrix here
    vec3 T = normalize((vec4(TANGENT, 0.0f) * MODEL).xyz);
    vec3 B = normalize((vec4(BITANGENT, 0.0f) * MODEL).xyz);
    vec3 N = normalize((vec4(NORMAL, 0.0f) * MODEL).xyz);
    TBN = mat3(T, B, N);
    gl_Position = clipPos;
}