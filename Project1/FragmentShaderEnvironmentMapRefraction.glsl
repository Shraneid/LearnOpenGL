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
	vec3 incidenceRay = normalize(FragPos - viewPos);

	float refractionRatio = 1.0 / 1.33;

	vec3 refractedDir = refract(incidenceRay, normalize(Normal), refractionRatio);
	vec4 refractedColor = texture(skybox, refractedDir);

	FragColor = refractedColor;
};
