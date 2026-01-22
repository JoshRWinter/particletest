#include "Renderer.hpp"

Renderer::Renderer()
{
	win::load_gl_functions();
	glClearColor(0.01f, 0.01f, 0.01f, 1.0);
}

void Renderer::render()
{
	glClear(GL_COLOR_BUFFER_BIT);
}
