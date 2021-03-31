#version 430

layout (location = 0) uniform sampler2D texture0;

in vec2 texCoord;

layout (location = 0) out vec4 color;

void main()
{
	color = vec4(texture(texture0, texCoord).rgb, 1.0);
}