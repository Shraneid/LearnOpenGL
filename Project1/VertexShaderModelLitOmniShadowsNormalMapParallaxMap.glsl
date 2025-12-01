#version 330 core

#define MAX_NR_LIGHTS 10

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 Tangent;
layout (location = 3) in vec2 texCoords;

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

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 viewPos;

uniform float reverse_normals; // 1.0 if we need to reverse, 0.0 otherwise


out VS_OUT {
	vec3 FragPos;
	vec2 TexCoords;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
	vec3[MAX_NR_LIGHTS] tangentPointLightPositions;
} vs_out;


void main()
{
	float reverse_normals_factor = -2.0 * (reverse_normals - 0.5);
	vec3 N = reverse_normals_factor * normalize(Normal);

	vec3 T = vec3(normalize(model * vec4(Tangent, 0.0)));
	T = T - dot(T, N) * N;
	
	vec3 B = cross(N, T);

	mat3 TBN = mat3(T, B, N);
	mat3 inverse_TBN = transpose(TBN); // TBN^-1 = TBN^T

	vs_out.FragPos = vec3(model * vec4(Position, 1.0));
	vs_out.TexCoords = texCoords;

	vs_out.TangentViewPos = inverse_TBN * viewPos;
	vs_out.TangentFragPos = inverse_TBN * vs_out.FragPos;

	for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++){
		vs_out.tangentPointLightPositions[i] = inverse_TBN * pointLights[i].position;
	}

	gl_Position = projection * view * vec4(vs_out.FragPos, 1.0f);
}
