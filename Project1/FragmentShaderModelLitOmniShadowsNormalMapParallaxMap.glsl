#version 330 core

out vec4 FragColor;

in VS_OUT {
	mat3 TBN;
	vec3 FragPos;
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
	sampler2D texture_normal1;
	sampler2D texture_parallax1;
};
uniform Material material;

uniform sampler2D shadowMap;
uniform samplerCube omniShadowMap;

uniform vec3 viewPos;
uniform float time;

uniform float far_plane;

vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 viewDir);

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
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);

	vec3 result = vec3(0.0);

	for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++){
		result += CalcPointLight(pointLights[i], fs_in.FragPos, viewDir);
	}

	FragColor = vec4(result, 1.0f);
};

vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 viewDir) {
	vec3 diffuse_sample = vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
	vec3 normal_sample = vec3(texture(material.texture_normal1, fs_in.TexCoords));
	float parallax_sample = texture(material.texture_parallax1, fs_in.TexCoords).r;

	vec3 normal = normal_sample * 2.0 - 1.0;
	vec3 worldSpaceNormal = fs_in.TBN * normal;

	vec3 lightDir = normalize(light.position - fragPos);

	float diff = max(dot(worldSpaceNormal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, worldSpaceNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 200.0f);
	
	if (diff < 0.001)
		spec = 0;

	vec3 ambient = light.ambient * diffuse_sample;
	vec3 diffuse = light.diffuse * diff * diffuse_sample;
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
								light.quadratic * pow(distance, 2));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	float shadow = OmniShadowCalculation(fragPos, light.position);

	vec3 litColor =  ambient + (1.0 - shadow) * (diffuse + specular);

	return litColor;
}

float OmniShadowCalculation(vec3 fragPos, vec3 lightPos){
	float shadow = 0.0;
	float bias = 0.05;
	int samples = 20;

	float viewDistance = length(viewPos - fragPos);
	float diskRadius = (1.0 + (viewDistance / far_plane)) / 50.0;
	
	vec3 fragToLight = fragPos - lightPos;
	float currentDepth = length(fragToLight);
	
	for (int i = 0; i < samples; ++i)
	{
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
