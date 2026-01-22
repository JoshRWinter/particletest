#version 440 core

const float[] verts = float[](-1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0);

void main()
{
	gl_Position = vec4(verts[gl_VertexID * 2 + 0], verts[gl_VertexID * 2 + 1], 0.0, 1.0);
}
