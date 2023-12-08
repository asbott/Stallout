#include "pch.h"

#include <Stallout/base.h>
#include <Stallout/prototyping.h>
#include <Stallout/gameutils.h>


export_function(int) update(st::Duration frame_time) {

	static st::vec2 tex_pos;
	const f32 tex_speed = 400.f;

	st::vec2 move;
	if (st::os::is_input_down(st::os::INPUT_CODE_W)) move.y += 1;
	if (st::os::is_input_down(st::os::INPUT_CODE_S)) move.y -= 1;
	if (st::os::is_input_down(st::os::INPUT_CODE_D)) move.x += 1;
	if (st::os::is_input_down(st::os::INPUT_CODE_A)) move.x -= 1;

	tex_pos += move * tex_speed * frame_time.get_secondsf();

	st::draw_image("proto/hello-world.png", tex_pos, { 256, 256 });

	st::draw_quad({ 100, 200, 164, 264 }, mz::COLOR_LIGHTGREEN);
	st::draw_quad({ -200, 0, -136, 64 }, mz::COLOR_LIGHTBLUE);
	st::draw_wire_quad({ -100, -100, -50, -50 }, mz::COLOR_WHITE);
	st::draw_wire_arrow({ -200, -100 }, { -150, -120 }, mz::COLOR_WHITE);
	st::draw_wire_circle({ 0, 0 }, 32, mz::COLOR_WHITE);

	st::draw_text("This is rendered text", { 300, 0 }, 24, mz::COLOR_CHARTREUSE);

	if (st::proto::get_window()->should_close()) {

		return 1;
	}

	return 0;
}
