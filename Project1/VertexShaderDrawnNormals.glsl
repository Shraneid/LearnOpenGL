#version 420 core

layout (location = 0) in vec3 LocalPos;
layout (location = 1) in vec3 Nor;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;

layout (std140) uniform MatricesBlock
{
	mat4 view;
	mat4 projection;
};
uniform mat4 model;


void main()
{	
	mat3 normalMatrix = mat3(transpose(inverse(view * model)));
	Normal = normalMatrix * Nor;
	gl_Position = view * model * vec4(LocalPos, 1.0);
}
