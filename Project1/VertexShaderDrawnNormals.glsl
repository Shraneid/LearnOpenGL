#version 420 core

layout (location = 0) in vec3 LocalPos;
layout (location = 1) in vec3 Nor;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;

void main()
{	
	Normal = Nor;
	gl_Position = vec4(LocalPos, 1.0);
}
