#pragma once //

#include "Stallout/containers.h"
#include "Stallout/maths.h"
#include "Stallout/graphics.h"

NS_BEGIN(stallout);

struct ST_API Texture2D {
    Texture2D(const char* filepath, graphics::Graphics_Driver* driver);
    Texture2D(mz::color* data, mz::ivec2 size, graphics::Graphics_Driver* driver);
    ~Texture2D();

    mz::ivec2 _size;
    graphics::Resource_Handle _handle;
    bool ready = false, error = false;
    graphics::Graphics_Driver* _driver;
};


struct Texture_Sheet_Spec {
    mz::uvec2 cell_size;
    Texture2D* texture;
    mz::uvec2 sheet_size;
};
struct ST_API Texture_Sheet {
    const Texture_Sheet_Spec config;
    const u32 xcount, ycount, total_count;
    Array<mz::frect> _uvs;

    Texture_Sheet(const Texture_Sheet_Spec& spec);

    mz::frect get_uv(mz::uvec2 cell_index) const;
    mz::frect get_uv(u32 cell_index) const;
    u32 get_index(mz::uvec2 index) const;
    Array<mz::fvec4> get_lines() const;
};

struct ST_API Sprite_Spec {
    Texture_Sheet* sheet;
    u32 fps, start, end;
};
struct ST_API Sprite {

    const Sprite_Spec spec;
    const f32 _cycle_duration;
    Timer _timer;

    Sprite(const Sprite_Spec& spec);

    void pause();
    void reset();
    void play();

    mz::frect get_uv() const;
    Texture2D* get_texture() const;
    mz::fvec2 get_size() const { return spec.sheet->config.cell_size; }
};



// TODO: #unicode
struct ST_API Font {

    

    static constexpr s32 CHARS_PER_ATLAS = 128;
    static constexpr lchar PLACEHOLDER_CHAR = 0;

    struct Char_Info {
        mz::ivec2 atlas_pos;
        mz::ivec2 offset;
        mz::ivec2 size;
        f32 advance;
        mz::fvec4 uv;
        bool has_bitmap = false;

        void* _pixels; // Used for temporary storage. Should be NULL after loading.
    };

    struct Atlas {
        gfx::Resource_Handle texture;
        s32 range_start, range_end;
        mz::ivec2 size;

        bool rendered = false;

        Char_Info chars[CHARS_PER_ATLAS];
    };


    Array<Atlas> _atlases;
    
    void* _ft_face;
    gfx::Graphics_Driver *const gfx_driver;
    const u32 font_size;

    Font(gfx::Graphics_Driver* gfx_driver, const char* path, u32 font_size = 14);
    ~Font();

    bool assure_rendered(const lchar c);
    bool assure_rendered(const String& s);
    bool assure_rendered(const LString& s);

    mz::fvec2 get_kerning(lchar a, lchar b);

    mz::fvec4 get_uv(lchar c);
    mz::fvec2 measure(lchar c, lchar next = -1);
    mz::fvec2 measure(const char* s);
    mz::fvec2 measure(const String& s);
    mz::fvec2 measure(const lchar* s);
    mz::fvec2 measure(const LString& s);

    const Char_Info& _get_char_info(lchar c);
    const Atlas& _get_atlas(lchar c);
};

NS_END(stallout)