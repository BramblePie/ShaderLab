#version 430

layout (location = 0) uniform sampler2D srcTex;

in vec2 texCoord;

out vec4 color;

void main()
{
	//const vec3 c = texture(srcTex, texCoord);
	color = vec4(texture(srcTex, texCoord));
}