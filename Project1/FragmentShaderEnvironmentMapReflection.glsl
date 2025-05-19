#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
};
uniform Material material;


uniform vec3 viewPos;
uniform samplerCube skybox;


void main()
{
	vec4 diffuse = texture(material.texture_diffuse1, TexCoords);
	vec4 specular = texture(material.texture_specular1, TexCoords);

	vec3 reflectedDir = reflect(-normalize(viewPos - FragPos), Normal);
	vec4 reflectionColor = texture(skybox, reflectedDir);

	float reflectPower = step(0, specular.r);
//	float reflectPower = step(0.1, specular.r);
	
	FragColor = diffuse * (1-reflectPower) + reflectionColor * reflectPower + reflectionColor * (1-reflectPower) * 0.1;
};
