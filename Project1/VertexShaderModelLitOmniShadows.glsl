#version 330 core

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Nor;
layout (location = 2) in vec3 Tangent;
layout (location = 3) in vec2 texCoords;

out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform bool reverse_normals;


void main()
{
	vs_out.FragPos = vec3(model * vec4(Pos, 1.0));

	if (reverse_normals){
		vs_out.Normal = transpose(inverse(mat3(model))) * (-1.0 * Nor);
	} else {
		vs_out.Normal = transpose(inverse(mat3(model))) * Nor;
	}

	vs_out.TexCoords = texCoords;

	gl_Position = projection * view * vec4(vs_out.FragPos, 1.0f);
}
