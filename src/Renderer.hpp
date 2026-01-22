#pragma once

#include <win/AssetRoll.hpp>
#include <win/gl/GL.hpp>
#include <win/Win.hpp>

class Renderer
{
	WIN_NO_COPY_MOVE(Renderer);

	static constexpr GLenum particle_texture_unit = GL_TEXTURE0;

public:
	Renderer(win::AssetRoll &roll);

	void render();

private:
	struct
	{
		win::GLProgram program;
	} processmode;

	struct
	{
		win::GLProgram program;
		win::GLVertexArray vao;
		int uniform_projection;
		win::GLTexture tex;
	} particlemode;
};
