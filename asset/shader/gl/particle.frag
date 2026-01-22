#version 440 core

layout (location = 0) out vec4 frag;
in vec2 ftexcoord;

uniform sampler2D tex;

void main()
{
	frag = texture(tex, ftexcoord);
}
