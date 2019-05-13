#version 420 core

// Suggest always manually setting bindings - again, consitancy with 
// other rendering APIs and well as you can make assumptions in your
// engine without having to query
layout(binding = 0) uniform samplerCube gTexSky;

in vec3 passWorldPosition; 
in vec4 passColor; 

out vec4 outColor; 

void main( void )
{
   vec3 normal = normalize(passWorldPosition); 
   vec4 tex_color = texture( gTexSky, normal ); 

   outColor = tex_color * passColor;
}
