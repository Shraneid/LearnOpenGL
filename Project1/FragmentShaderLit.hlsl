#version 330 core

struct Material {
	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D emissionMap;
	vec3 specular;
	float shininess;
};

struct Light {
	vec3 position;
	vec3 direction;
	float outer_cutoff;
	float inner_cutoff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specularColor;

	float constant;
	float linear;
	float quadratic;
};

uniform Light light;
uniform Material material;

uniform vec3 cameraPos;

uniform float time;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

void main()
{
	vec3 diffuseValue = texture(material.diffuseMap, TexCoords).rgb;
	vec3 specularValue = texture(material.specularMap, TexCoords).rgb;
	vec3 emissionValue = texture(material.emissionMap, vec2(TexCoords.x, TexCoords.y + time/4.0)).rgb;

	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);

	float distance = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * pow(distance,2));

	vec3 ambient = light.ambient * diffuseValue;
	
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * diffuseValue;

	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 reflectedDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectedDir), 0.0), material.shininess * 128);
	vec3 specular = light.specularColor * specularValue * spec;
	
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	// cone cutoff calculations
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.inner_cutoff - light.outer_cutoff;
	float intensity = clamp((theta - light.outer_cutoff)/epsilon, 0.0, 1.0);

	diffuse *= intensity;
	specular *= intensity;
	
	vec3 result = ambient + diffuse + specular;

	FragColor = vec4(result, 1.0f);
};
