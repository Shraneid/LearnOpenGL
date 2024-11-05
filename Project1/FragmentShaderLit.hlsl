#version 330 core

struct Material {
	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D emissionMap;
	vec3 specular;
	float shininess;
};
uniform Material material;

// Light Definition
struct DirectionalLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirectionalLight directionalLight;

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct SpotLight {
	vec3 direction;
	vec3 position;

	float innerCutoff;
	float outerCutoff;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform SpotLight spotLight;

uniform vec3 viewPos;

uniform float time;

uniform bool blackLightOn;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

// light calculation functions
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 fragPos, vec3 normal, vec3 viewDir);

void main()
{
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 result = vec3(0.0);
	
	result += CalcDirectionalLight(directionalLight, normal, viewDir);

	for (int i = 0; i < NR_POINT_LIGHTS; i++){
		result += CalcPointLight(pointLights[i], FragPos, normal, viewDir);
	}

	if (blackLightOn)
		result += CalcSpotLight(spotLight, FragPos, normal, viewDir);

	vec3 lightDir = normalize(-directionalLight.direction);
	vec3 reflectDir = reflect(-lightDir, normal);

	FragColor = vec4(result, 1.0f);
};


vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
	vec3 lightDir = normalize(-light.direction);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);

	if (diff < 0.001)
		spec = 0;

	vec3 ambient = light.ambient * vec3(texture(material.diffuseMap, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuseMap, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.specularMap, TexCoords));

	return (ambient + diffuse + specular);
}


vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir) {
	vec3 lightDir = normalize(light.position - fragPos);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	
	if (diff < 0.001)
		spec = 0;

	vec3 ambient = light.ambient * vec3(texture(material.diffuseMap, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuseMap, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.specularMap, TexCoords));
	specular = vec3(spec) * vec3(texture(material.specularMap, TexCoords));

	float distance = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
								light.quadratic * pow(distance, 2));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}


vec3 CalcSpotLight(SpotLight light, vec3 fragPos, vec3 normal, vec3 viewDir) {
	vec3 lightDir = normalize(light.position - fragPos);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);

	if (diff < 0.001)
		spec = 0;

	vec3 ambient = light.ambient * vec3(texture(material.emissionMap, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.emissionMap, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.specularMap, TexCoords));
	
	float distance = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
								light.quadratic * pow(distance,2));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.innerCutoff - light.outerCutoff;
	float intensity = clamp((theta - light.outerCutoff)/epsilon, 0.0, 1.0);

	diffuse *= intensity;
	specular *= intensity;
	
	//return (ambient + diffuse + specular);
	return (diffuse + specular);
}
