#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 normal;
layout (location = 4) in vec4 tangent;

out Vertex {
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
    vec3 tangent;
	vec3 bitangent;
} OUT;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vec3 oNormal = normalize(normalMatrix * normalize(normal));
	vec3 oTangent = normalize(normalMatrix * normalize(tangent.xyz));
	vec3 oBiTangent = cross(oTangent, oNormal) * tangent.w;

    OUT.texCoord = texCoord;
    OUT.worldPos = vec3(modelMatrix * vec4(position, 1.0));
    OUT.normal = oNormal;
    OUT.tangent = oTangent;
	OUT.bitangent = oBiTangent;

    gl_Position =  projMatrix * viewMatrix * vec4(OUT.worldPos, 1.0);
} 