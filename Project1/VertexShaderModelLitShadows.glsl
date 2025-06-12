#version 330 core

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Nor;
layout (location = 2) in vec2 texCoords;

out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;


void main()
{
	vs_out.Normal = mat3(transpose(inverse(model))) * Nor;
	vs_out.FragPos = (model * vec4(Pos, 1.0)).xyz;
	
	vs_out.TexCoords = texCoords;

	vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

	gl_Position = projection * view * vec4(vs_out.FragPos, 1.0f);
}
