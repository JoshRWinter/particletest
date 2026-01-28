#version 440 core

struct Particle
{
	float x;
	float y;
	float xv;
	float yv;
};

layout (std430) buffer Particles
{
	Particle[] particles;
};

struct Position
{
	float x, y;
	float speed;
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
		if (particles[index].x > area.y)
		{
			particles[index].x = area.y;
			particles[index].xv = -particles[index].xv;
		}
		else if (particles[index].x < area.x)
		{
			particles[index].x = area.x;
			particles[index].xv = -particles[index].xv;
		}

		if (particles[index].y > area.w)
		{
			particles[index].y = area.w;
			particles[index].yv = -particles[index].yv;
		}
		else if (particles[index].y < area.z)
		{
			particles[index].y = area.z;
			particles[index].yv = -particles[index].yv;
		}

		float dist = max(distance(pointer, vec2(particles[index].x, particles[index].y)), 0.04);
		float angle = atan(particles[index].y - pointer.y, particles[index].x - pointer.x);
		vec2 influence = vec2(cos(angle), sin(angle)) * (0.0002 / pow(dist, 2));

		particles[index].xv += influence.x;
		particles[index].yv += influence.y;

		particles[index].xv *= 0.99f;
		particles[index].yv *= 0.99f;

		particles[index].x += particles[index].xv;
		particles[index].y += particles[index].yv;

		positions[index].x = particles[index].x;
		positions[index].y = particles[index].y;
		positions[index].speed = sqrt(pow(particles[index].xv, 2) + pow(particles[index].yv, 2)) * 10.0;
	}

	frag = vec4(1.0, 0.0, 0.0, 1.0);
}
