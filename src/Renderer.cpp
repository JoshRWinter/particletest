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
	}
}

void Renderer::render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(particlemode.program.get());
	glBindVertexArray(particlemode.vao.get());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	win::gl_check_error();
}
