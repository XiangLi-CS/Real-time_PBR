#version 330 core
out vec4 fragColour;

in Vertex{
    vec3 worldPos;
} IN;

uniform sampler2D equirectangularMap;

const float GAMMA = 2.2;
const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main(void)
{
     vec2 uv = SampleSphericalMap(normalize(IN.worldPos));

	 //vec3 finalColor = texture(equirectangularMap, uv).rgb;
	 //finalColor = pow(finalColor, vec3(GAMMA));
	 //finalColor = finalColor / (finalColor + vec3(1.0));
	 //finalColor = pow(finalColor, vec3(1.0 / GAMMA));
	 vec3 color = texture(equirectangularMap, uv).rgb;

	 fragColour = vec4(color, 1.0);
}