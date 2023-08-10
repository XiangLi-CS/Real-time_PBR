#version 330 core

layout (location = 0) in vec3 position;

out Vertex {
	vec3 worldPos;
} OUT;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main(void)
{ 
   OUT.worldPos = position;
   gl_Position = projMatrix * viewMatrix * vec4(OUT.worldPos, 1.0);
}