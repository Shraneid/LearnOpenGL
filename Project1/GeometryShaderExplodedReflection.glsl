#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices=5) out;

in VS_OUT {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
} gs_in[];

out vec3 fColor;

void main() {
	fColor = gs_in[0].color;
	gl_Position = gl_in[0].gl_Position + vec4(-.1, -.1, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(+.1, -.1, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-.1, +.1, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(+.1, +.1, 0.0, 0.0);
	EmitVertex();

	fColor = vec3(1, 1, 1);
	gl_Position = gl_in[0].gl_Position + vec4(0.0, +.3, 0.0, 0.0);
	EmitVertex();


	EndPrimitive();
}
