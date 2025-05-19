#version 330 core

layout (location = 0) in vec3 Pos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 pos = projection * view * vec4(Pos, 1.0);
	gl_Position = pos.xyww;
	TexCoords = Pos;
}
