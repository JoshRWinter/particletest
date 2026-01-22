#include <win/Display.hpp>
#include <win/gl/GL.hpp>
#include <win/Utility.hpp>

#include "Renderer.hpp"

#if defined WINPLAT_WINDOWS && NDEBUG
int WinMain(HINSTANCE hinstance, HINSTANCE prev, PSTR cmd, int show)
#else
int main(int argc, char **argv)
#endif
{
	// display setup
	win::DisplayOptions display_options;
#ifndef NDEBUG
	display_options.caption = "debug_window";
	display_options.fullscreen = false;
	display_options.width = 1600;
	display_options.height = 900;
	display_options.debug = false;
#else
	display_options.caption = "particles";
	display_options.fullscreen = true;
	display_options.width = 1600;
	display_options.height = 900;
#endif
	display_options.gl_major = 4;
	display_options.gl_minor = 4;

	win::Display display(display_options);
	display.vsync(true);

	bool quit = false;
	display.register_button_handler(
		[&quit](win::Button button, bool press)
		{
			switch (button)
			{
				case win::Button::esc:
					if (press)
						quit = true;
					break;
				default:
					break;
			}
		});

	display.register_window_handler(
		[&quit](win::WindowEvent e)
		{
			if (e == win::WindowEvent::close)
				quit = true;
		});

	Renderer renderer;

	while (!quit)
	{
		display.process();

		renderer.render();

		display.swap();
	}

	return 0;
}
