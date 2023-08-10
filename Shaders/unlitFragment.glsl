#version 330 core

out vec4 fragColour;

uniform vec4 lightColor;

void main(void)
{
     fragColour = lightColor;
}