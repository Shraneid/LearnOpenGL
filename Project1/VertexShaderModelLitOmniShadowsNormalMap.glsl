#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 Tangent;
layout (location = 3) in vec2 texCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform float reverse_normals; // 1.0 if we need to reverse, 0.0 otherwise


out VS_OUT {
	mat3 TBN;
	vec3 FragPos;
	vec2 TexCoords;
} vs_out;


void main()
{
	mat3 NormalMatrix = transpose(inverse(mat3(model)));

//	float reverse_normals_factor = -2.0 * (reverse_normals - 0.5);
	vec3 WorldSpaceNormal = normalize(NormalMatrix * Normal);

//	vec3 tangent = vec3(1,0,0);
	vec3 tangent = normalize(NormalMatrix * Tangent);
	
	vec3 Bitangent = cross(WorldSpaceNormal, tangent);
	vs_out.TBN = mat3(tangent, Bitangent, WorldSpaceNormal);


	vs_out.FragPos = vec3(model * vec4(Position, 1.0));
	vs_out.TexCoords = texCoords;

	gl_Position = projection * view * vec4(vs_out.FragPos, 1.0f);
}
