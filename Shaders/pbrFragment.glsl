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
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

//IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// pointlight
uniform vec3 lightPos;
uniform vec4 lightColor;

//directionallight
uniform vec3 lightDir;
uniform vec4 lightDirColor;

uniform vec3 cameraPos;

const float PI = 3.14159265359;
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
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (nom - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = (roughness + 1.0);
    float k = (a*a) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  
// ----------------------------------------------------------------------------
void calculateDirectlight(inout vec3 Lo, vec3 albedo, float metallic, float roughness, vec3 N, vec3 V, vec3 F0){
    vec3 L = normalize(-lightDir);
	vec3 H = normalize(V + L);

	vec3 radiance = lightDirColor.xyz;

    float cosTheta = max(dot(H, V), 0.0);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(cosTheta, F0);      
    
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
    vec3 specular = numerator / denominator;
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);        


    Lo += (kD * albedo / PI + specular) * radiance * NdotL;  
}
// ----------------------------------------------------------------------------
void calculatePointlight(inout vec3 Lo, vec3 albedo, float metallic, float roughness, vec3 N, vec3 V, vec3 F0){
    vec3 L = normalize(lightPos - IN.worldPos);
    vec3 H = normalize(V + L);

    float distance = length(lightPos - IN.worldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor.xyz * attenuation;

    float cosTheta = max(dot(H, V), 0.0);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(cosTheta, F0);        
    
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);        

    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    
    
}
// ----------------------------------------------------------------------------
void calculateIBL(inout vec3 Lo, vec3 albedo, float metallic, float roughness, float ao, vec3 N, vec3 V, vec3 R, vec3 F0){
    vec3 F =fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F; 
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;   
    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient    = (kD * diffuse + specular) * ao; 
    Lo += ambient;
}
// ----------------------------------------------------------------------------
void main()
{
    vec3 albedo = pow(texture(albedoMap, IN.texCoord).rgb, vec3(GAMMA));
    float metallic = texture(metallicMap, IN.texCoord).r;
    float roughness = texture(roughnessMap, IN.texCoord).r;
    float ao = texture(aoMap, IN.texCoord).r;

    /*mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.bitangent), normalize(IN.normal));
    vec3 normalColor = texture(normalMap, IN.texCoord).rgb;
	normalColor = normalColor * 2.0 - 1.0;
	normalColor.xy *= 1.0;
    normalColor = normalize(TBN * normalize(normalColor));
    vec3 N = normalize(normalColor);*/

    vec3 N = getNormalFromMap();
    vec3 V = normalize(cameraPos - IN.worldPos);
    vec3 R = reflect(-V, N); 

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    calculateDirectlight(Lo, albedo, metallic, roughness, N, V, F0);
    calculatePointlight(Lo, albedo, metallic, roughness, N, V, F0);
    calculateIBL(Lo, albedo, metallic, roughness, ao, N, V, R, F0);
    
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