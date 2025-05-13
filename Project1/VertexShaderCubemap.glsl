#version 330 core

layout (location = 0) in vec3 Pos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * vec4(Pos, 1.0);
	TexCoords = Pos;
}
