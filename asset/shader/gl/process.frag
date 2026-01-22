#version 440 core

layout (location = 0) out vec4 frag;

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

void main()
{
	ivec2 xy = ivec2(gl_FragCoord.x, gl_FragCoord.y);
	uint index = 1600 * xy.y + xy.x;

	if (index < 2)
	{
		particles[index].x += 0.01f;
		particles[index].y += 0.01f;

		positions[index].x = particles[index].x;
		positions[index].y = particles[index].y;
	}

	frag = vec4(1.0, 0.0, 0.0, 1.0);
}
