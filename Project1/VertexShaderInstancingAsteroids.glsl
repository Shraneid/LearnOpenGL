#version 330 core

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Nor;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;


uniform mat4 view;
uniform mat4 projection;

void main()
{
	Normal = mat3(inverse(transpose(instanceMatrix))) * Pos;
	TexCoords = aTexCoords;
	FragPos = vec3(instanceMatrix * vec4(Pos, 1.0));
	gl_Position = projection * view * vec4(FragPos, 1);
}
