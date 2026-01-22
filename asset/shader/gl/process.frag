#version 440 core

struct Particle
{
	float x, y;
};

layout (std430) buffer Particles
{
	Particle[] particles;
};

struct Position
{
	float x, y;
};

layout (std430) buffer Positions
{
	Position[] positions;
};

layout (location = 0) out vec4 frag;

uniform ivec2 res;
uniform uint count;

void main()
{
	ivec2 xy = ivec2(gl_FragCoord.x, gl_FragCoord.y);
	uint index = res.x * xy.y + xy.x;

	if (index < count)
	{
		particles[index].x += 0.01f;
		particles[index].y += 0.01f;

		positions[index].x = particles[index].x;
		positions[index].y = particles[index].y;
	}

	frag = vec4(1.0, 0.0, 0.0, 1.0);
}
