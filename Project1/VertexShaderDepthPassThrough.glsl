#version 330 core

layout (location = 0) in vec3 Pos;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
	gl_Position = lightSpaceMatrix * model * vec4(Pos, 1.0);
}
