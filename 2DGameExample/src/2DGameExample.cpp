#include "pch.h"

#include <Stallout/base.h>
#include <Stallout/gameutils.h>
#include <Stallout/runtime.h>

st::Game_Window* window;

st::Texture2D* image;

st::Texture2D* sprite_sheet_texture;
st::Texture_Sheet* sprite_sheet;
st::Sprite* move_sprites[4];

st::Font* font;

export_function(int) init() { 
   
   window = stnew (st::Game_Window) ("2D Game Example");

   // Load single sprite texture
   st::String path = st::get_module_dir();
   path.concat("/sprite.png");
   image = stnew (st::Texture2D)(path.str, window->_gfx_driver);

   // Load sprite sheet texture
   path = st::get_module_dir();
   path.concat("/spritesheet.png");
   sprite_sheet_texture = stnew (st::Texture2D)(path.str, window->_gfx_driver);

   {
	   // Create a sheet from the sprite sheet texture
	   st::Texture_Sheet_Spec spec;
	   spec.cell_size = { 60, 60 };
	   spec.sheet_size = sprite_sheet_texture->_size;
	   spec.texture = sprite_sheet_texture;
	   sprite_sheet = stnew (st::Texture_Sheet)(spec);
   }

   {
	   // Set directional running sprites
	   st::Sprite_Spec spec;
	   spec.fps = 12;
	   spec.sheet = sprite_sheet;

	   // Run down
	   spec.start = sprite_sheet->get_index({ 0, 0 });
	   spec.end = sprite_sheet->get_index({ 7, 0 });
	   move_sprites[0] = stnew (st::Sprite)(spec);

	   // Run left
	   spec.start = sprite_sheet->get_index({ 0, 1 });
	   spec.end = sprite_sheet->get_index({ 7, 1 });
	   move_sprites[1] = stnew (st::Sprite)(spec);

	   // Run right
	   spec.start = sprite_sheet->get_index({ 0, 2 });
	   spec.end = sprite_sheet->get_index({ 7, 2 });
	   move_sprites[2] = stnew (st::Sprite)(spec);

	   // Run up
	   spec.start = sprite_sheet->get_index({ 0, 3 });
	   spec.end = sprite_sheet->get_index({ 7, 3 });
	   move_sprites[3] = stnew (st::Sprite)(spec);
   }

   path = st::os::io::get_workspace_dir();
   path.concat("/proto/arial.ttf");
   font = stnew (st::Font)(window->_gfx_driver, path.str, 24);

   window->clear_color = mz::COLOR_CYAN;

   st::runtime::set_fps_limit(120.f);

   return 0;
}

export_function(int) update(st::Duration frame_time) {

	window->new_frame((f32)frame_time.get_seconds());

	static int dir = 0;
	static f32 speed = 300.f;
	static st::Sprite* current_sprite = NULL;
	static st::vec2 pos = { -100, 50 };

	st::vec2 move = 0;

	if (window->is_input_down(st::os::INPUT_CODE_S)) {
		move.y -= 1.f;
	}
	if (window->is_input_down(st::os::INPUT_CODE_A)) {
		move.x -= 1.f;
	}
	if (window->is_input_down(st::os::INPUT_CODE_D)) {
		move.x += 1.f;
	}
	if (window->is_input_down(st::os::INPUT_CODE_W)) {
		move.y += 1.f;
	}

	if (move.y > 0) dir = 3;
	if (move.y < 0) dir = 0;
	if (move.x > 0) dir = 2;
	if (move.x < 0) dir = 1;

	current_sprite = move_sprites[dir];

	pos += move * speed * frame_time.get_secondsf();

	window->draw_sprite(current_sprite, pos);
	window->draw_texture(image, 0, { 64, 64 });

	window->draw_text(font, "This is a simple 2D Game Example!", { -window->get_size().x / 2.f + 48, window->get_size().y / 2.f - 48 });
	
	static st::String fps_string = "";
	fps_string = "";
	fps_string.concat("FPS: %.1f", 1.f / frame_time.get_secondsf());
	window->draw_text(font, fps_string, { -window->get_size().x / 2.f + 48, window->get_size().y / 2.f - 96 });
	log_debug("{}", window->should_close());
	window->render();

	if (window->is_input_down(st::os::INPUT_CODE_ESCAPE) || window->should_close()) {
		ST_DELETE(window);
		return 1; // Exit with code 1
	}

	return 0; // Code 0 = don't exit
}
