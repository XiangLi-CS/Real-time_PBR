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

   mat4 rotView = mat4(mat3(viewMatrix));
   vec4 clipPos = projMatrix * rotView * vec4(OUT.worldPos, 1.0);

   gl_Position = clipPos.xyww;
}