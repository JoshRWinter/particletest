#include <chrono>

#include <win/AssetRoll.hpp>
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
	display_options.width = 1920;
	display_options.height = 1080;
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

	win::gl_load_functions();

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

	win::Area area(-8.0f, 8.0f, -4.5f, 4.5f);

	float mousex = 0.0f, mousey = 0.0f;
	display.register_mouse_handler(
		[&display, &area, &mousex, &mousey](int x, int y)
		{
			mousex = (x / (float)display.width()) * (area.right - area.left) - area.right;
			mousey = (-y / (float)display.height()) * (area.top - area.bottom) + area.top;
		});

	win::AssetRoll roll("pt.roll");

	Renderer renderer(roll, area, 1'000'000);

	while (!quit)
	{
		display.process();

		renderer.render(mousex, mousey);

		display.swap();
		{
			static auto last = std::chrono::steady_clock::now();
			const auto now = std::chrono::steady_clock::now();
			static int fps = 0;

			if (std::chrono::duration<float>(now - last).count() > 1.0f)
			{
				last = now;
				fprintf(stderr, "FPS: %d\n", fps);
				fps = 0;
			}

			++fps;
		}
	}

	return 0;
}
