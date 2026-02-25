#include <ctime>
#include <random>
#define _USE_MATH_DEFINES
#include <cmath>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <win/Targa.hpp>

#include "Renderer.hpp"

using namespace win::gl;

static GLint get_uniform(win::GLProgram &program, const char *name)
{
	const auto loc = win::gl::glGetUniformLocation(program.get(), name);
	if (loc == -1)
		win::bug("No uniform " + std::string(name));

	return loc;
}

Renderer::Renderer(win::AssetRoll &roll, const win::Area<float> &area, int count)
	: count(count)
{
	fprintf(stderr, "%s %s\n", (const char *)glGetString(GL_VENDOR), (const char *)glGetString(GL_RENDERER));

	for (int i = 0; i < positionmap_width * positionmap_height; ++i)
		empty_positionmap[i] = -1;

	glClearColor(0.01f, 0.01f, 0.01f, 1.0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const glm::mat4 projection = glm::ortho(area.left, area.right, area.bottom, area.top);

	{
		processmode.program = win::GLProgram(win::gl_load_compute_shader(roll["shader/gl/process.comp"]));
		glUseProgram(processmode.program.get());

		processmode.uniform_count = get_uniform(processmode.program, "count");
		glUniform1i(processmode.uniform_count, count);

		processmode.uniform_area = get_uniform(processmode.program, "area");
		glUniform4f(processmode.uniform_area, area.left, area.right, area.bottom, area.top);

		processmode.uniform_pointer = get_uniform(processmode.program, "pointer");

		processmode.uniform_postionmap_res = get_uniform(processmode.program, "positionmap_res");
		glUniform2i(processmode.uniform_postionmap_res, positionmap_width, positionmap_height);

		// particles buffer
		{
			int len;
			const auto particles = get_initial_particles(area, count, len);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.particles.get());
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * len, particles.get(), GL_STATIC_DRAW);
			const auto loc = glGetProgramResourceIndex(processmode.program.get(), GL_SHADER_STORAGE_BLOCK, "Particles");
			if (loc == GL_INVALID_INDEX)
				win::bug("No buffer Particles");
			glShaderStorageBlockBinding(processmode.program.get(), loc, particle_ssbo_index);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, particle_ssbo_index, processmode.particles.get());
		}

		// randomness buffer
		{
			const auto randomness = get_randomness(count);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.randomness.get());
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * count, randomness.get(), GL_STATIC_DRAW);
			const auto loc = glGetProgramResourceIndex(processmode.program.get(), GL_SHADER_STORAGE_BLOCK, "Randomness");
			if (loc == GL_INVALID_INDEX)
				win::bug("No buffer Randomness");
			glShaderStorageBlockBinding(processmode.program.get(), loc, randomness_ssbo_index);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, randomness_ssbo_index, processmode.randomness.get());
		}

		// positions buffer
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positions.get());
			glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(float) * count * 3, NULL, 0);
			const auto loc = glGetProgramResourceIndex(processmode.program.get(), GL_SHADER_STORAGE_BLOCK, "Positions");
			if (loc == GL_INVALID_INDEX)
				win::bug("No buffer Positions");
			glShaderStorageBlockBinding(processmode.program.get(), loc, position_ssbo_index);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position_ssbo_index, processmode.positions.get());
		}

		// positionmap buffers
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positionmap_a.get());
			glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(empty_positionmap), empty_positionmap, GL_DYNAMIC_STORAGE_BIT);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positionmap_b.get());
			glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(empty_positionmap), empty_positionmap, GL_DYNAMIC_STORAGE_BIT);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positionmap_c.get());
			glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(empty_positionmap), empty_positionmap, GL_DYNAMIC_STORAGE_BIT);

			const auto loc = glGetProgramResourceIndex(processmode.program.get(), GL_SHADER_STORAGE_BLOCK, "PositionMapCurrent");
			if (loc == GL_INVALID_INDEX)
				win::bug("No buffer PositionMapCurrent");

			const auto loc2 = glGetProgramResourceIndex(processmode.program.get(), GL_SHADER_STORAGE_BLOCK, "PositionMapPrevious");
			if (loc2 == GL_INVALID_INDEX)
				win::bug("No buffer PositionMapPrevious");

			glShaderStorageBlockBinding(processmode.program.get(), loc, position_map_current_ssbo_index);
			glShaderStorageBlockBinding(processmode.program.get(), loc2, position_map_previous_ssbo_index);
		}
	}

	{
		particlemode.program = win::GLProgram(win::gl_load_shaders(roll["shader/gl/particle.vert"], roll["shader/gl/particle.frag"]));
		glUseProgram(particlemode.program.get());
		particlemode.uniform_projection = get_uniform(particlemode.program, "projection");

		glUniformMatrix4fv(particlemode.uniform_projection, 1, GL_FALSE, glm::value_ptr(projection));

		win::Targa tga(roll["texture/particle.tga"]);
		if (tga.bpp() != 32)
			win::bug("Need 32bit tga");
		glActiveTexture(particle_texture_unit);
		glBindTexture(GL_TEXTURE_2D, particlemode.tex.get());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tga.width(), tga.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tga.data());
		glUniform1i(get_uniform(particlemode.program, "tex"), particle_texture_unit - GL_TEXTURE0);

		{
			const auto loc = glGetProgramResourceIndex(particlemode.program.get(), GL_SHADER_STORAGE_BLOCK, "Positions");
			if (loc == GL_INVALID_INDEX)
				win::bug("No buffer positions");
			glShaderStorageBlockBinding(particlemode.program.get(), loc, position_ssbo_index);
		}
	}

	win::gl_check_error();
}

void Renderer::render(float x, float y)
{
	{
		if (processmode.positionmap_cycle == 0)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position_map_previous_ssbo_index, processmode.positionmap_a.get());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position_map_current_ssbo_index, processmode.positionmap_b.get());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positionmap_c.get());
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(empty_positionmap), empty_positionmap);
		}
		else if (processmode.positionmap_cycle == 1)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position_map_previous_ssbo_index, processmode.positionmap_b.get());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position_map_current_ssbo_index, processmode.positionmap_c.get());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positionmap_a.get());
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(empty_positionmap), empty_positionmap);
		}
		else
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position_map_previous_ssbo_index, processmode.positionmap_c.get());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position_map_current_ssbo_index, processmode.positionmap_a.get());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positionmap_b.get());
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(empty_positionmap), empty_positionmap);
		}

		processmode.positionmap_cycle = (processmode.positionmap_cycle + 1) % 3;

		glUseProgram(processmode.program.get());
		glUniform2f(processmode.uniform_pointer, x, y);
		glDispatchCompute(count, 1, 1);
	}

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	/*
	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
	int a[positionmap_width * positionmap_height];
	int b[positionmap_width * positionmap_height];
	int c[positionmap_width * positionmap_height];

	memset(a, 0, sizeof(a));
	memset(b, 0, sizeof(b));
	memset(c, 0, sizeof(c));

	glFlush();
	glFinish();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positionmap_a.get());
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(empty_positionmap), a);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positionmap_b.get());
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(empty_positionmap), b);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positionmap_c.get());
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(empty_positionmap), c);

	win::gl_check_error();
	*/

	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(particlemode.program.get());
		glBindVertexArray(particlemode.vao.get());

		glDrawArrays(GL_TRIANGLES, 0, 6 * count);
	}

	win::gl_check_error();
}

std::unique_ptr<float[]> Renderer::get_initial_particles(const win::Area<float> &area, int count, int &len)
{
	std::mt19937 mersenne(time(NULL));

	const auto random = [&mersenne](float a, float b)
	{
		return std::uniform_real_distribution(a, b)(mersenne);
	};

	len = count * 4;
	std::unique_ptr<float[]> particles(new float[len]);

	for (int i = 0; i < count; ++i)
	{
		const float speed = random(0.01f, 0.02f);
		const float angle = random(0, 2 * M_PI);

		particles[i * 4 + 0] = random(area.left, area.right);
		particles[i * 4 + 1] = random(-4.5f, 4.5f);
		particles[i * 4 + 2] = std::cosf(angle) * speed;
		particles[i * 4 + 3] = std::sinf(angle) * speed;
	}

	return particles;
}

std::unique_ptr<float[]> Renderer::get_randomness(int count)
{
	std::mt19937 mersenne(time(NULL));

	const auto random = [&mersenne](float a, float b)
	{
		return std::uniform_real_distribution(a, b)(mersenne);
	};

	std::unique_ptr<float[]> rands(new float[count]);

	for (int i = 0; i < count; ++i)
	{
		rands[i] = random(990, 1010) / 1000.0f;
	}

	return rands;
}
