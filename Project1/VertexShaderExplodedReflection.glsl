#version 420 core

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Nor;
layout (location = 2) in vec2 texCoords;

out VS_OUT {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
} vs_out;

layout (std140) uniform MatricesBlock
{
	mat4 view;
	mat4 projection;
};

uniform mat4 model;


void main()
{
	vs_out.Normal = mat3(transpose(inverse(model))) * Nor;
	vs_out.FragPos = (model * vec4(Pos, 1.0)).xyz;
	
	vs_out.TexCoords = texCoords;

	gl_Position = projection * view * vec4(vs_out.FragPos, 1.0f);
}
