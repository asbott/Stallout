#pragma once //

#include "Stallout/maths.h"
#include "Stallout/strings.h"

NS_BEGIN(stallout);
struct Game_Window;

NS_BEGIN(proto);

constexpr u32 DEFAULT_FONT_SIZE = 24;

ST_API void draw_quad(mz::frect aabb, mz::color color = mz::COLOR_WHITE);
ST_API void draw_image(const char* path, mz::fvec2 position, mz::fvec2 size = 0, mz::color color = mz::COLOR_WHITE);
ST_API void draw_text(const String& text, mz::fvec2 position, f32 font_size = DEFAULT_FONT_SIZE, mz::color color = mz::COLOR_WHITE);

ST_API void draw_line(mz::fvec2 a, mz::fvec2 b, mz::color color = mz::COLOR_WHITE);
ST_API void draw_wire_quad(mz::frect aabb, mz::color color = mz::COLOR_WHITE);
ST_API void draw_wire_arrow(mz::fvec2 a, mz::fvec2 b, mz::color color = mz::COLOR_WHITE);
ST_API void draw_wire_circle(mz::fvec2 pos, f32 radius, mz::color color = mz::COLOR_WHITE);


ST_API void set_window(Game_Window* wnd);
ST_API Game_Window* get_window();
ST_API bool need_window_management();
ST_API void shutdown();

NS_END(proto);
using namespace proto;
NS_END(stallout);