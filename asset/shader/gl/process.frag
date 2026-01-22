#version 440 core

struct Particle
{
	float distance;
	float angle;
	float speed;
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
uniform uint time;

void main()
{
	ivec2 xy = ivec2(gl_FragCoord.x, gl_FragCoord.y);
	uint index = res.x * xy.y + xy.x;

	if (index < count)
	{
		particles[index].angle += particles[index].speed;

		Particle particle = particles[index];

		positions[index].x = cos(particle.angle) * particle.distance;
		positions[index].y = sin(particle.angle) * particle.distance;
	}

	frag = vec4(1.0, 0.0, 0.0, 1.0);
}
