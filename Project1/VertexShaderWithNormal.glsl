#version 330 core

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Nor;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	TexCoords = texCoords;

	gl_Position = projection * view * model * vec4(Pos, 1.0f);
}
