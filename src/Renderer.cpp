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

Renderer::Renderer(win::AssetRoll &roll)
{
	glClearColor(0.01f, 0.01f, 0.01f, 1.0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const glm::mat4 projection = glm::ortho(-8.0f, 8.0f, -4.5f, 4.5f);

	{
		glBindFramebuffer(GL_FRAMEBUFFER, processmode.fbo.get());
		glActiveTexture(process_fbo_texture_unit);
		glBindTexture(GL_TEXTURE_2D, processmode.fbotex.get());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 100, 100, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, processmode.fbotex.get(), 0);
		const GLenum buffers[] {0};
		glDrawBuffers(1, buffers);

		processmode.program = win::GLProgram(win::load_gl_shaders(roll["shader/gl/process.vert"], roll["shader/gl/process.frag"]));
		glUseProgram(processmode.program.get());
		glBindVertexArray(processmode.vao.get());

		{
			const float particles[] {2, 2, -2, -2};
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.particles.get());
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4, particles, GL_STATIC_DRAW);
			const auto loc = glGetProgramResourceIndex(processmode.program.get(), GL_SHADER_STORAGE_BLOCK, "Particles");
			if (loc == GL_INVALID_INDEX)
				win::bug("No buffer particles");
			glShaderStorageBlockBinding(processmode.program.get(), loc, particle_ssbo_index);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, particle_ssbo_index, processmode.particles.get());
		}

		{
			const float positions[] {2, 2, -2, -2};
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, processmode.positions.get());
			glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4, positions, 0);
			const auto loc = glGetProgramResourceIndex(processmode.program.get(), GL_SHADER_STORAGE_BLOCK, "Positions");
			if (loc == GL_INVALID_INDEX)
				win::bug("No buffer positions");
			glShaderStorageBlockBinding(processmode.program.get(), loc, position_ssbo_index);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position_ssbo_index, processmode.positions.get());
		}
	}

	{
		particlemode.program = win::GLProgram(win::load_gl_shaders(roll["shader/gl/particle.vert"], roll["shader/gl/particle.frag"]));
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

void Renderer::render()
{
	{
		glBindFramebuffer(GL_FRAMEBUFFER, processmode.fbotex.get());
		glUseProgram(processmode.program.get());
		glBindVertexArray(processmode.vao.get());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(particlemode.program.get());
		glBindVertexArray(particlemode.vao.get());

		glDrawArrays(GL_TRIANGLES, 0, 12);
	}

	win::gl_check_error();
}
