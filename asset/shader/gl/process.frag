#version 440 core

struct Particle
{
	float x;
	float y;
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
uniform vec4 area;
uniform vec2 pointer;

const float pi = 3.1415926;

float reflectv(float a)
{
	float diff = pi - a;
	return a + diff * 2;
}

float reflecth(float a)
{
	float diff = pi / 2.0 - a;
	return a + diff * 2.0f;
}

void main()
{
	ivec2 xy = ivec2(gl_FragCoord.x, gl_FragCoord.y);
	uint index = res.x * xy.y + xy.x;

	if (index < count)
	{
		particles[index].x += cos(particles[index].angle) * particles[index].speed;
		particles[index].y += sin(particles[index].angle) * particles[index].speed;

		if (particles[index].x > area.y)
		{
			particles[index].x = area.y;
			particles[index].angle = reflecth(particles[index].angle);
		}
		else if (particles[index].x < area.x)
		{
			particles[index].x = area.x;
			particles[index].angle = reflecth(particles[index].angle);
		}

		if (particles[index].y > area.w)
		{
			particles[index].y = area.w;
			particles[index].angle = reflectv(particles[index].angle);
		}
		else if (particles[index].y < area.z)
		{
			particles[index].y = area.z;
			particles[index].angle = reflectv(particles[index].angle);
		}

		float dist = distance(pointer, vec2(particles[index].x, particles[index].y));
		if (dist < 2.2)
		particles[index].angle = atan(particles[index].y - pointer.y, particles[index].x - pointer.x);

		particles[index].speed *= 0.9999;
		if (particles[index].speed < 0.0003f) particles[index].speed = 0.0f;

		positions[index].x = particles[index].x;
		positions[index].y = particles[index].y;
	}

	frag = vec4(1.0, 0.0, 0.0, 1.0);
}
