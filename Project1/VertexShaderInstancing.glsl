#version 330 core

layout (location = 0) in vec2 Pos;
layout (location = 1) in vec3 Col;

out vec3 Color;

uniform vec2 offsets[100];

void main()
{
	vec2 offset = offsets[gl_InstanceID];

	Color = Col;
	gl_Position = vec4((Pos*2) + offset, 0, 1);
}
