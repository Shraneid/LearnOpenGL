#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
} fs_in;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
};
uniform Material material;


uniform vec3 viewPos;
uniform samplerCube skybox;

uniform vec3 lightDirection;


void main()
{
	vec3 normal = normalize(fs_in.Normal);
	vec3 fragPosToViewDir = normalize(viewPos - fs_in.FragPos);
	vec3 fragPosToLightDir = normalize(-lightDirection);

	float diff = max(dot(normal, fragPosToLightDir), 0.0);

	vec3 reflectedViewDir = reflect(-fragPosToViewDir, normal);

	vec3 reflectedLightDir = reflect(-fragPosToLightDir, normal);
	float spec = pow(max(dot(fragPosToViewDir, reflectedLightDir), 0.0f), 200.0f);

	if (diff < 0.001)
		spec = 0;

	vec3 ambient = 0.05 * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
	vec3 diffuse = 0.4 * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));

	float showSpecularReflection = 1 - step((spec * vec3(texture(material.texture_specular1, fs_in.TexCoords))).x, 0.05);

	vec3 specular = 0.55 * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));
	vec3 specularReflection = 0.55 * showSpecularReflection * vec3(texture(skybox, reflectedViewDir));

	FragColor = vec4((ambient + diffuse + specular + specularReflection), 1.0);
};
