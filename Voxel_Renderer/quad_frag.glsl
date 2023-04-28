#version 430 core

out vec4 FragColor;
  
in vec2 fragPos;

uniform sampler2D screenTexture;

void main() { 
    FragColor = texture(screenTexture, fragPos * 0.5 + 0.5);
}