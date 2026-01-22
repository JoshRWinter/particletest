#pragma once

#include <win/AssetRoll.hpp>
#include <win/gl/GL.hpp>
#include <win/Win.hpp>

class Renderer
{
	WIN_NO_COPY_MOVE(Renderer);

	static constexpr GLenum particle_texture_unit = GL_TEXTURE0;
	static constexpr GLenum process_fbo_texture_unit = GL_TEXTURE1;

	static constexpr GLuint particle_ssbo_index = 0;
	static constexpr GLuint position_ssbo_index = 1;

public:
	Renderer(win::AssetRoll &roll);

	void render();

private:
	struct
	{
		win::GLFramebuffer fbo;
		win::GLTexture fbotex;
		win::GLProgram program;
		win::GLVertexArray vao;
		win::GLBuffer particles;
		win::GLBuffer positions;
	} processmode;

	struct
	{
		win::GLProgram program;
		win::GLVertexArray vao;
		int uniform_projection;
		win::GLTexture tex;
	} particlemode;
};
