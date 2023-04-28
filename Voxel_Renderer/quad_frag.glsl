#version 430 core

out vec4 FragColor;
  
in vec2 fragPos;

uniform sampler2D u_Texture;

void main() { 
    FragColor = texture(u_Texture, fragPos * 0.5 + 0.5);
}