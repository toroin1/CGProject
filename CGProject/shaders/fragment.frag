#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1; //only diffuse map, the ambient usually has the same color as the diffuse 
    sampler2D texture_specular1;    

    float shininess;
}; 



struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
     vec3 ambient;
     vec3 diffuse;
};

in vec3 FragPos;  
in vec3 Normal;  

in vec2 TexCoords;
  
uniform vec3 viewPos;
uniform Material material;
uniform float far_plane;
uniform float ambientlight;

#define NR_POINT_LIGHTS 2
uniform PointLight lamp[NR_POINT_LIGHTS];
uniform samplerCube depthMap[NR_POINT_LIGHTS];
uniform bool shadowenable[NR_POINT_LIGHTS];


// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalc(vec3 fragPos, vec3 lightPos, samplerCube depthMap)
{
    vec3 fragToLight = fragPos - lightPos;

    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;

    float closestDepth;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);


    return shadow;

}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,samplerCube depthMap, bool shadowenable)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));
                 
    // combine results
    vec3 ambient  = light.ambient * texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 diffuse  = light.diffuse  * diff * texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specular = spec * texture(material.texture_specular1, TexCoords).rgb;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    float shadow = ShadowCalc(fragPos, light.position, depthMap);

    vec3 result = shadowenable ? (ambient + (1.0 - shadow)*(diffuse + specular)) : (ambient + (1.0 - shadow)*(diffuse + specular))*0.0;;


    return result;
} 


void main()
{

    vec3 norm = normalize(Normal);




    vec3 viewDir = normalize(viewPos - FragPos);


    vec3 ambient = ambientlight * vec3(1.0,1.0,1.0);
    vec3 result = ambient*texture(material.texture_diffuse1, TexCoords).rgb;

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(lamp[i], norm, FragPos, viewDir, depthMap[i], shadowenable[i]);
        
    FragColor = vec4(result, 1.0);
} 


