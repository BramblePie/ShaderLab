#version 430

layout (location = 0) uniform sampler2D map;

in vec2 texCoord;

out vec4 color;

void main()
{
	color = vec4(texture(map, texCoord));
}