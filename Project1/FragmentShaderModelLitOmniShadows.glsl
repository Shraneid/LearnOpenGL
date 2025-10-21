#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

#define MAX_NR_LIGHTS 10
struct DirectionalLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirectionalLight directionalLights[MAX_NR_LIGHTS];
uniform int NUMBER_OF_DIRECTIONAL_LIGHTS;

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
uniform PointLight pointLights[MAX_NR_LIGHTS];
uniform int NUMBER_OF_POINT_LIGHTS;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
};
uniform Material material;

uniform sampler2D shadowMap;
uniform samplerCube omniShadowMap;

uniform vec3 viewPos;
uniform float time;

uniform float far_plane;

vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir);

float OmniShadowCalculation(vec3 fragPos, vec3 lightPos);

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);  

void main()
{
	vec3 normal = normalize(fs_in.Normal);
	vec3 viewDir = -normalize(fs_in.FragPos - viewPos);

	vec3 result = vec3(0.0);

	for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++){
		result += CalcPointLight(pointLights[i], fs_in.FragPos, normal, viewDir);
	}

	FragColor = vec4(result, 1.0f);
};

vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir) {	
	vec3 lightDir = normalize(light.position - fragPos);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 200.0f);
	
	if (diff < 0.001)
		spec = 0;

	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));

	float distance = length(light.position - fs_in.FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
								light.quadratic * pow(distance, 2));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	float shadow = OmniShadowCalculation(fs_in.FragPos, light.position);

	vec3 litColor =  ambient + (1.0 - shadow) * (diffuse + specular);

	return litColor;
}

float OmniShadowCalculation(vec3 fragPos, vec3 lightPos){
	float shadow = 0.0;
	float bias = 0.05;
	int samples = 20;

	float viewDistance = length(viewPos - fragPos);
//	float diskRadius = 0.005;
	float diskRadius = (1.0 + (viewDistance / far_plane)) / 50.0;
	
	vec3 fragToLight = fragPos - lightPos;
	float currentDepth = length(fragToLight);
	
	for (int i = 0; i < samples; ++i)
	{
//		float closestDepth = texture(omniShadowMap, fragToLight).r;
		float closestDepth = texture(omniShadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= far_plane;
				
		if (currentDepth-bias > closestDepth)
		{
			shadow += 1.0;
		}
	}
	
	shadow /= float(samples);

	return shadow;
}
