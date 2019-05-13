// define the shader version(this is required)
#version 420 core

// Attributes = input to this shader stage(constant as afar as the code is concerned)
in vec3 POSITION;
in vec4 COLOR;

out vec4 outColor;

void main(void){
  gl_Position = vec4(POSITION, 1);
  outColor = vec4(COLOR.x / 255, COLOR.y / 255, COLOR.z / 255, COLOR.w / 255);
}
