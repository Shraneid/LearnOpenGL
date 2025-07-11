#version 330 core

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Nor;
layout (location = 2) in vec2 texCoords;


out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	Normal = normalize(mat3(transpose(inverse(model))) * Nor);
	FragPos = (model * vec4(Pos, 1.0)).xyz;
	
	TexCoords = texCoords;

	gl_Position = projection * view * vec4(FragPos, 1.0f);
}
