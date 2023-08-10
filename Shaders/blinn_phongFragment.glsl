#version 330 core
out vec4 fragColor;

in Vertex
{
    vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
    vec3 tangent;
	vec3 bitangent;
} IN;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;

// pointlight
uniform vec3 lightPos;
uniform vec4 lightColor;
uniform vec4 lightData;

//directionallight
uniform vec3 lightDir;
uniform vec4 lightDirColor;

uniform vec3 cameraPos;

const float GAMMA = 2.2;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, IN.texCoord).rgb * 2.0 - 1.0;

    vec3 Q1  = dFdx(IN.worldPos);
    vec3 Q2  = dFdy(IN.worldPos);
    vec2 st1 = dFdx(IN.texCoord);
    vec2 st2 = dFdy(IN.texCoord);

    vec3 N   = normalize(IN.normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
vec3 calculateDirectlight(vec3 albedo, vec3 N, vec3 V){
     float ambientStrength = 0.03;
     vec3 ambient = ambientStrength * vec3(lightDirColor.xyz);

     vec3 L = normalize(-lightDir);
     vec3 diffuse = max(dot(N, L), 0.0) * vec3(lightDirColor.xyz);
     
     float specularStrength = 1.0;
     vec3 VHalf = normalize(L + V);
     vec3 specular = specularStrength * pow(max(dot(N, VHalf), 0.0), 128.0) * vec3(lightDirColor.xyz);

     return (ambient + diffuse + specular) * albedo;
        

}
// ----------------------------------------------------------------------------
vec3 calculatePointlight(vec3 albedo, vec3 N, vec3 V){
    float ambientStrength = 0.03;
    vec3 ambient = ambientStrength * vec3(lightColor.xyz);

    vec3 L = normalize(lightPos - IN.worldPos);
    vec3 diffuse = max(dot(N, L), 0.0) * vec3(lightColor.xyz);

    float specularStrength = 1.0;
    vec3 VHalf = normalize(L + V);
    vec3 specular = specularStrength * pow(max(dot(N, VHalf), 0.0), 128.0) * vec3(lightColor.xyz);
    
    float distance = length(lightPos - IN.worldPos);
    float attenuation = 1.0 / (lightData.x + lightData.y * distance + lightData.z * (distance * distance));

    ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

    return (ambient + diffuse + specular) * albedo;
}
// ----------------------------------------------------------------------------
void main()
{
    vec3 albedo     = pow(texture(albedoMap, IN.texCoord).rgb, vec3(GAMMA));

    /*mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.bitangent), normalize(IN.normal));
    vec3 normalColor = texture(normalMap, IN.texCoord).rgb;
	normalColor = normalColor * 2.0 - 1.0;
	normalColor.xy *= 1.0;
    normalColor = normalize(TBN * normalize(normalColor));
    vec3 N = normalize(normalColor);*/

    vec3 N = getNormalFromMap();
    vec3 V = normalize(cameraPos - IN.worldPos);

    vec3 Lo = calculateDirectlight(albedo, N, V);
    
    Lo += calculatePointlight(albedo, N, V);
    
    // ambient lighting 
    //vec3 ambient = vec3(0.03) * albedo * ao;
    //Lo += ambient;

    //Lo = pow(Lo, vec3(GAMMA));
    // HDR tonemapping
    Lo = Lo / (Lo + vec3(1.0));
    // gamma correct
    Lo = pow(Lo, vec3(1.0/GAMMA)); 

    fragColor = vec4(Lo, 1.0);
}