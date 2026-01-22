#version 440 core

const float[] verts = float[](-0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, -0.5);
const float[] tc = float[](0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0);

out vec2 ftexcoord;

uniform mat4 projection;

void main()
{
	ftexcoord = vec2(tc[gl_VertexID * 2 + 0], tc[gl_VertexID * 2 + 1]);
	vec2 size = vec2(1.0, 1.0);
	vec2 pos = vec2(2, 2);
	mat4 model = mat4(size.x, 0.0, 0.0, 0.0, 0.0, size.y, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, pos.x, pos.y, 0.0, 1.0);
	gl_Position = projection * model * vec4(verts[gl_VertexID * 2 + 0], verts[gl_VertexID * 2 + 1], 0.0, 1.0);
}
