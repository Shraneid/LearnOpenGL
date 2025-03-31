#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

uniform float rightOffset;

out vec3 colorFromVertex;

void main()
{
	gl_Position = vec4(aPos.x + rightOffset, aPos.yz, 1.0);
	colorFromVertex = aCol;
}