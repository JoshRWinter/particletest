#pragma once

#include <memory>

#include <win/AssetRoll.hpp>
#include <win/gl/GL.hpp>
#include <win/Utility.hpp>
#include <win/Win.hpp>

class Renderer
{
	WIN_NO_COPY_MOVE(Renderer);
	static constexpr int positionmap_width = 1600;
	static constexpr int positionmap_height = 900;

	static inline int empty_positionmap[positionmap_width * positionmap_height];

	static constexpr GLenum particle_texture_unit = GL_TEXTURE0;
	static constexpr GLenum process_fbo_texture_unit = GL_TEXTURE1;

	static constexpr GLuint particle_ssbo_index = 0;
	static constexpr GLuint randomness_ssbo_index = 1;
	static constexpr GLuint position_ssbo_index = 2;
	static constexpr GLuint position_map_current_ssbo_index = 3;
	static constexpr GLuint position_map_previous_ssbo_index = 4;

public:
	Renderer(win::AssetRoll &roll, const win::Area<float> &area, int count);

	void render(float x, float y);

private:
	static std::unique_ptr<float[]> get_initial_particles(const win::Area<float> &area, int count, int &len);
	static std::unique_ptr<float[]> get_randomness(int count);

	const int count;

	struct
	{
		win::GLProgram program;
		int uniform_count;
		int uniform_area;
		int uniform_pointer;
		int uniform_postionmap_res;

		win::GLBuffer particles;
		win::GLBuffer randomness;
		win::GLBuffer positions;
		win::GLBuffer positionmap_a, positionmap_b, positionmap_c;
		int positionmap_cycle = 0;
	} processmode;

	struct
	{
		win::GLProgram program;
		win::GLVertexArray vao;
		int uniform_projection;
		win::GLTexture tex;
	} particlemode;
};
