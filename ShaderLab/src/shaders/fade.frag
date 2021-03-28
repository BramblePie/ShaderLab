#version 430

layout (location = 0) uniform sampler2D map;
layout (location = 1) uniform sampler2D postMap;
layout (location = 2) uniform float delta;

in vec2 texCoord;

out vec4 color;

void main()
{
	color = vec4(texture(postMap, texCoord));
	color.rgb -= delta * 0.01;
}