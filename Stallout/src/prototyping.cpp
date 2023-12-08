#include "pch.h"

#include "prototyping.h"

#include "gameutils.h"

#include "os/io.h"

NS_BEGIN(stallout);
NS_BEGIN(proto);

Game_Window* window = NULL;
bool window_owned = false;

Hash_Map<const char*, Texture2D*> loaded_textures;

Font* arial_font = NULL;

void ensure_ready_to_draw() {
    if (!window) {
        window = stnew (Game_Window)("Prototyping Window");
        window->new_frame(1.f / 60.f);
        window_owned = true;
    }
}

void draw_quad(mz::frect quad, mz::color color) {
    ensure_ready_to_draw();

    window->draw_quad(quad.xy, quad.zw - quad.xy, 0, (quad.zw - quad.xy) / 2.f, color);
}
void draw_image(const char* path, mz::fvec2 position, mz::fvec2 size, mz::color color) {
    ensure_ready_to_draw();

    Texture2D* tex = NULL;

    if (loaded_textures.contains(path)) {
        tex =  loaded_textures[path];
    } else {
        auto wks = os::io::get_workspace_dir();
        wks.concat("/%s", path);

        tex = stnew (Texture2D)(wks.str, window->_gfx_driver);
        
        if (tex->error) {
            ST_DELETE(tex);
            tex = NULL;
        } else {
            char* str = (char*)ST_MEM(strlen(path)+1);
            strcpy(str, path);
            loaded_textures[str] = tex;
        }
    }

    if (tex) {
        size = size.magnitude() ? size : (mz::fvec2)tex->_size;
        window->draw_texture(tex, position, size, 0, size / 2.f, color);
    } else {
        draw_wire_quad({ position.x - 32, position.y - 32, position.x + 32, position.y + 32 }, mz::COLOR_RED);
    }
}

void draw_text(const String& text, mz::fvec2 position, f32 font_size, mz::color color) {
    if (!arial_font) {
        String path = os::io::get_workspace_dir();
        path.concat("/proto/arial.ttf");

        arial_font = stnew (Font)(window->_gfx_driver, path.str, DEFAULT_FONT_SIZE);
    }

    window->draw_text(arial_font, text, position, font_size / DEFAULT_FONT_SIZE, 0, 0, color);
}

void draw_line(mz::fvec2 a, mz::fvec2 b, mz::color color) {
    ensure_ready_to_draw();

    window->debug_draw_line(a, b, color);
}
void draw_wire_quad(mz::frect quad, mz::color color) {
    ensure_ready_to_draw();
    window->debug_draw_wire_frame(quad, color);
}
void draw_wire_arrow(mz::fvec2 a, mz::fvec2 b, mz::color color) {
    ensure_ready_to_draw();
    window->debug_draw_arrow(a, b, color);
}
void draw_wire_circle(mz::fvec2 pos, f32 radius, mz::color color) {
    ensure_ready_to_draw();
    window->debug_draw_circle(pos, radius, color);
}

void set_window(Game_Window* wnd) {
    if (window_owned) {
        ST_DELETE(window);
        window = NULL;
    }

    window_owned = false;
    window = wnd;
}
Game_Window* get_window() {
    return window;
}
bool need_window_management() {
    return window_owned;
}
void shutdown() {
    if (window_owned) {
        ST_DELETE(window);
        window = NULL;
        window_owned = false;
    }

    for (auto [path, tex] : loaded_textures) {
        ST_FREE(const_cast<char*>(path), strlen(path) + 1);
        ST_DELETE(tex);
    }
    loaded_textures.clear();
}

NS_END(proto);
NS_END(stallout);