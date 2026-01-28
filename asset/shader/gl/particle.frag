#version 440 core

layout (location = 0) out vec4 frag;
in vec2 ftexcoord;
flat in float speed;

uniform sampler2D tex;

void main()
{
	frag = texture(tex, ftexcoord) * vec4(1.0 - speed, 1.0 - (speed / 1.5), 1.0 - (speed / 4.0), 1.0);
}
