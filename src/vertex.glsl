#version 430 core

layout (location = 0) in vec3 aPos;

out vec2 fragPos;

void main() {
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	fragPos = aPos.xy;
}