#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices=6) out;

in vec3 Normal[];


layout (std140) uniform MatricesBlock
{
	mat4 view;
	mat4 projection;
};
uniform mat4 model;

void GenerateLine(int idx){
	vec3 normal = mat3(transpose(inverse(model))) * Normal[idx];

	gl_Position = projection * view * model * gl_in[idx].gl_Position;
	EmitVertex();
	
	gl_Position = gl_Position + vec4(normalize(normal) * 0.2, 0);
	EmitVertex();

	EndPrimitive();
}


void main() {
	GenerateLine(0);
	GenerateLine(1);
	GenerateLine(2);
}
