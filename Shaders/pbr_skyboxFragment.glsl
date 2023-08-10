#version 330 core
out vec4 fragColour;

in Vertex{
    vec3 worldPos;
} IN;

uniform samplerCube environmentMap;

const float GAMMA = 2.2;

void main(void)
{
     vec3 envColor = texture(environmentMap, IN.worldPos).rgb;

     envColor = pow(envColor, vec3(GAMMA));
     envColor = envColor / (envColor + vec3(1.0));
     envColor = pow(envColor, vec3(1.0 / GAMMA));

     fragColour = vec4(envColor, 1.0);
}