#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float visible;

void main()
{
	float isInCircle = step(length(vec2(0.5f, 0.5f) - TexCoord), 0.25);
	FragColor = texture(texture2, TexCoord) * isInCircle * visible + texture(texture1, TexCoord) * (1.0 - isInCircle);
};