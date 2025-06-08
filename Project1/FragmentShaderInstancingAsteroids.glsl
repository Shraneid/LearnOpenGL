#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

struct Material {
	sampler2D texture_diffuse1;
};
uniform Material material;

uniform vec3 viewPos;

void main()
{
	vec3 viewDir = viewPos - FragPos;

	vec3 lightDir = normalize(vec3(-0.2, -1, 0.3));

	float diff = min(max(dot(Normal,-lightDir), 0.0), 1.0);

	vec3 reflectDir = reflect(lightDir, Normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 0.5);

	if (diff < 0.001)
		spec = 0;

	vec3 ambient = 0.05 * vec3(texture(material.texture_diffuse1, TexCoords));
	vec3 diffuse = 0.95 * diff * vec3(texture(material.texture_diffuse1, TexCoords));

//	FragColor = vec4(ambient + diffuse, 1);
	FragColor = vec4(Normal, 1);
};