#include "pch.h"

#include <Stallout/base.h>
#include <Stallout/gameutils.h>

st::Game_Window* window;

st::Font* font;

export_function(int) init() { 
   
	window = stnew (st::Game_Window) ("Fonts Test");

	auto path = st::os::io::get_workspace_dir();
	path.concat("/proto/arial.ttf");
	font = stnew (st::Font)(window->_gfx_driver, path.str, 24);

	st::set_logger_level("Core", spdlog::level::info);

	return 0;
}



export_function(int) update(st::Duration frame_time) { 
	window->clear_color = mz::COLOR_CYAN;
	window->new_frame((f32)frame_time.get_seconds());

	f32 ystart = window->get_size().y / 2.f - 80;

	st::vec2 sz = font->measure(L"Ἐν ἀρχῇ ἦν ὁ λόγος, καὶ ὁ λόγος ἦν πρὸς τὸν θεόν, καὶ θεός ἦν ὁ λόγος");
	window->draw_quad({ -window->get_size().x/2.f + 40, ystart - 30 }, sz, 0, 0, mz::COLOR_BLACK);
	window->draw_text(font, L"Ἐν ἀρχῇ ἦν ὁ λόγος, καὶ ὁ λόγος ἦν πρὸς τὸν θεόν, καὶ θεός ἦν ὁ λόγος", { -window->get_size().x/2.f + 40, ystart });
	window->draw_text(font, "This is good old regular ASCII. With kerning.", { -window->get_size().x/2.f + 40, ystart-80 });
	window->draw_text(font, "This is good old regular ASCII. Without kerning.", { -window->get_size().x/2.f + 40, ystart-160 }, 1, 0, 0, mz::COLOR_WHITE, false);
	window->draw_text(font, "This is good old regular ASCII. With kerning. Scaled down.", { -window->get_size().x/2.f + 40, ystart-240 }, 0.5f);
	sz = font->measure("This is good old regular ASCII. Without kerning. Scaled up.");
	window->draw_quad({ -window->get_size().x / 2.f + 40, ystart - 320 }, sz * 2, 0, { 0, 12 }, mz::COLOR_BLACK);
	window->draw_text(font, "This is good old regular ASCII. Without kerning. Scaled up.", { -window->get_size().x / 2.f + 40, ystart - 320 }, 2, 0, { 0, 12 }, mz::COLOR_WHITE, false);

	window->render();

	if (window->is_input_down(st::os::INPUT_CODE_ESCAPE)) {
		return 1; // Exit with code 1
	}

	return 0; // Code 0 = don't exit
}
