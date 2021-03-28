#version 430

layout (location = 0) in vec2 pos;

out vec2 texCoord;

void main()
{
	texCoord = pos * .5 + .5;
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}