#version 330 core

#define MAX_NR_LIGHTS 10

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 Tangent;
layout (location = 3) in vec3 Bitangent;
layout (location = 4) in vec2 texCoords;

struct PointLight {
	vec3 position;
	bool casts_shadows;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
uniform PointLight pointLights[MAX_NR_LIGHTS];
uniform int NUMBER_OF_POINT_LIGHTS;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 viewPos;


out VS_OUT {
	vec3 Debug;
	vec3 FragPos;
	vec2 TexCoords;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
	vec3[MAX_NR_LIGHTS] tangentPointLightPositions;
} vs_out;


void main()
{
	vec3 T = vec3(normalize(model * vec4(Tangent, 0.0)));
	vec3 B = vec3(normalize(model * vec4(Bitangent, 0.0)));
	vec3 N = vec3(normalize(model * vec4(Normal, 0.0)));

	mat3 TBN = transpose(mat3(T, B, N)); // World to tangent space

	vs_out.FragPos = vec3(model * vec4(Position, 1.0));
	vs_out.TexCoords = texCoords;

	vs_out.TangentViewPos = TBN * viewPos;
	vs_out.TangentFragPos = TBN * vs_out.FragPos;

	for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++){
		vs_out.tangentPointLightPositions[i] = TBN * pointLights[i].position;
	}

	vs_out.Debug = pointLights[1].position;

	gl_Position = projection * view * vec4(vs_out.FragPos, 1.0f);
}
