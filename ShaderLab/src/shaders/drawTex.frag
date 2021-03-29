#version 430

layout (location = 0) uniform sampler2D map;
layout (location = 1) uniform sampler2D post;

in vec2 texCoord;

layout (location = 0) out vec4 color;

void main()
{
	color = vec4(texture(post, texCoord).rgb, 1.0);
}