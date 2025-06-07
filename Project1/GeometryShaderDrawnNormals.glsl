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

void GenerateLine(int index){
	gl_Position = projection * gl_in[index].gl_Position;
	EmitVertex();
	
	vec3 normalVec = Normal[index];
	normalVec *= 0.2;
	gl_Position = projection * (gl_in[index].gl_Position + vec4(normalVec, 0));
	EmitVertex();

	EndPrimitive();
}


void main() {
	GenerateLine(0);
	GenerateLine(1);
	GenerateLine(2);
}
