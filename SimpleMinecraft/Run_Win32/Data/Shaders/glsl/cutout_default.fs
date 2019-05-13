#version 420 core

layout(binding=3, std140) uniform MaterialPropertyBlock{
    float specularStrength;
	float specularPower;
	vec2 pad01;
    vec4 pengbaba;
};

layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec2 passUV; 
in vec4 passColor; 
out vec4 outColor; 

void main()
{
   vec4 tex_color = texture( gTexDiffuse, passUV ); 
   vec4 final_color = tex_color * passColor;

   if (final_color.a <= .01f) {
      discard; 
   }
   outColor = final_color; 
}