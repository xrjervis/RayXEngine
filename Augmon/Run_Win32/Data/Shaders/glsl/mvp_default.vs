#version 420 core

layout(binding=1, std140) uniform uboCamera{
    mat4 VIEW;
    mat4 PROJECTION;
    vec3 CAMERA_POSITION;
    float pad00;
};

uniform mat4 MODEL;

in vec3 POSITION;
in vec4 COLOR;       // NEW - GLSL will use a Vector4 for this;
in vec2 UV;

out vec2 passUV;
out vec4 passColor;  // NEW - to use it in the pixel stage, we must pass it.

void main()
{
    vec4 localPos = vec4( POSITION, 1.0f );  

    vec4 worldPos = localPos * MODEL; 
    vec4 cameraPos = worldPos * transpose(VIEW); 
    vec4 clipPos = cameraPos * transpose(PROJECTION); 

   passColor = COLOR; // pass it on.
   passUV = UV;
   gl_Position = clipPos;
}
