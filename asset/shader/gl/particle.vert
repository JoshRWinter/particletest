#version 440 core

struct Position
{
	float x, y;
	float speed;
};

layout (std430) buffer Positions
{
	Position[] positions;
};

const float h = 0.5f;
const float[] verts = float[](-h, h, -h, -h, h, -h, -h, h, h, -h, h, h);
const float[] tc = float[](0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1);

out vec2 ftexcoord;
out float speed;

uniform mat4 projection;

void main()
{
	uint index = gl_VertexID / 6;
	uint vert = gl_VertexID % 6;

	ftexcoord = vec2(tc[vert * 2 + 0], tc[vert * 2 + 1]);
	speed = positions[index].speed;

	vec2 size = vec2(0.025, 0.025);
	vec2 pos = vec2(positions[index].x, positions[index].y);
	mat4 model = mat4(size.x, 0.0, 0.0, 0.0, 0.0, size.y, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, pos.x, pos.y, 0.0, 1.0);
	gl_Position = projection * model * vec4(verts[vert * 2 + 0], verts[vert * 2 + 1], 0.0, 1.0);
}
