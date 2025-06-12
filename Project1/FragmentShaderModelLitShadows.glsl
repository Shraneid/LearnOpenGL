#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
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

uniform vec3 viewPos;
uniform float time;

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir);
//vec3 CalcSpotLight(SpotLight light, vec3 fragPos, vec3 normal, vec3 viewDir);

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);

void main()
{
	vec3 normal = normalize(fs_in.Normal);
	vec3 viewDir = -normalize(fs_in.FragPos - viewPos);

	vec3 result = vec3(0.0);

	for (int i = 0; i < NUMBER_OF_DIRECTIONAL_LIGHTS; i++){
		result += CalcDirectionalLight(directionalLights[i], normal, viewDir);
	}

	for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++){
		result += CalcPointLight(pointLights[i], fs_in.FragPos, normal, viewDir);
	}

	FragColor = vec4(result, 1.0f);
};

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
	vec3 color = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;

	vec3 ambient = color * light.ambient;

	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(lightDir, normal), 0.0);
	
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * light.specular;

	float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);

	vec3 litColor =  (ambient + (1.0 - shadow) * (diff + specular)) * color;

	return litColor;
}

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

	return (ambient + diffuse + specular);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir){
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	if (projCoords.z > 1) return 0;
	
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	float viewDepth = projCoords.z;

	
	float pcfFactor = 4.0;
	float baseShadowBias = 0.001;

	float shadowBias = max(baseShadowBias * 10 * (1.0 - dot(normal, lightDir)), baseShadowBias);

	float shadow = 0;

	for (float j = -pcfFactor; j <= pcfFactor; j++){
		for (float i = -pcfFactor; i <= pcfFactor; i++){
			float lightDepth = texture(shadowMap, projCoords.xy + vec2(i,j) * texelSize).r;
			shadow += viewDepth - shadowBias > lightDepth ? 1 : 0;
		}
	}

	return shadow / pow((pcfFactor * 2.0 + 1.0), 2);
}
