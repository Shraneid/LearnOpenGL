#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in VS_OUT {
	vec2 TexCoords;
} gs_in[];

out GS_OUT {
	vec3 Normal;
	vec3 WorldPos;
	vec2 TexCoords;
} gs_out;


layout (std140) uniform MatricesBlock
{
	mat4 view;
	mat4 projection;
};
uniform mat4 model;
uniform float time;


vec3 CalculateNormal(){
    vec3 a = vec3(gl_in[1].gl_Position) - vec3(gl_in[0].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[0].gl_Position);
    return normalize(cross(a, b));
}


vec4 CalculateExplodePosition(vec4 position, vec3 normal){
	float force = 2;
	vec4 displacement = vec4(normal * (sin(time)+1)/2 * force, 0);

	return position + displacement;
}


void main() {
	vec3 triangleNormal = CalculateNormal();

	vec4 worldPos = model * CalculateExplodePosition(gl_in[0].gl_Position, triangleNormal);
	gl_Position = projection * view * worldPos;
	gs_out.Normal = triangleNormal;
	gs_out.WorldPos = worldPos.xyz;
	gs_out.TexCoords = gs_in[0].TexCoords;
	EmitVertex();

	worldPos = model * CalculateExplodePosition(gl_in[1].gl_Position, triangleNormal);
	gl_Position = projection * view * worldPos;
	gs_out.Normal = triangleNormal;
	gs_out.WorldPos = worldPos.xyz;
	gs_out.TexCoords = gs_in[1].TexCoords;
	EmitVertex();

	worldPos = model * CalculateExplodePosition(gl_in[2].gl_Position, triangleNormal);
	gl_Position = projection * view * worldPos;
	gs_out.Normal = triangleNormal;
	gs_out.WorldPos = worldPos.xyz;
	gs_out.TexCoords = gs_in[2].TexCoords;
	EmitVertex();

	EndPrimitive();
}
