#pragma once

#include <win/gl/GL.hpp>
#include <win/Win.hpp>

class Renderer
{
	WIN_NO_COPY_MOVE(Renderer);

public:
	Renderer();

	void render();

private:
	struct
	{
		win::GLProgram program;
	} particlemode;

	struct
	{
		win::GLProgram program;
	} postmode;
};
