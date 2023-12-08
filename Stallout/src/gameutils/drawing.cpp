#include "pch.h"

#include "Stallout/gameutils/drawing.h"
#include "Stallout/os/io.h"

NS_BEGIN(stallout)


Texture2D::Texture2D(const char* filepath, graphics::Graphics_Driver* driver) : _driver(driver) {
    int c;
    void* data = utils::load_image_from_file(filepath, &_size.x, &_size.y, &c, 4);

    if (!data) {
        _handle = 0;
        _size = 0;
        error = true;
        log_error("Failed loading texture at '{}'", filepath);
        return;
    }


    _handle = driver->create_texture(
        _size.x, _size.y, 4, 
        gfx::TEXTURE2D_FORMAT_RGBA,
        gfx::DATA_TYPE_UBYTE,
        gfx::TEXTURE2D_FORMAT_RGBA,
        gfx::TEXTURE_FILTER_MODE_LINEAR,
        gfx::TEXTURE_FILTER_MODE_LINEAR,
        gfx::MIPMAP_MODE_LINEAR
    );
    // TODO (2023-11-02): #unfinished
    // Graphics driver does not yet have a way to handle errors
    // for resources.
    // When it's implemented, set a ready flag to true once
    // the texture is created and set, or leave at false and
    // set an error flag if it fails. When rendering this
    // texture it should check if it's ready and if not, render
    // a placeholder or error texture.
    driver->set_texture2d(_handle, data, _size.x * _size.y * 4);
    ready = true;

    utils::free_image(data);
}
Texture2D::Texture2D(mz::color* data, mz::ivec2 size, graphics::Graphics_Driver* driver) : _driver(driver) {
    if (!data) {
        _handle = 0;
        _size = 0;
        error = true;
        return;
    }

    _size = size;

    _handle = driver->create_texture(_size.x, _size.y, 4, graphics::TEXTURE2D_FORMAT_RGBA);
    ready = true;
    // TODO (2023-11-02): #unfinished
    // Same as above
    driver->set_texture2d(_handle, data, _size.x * _size.y * 4);
}
Texture2D::~Texture2D() {
    if (!this->error) {
        _driver->destroy(_handle);
    }
}


Texture_Sheet::Texture_Sheet(const Texture_Sheet_Spec& spec) :
    config(spec), 
    xcount(spec.sheet_size.x / spec.cell_size.x),
    ycount(spec.sheet_size.y / spec.cell_size.y),
    total_count((spec.sheet_size.x / spec.cell_size.x) * (spec.sheet_size.y / spec.cell_size.y)) {
    
    _uvs.reserve(total_count);

    for (u32 i = 0; i < total_count; i++) {
        u32 x = i % xcount;
        u32 y = i / xcount;

        f32 x1 = (f32)((x * config.cell_size.x) / (f32)config.sheet_size.x);
        f32 y1 = (f32)((y * config.cell_size.y) / (f32)config.sheet_size.y);
        f32 x2 = (f32)(x1 + (config.cell_size.x) / (f32)config.sheet_size.x);
        f32 y2 = (f32)(y1 + (config.cell_size.y) / (f32)config.sheet_size.y);

        _uvs.push_back({ x1, y1, x2, y2 });
    }
}

mz::frect Texture_Sheet::get_uv(mz::uvec2 cell_index) const {
    return _uvs[this->get_index(cell_index)];
}
mz::frect Texture_Sheet::get_uv(u32 index) const {
    return _uvs[index];
}
Array<mz::fvec4> Texture_Sheet::get_lines() const {
    Array<mz::fvec4> lines;
    for (u32 j = 0; j <= xcount; ++j) {
        f32 x = j * (f32)config.cell_size.x;
        lines.push_back(mz::fvec4(x, 0, x, ycount * config.cell_size.y));
    }

    for (u32 i = 0; i <= ycount; ++i) {
        f32 y = i * (f32)config.cell_size.y;
        lines.push_back(mz::fvec4(0, y, xcount * config.cell_size.x, y));
    }

    return lines;
}

u32 Texture_Sheet::get_index(mz::uvec2 cell_index) const {
    return cell_index.y * xcount + cell_index.x;
}


Sprite::Sprite(const Sprite_Spec& spec) :
    spec(spec),
    _cycle_duration(1.f / spec.fps) {
}

void Sprite::pause() {
    _timer.pause();
}
void Sprite::reset() {
    _timer.reset();
}
void Sprite::play() {
    _timer.resume();
}

mz::frect Sprite::get_uv() const {
    u32 index = (u32)(_timer.record().get_seconds() / _cycle_duration) % (spec.end - spec.start + 1) + spec.start;

    return spec.sheet->get_uv(index);
}
Texture2D* Sprite::get_texture() const {
    return spec.sheet->config.texture;
}

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H
#include FT_GLYPH_H

#ifndef _ST_DISABLE_ASSERTS
#define FT_CALL(x) do { auto err = x; (void)err; ST_DEBUG_ASSERT(!err, "FreeType2 Font Loading error: {}", FT_Error_String(err)); } while (0)
#endif

FT_Library ft = NULL;

Font::Font(gfx::Graphics_Driver* gfx_driver, const char* path, u32 font_size) : gfx_driver(gfx_driver), font_size(font_size) {

    // TODO (2023-12-03): #errorhandling

    if (!ft) {
        FT_CALL(FT_Init_FreeType(&ft));
    }

    FT_Face face;
    FT_CALL(FT_New_Face(ft, path, 0, &face));
    FT_CALL(FT_Set_Pixel_Sizes(face, 0, font_size));

    _ft_face = face;

    ST_ASSERT(assure_rendered(PLACEHOLDER_CHAR));
}

Font::~Font() {
    for (auto& atlas : _atlases) {
        gfx_driver->destroy(atlas.texture);
    }

    FT_Face face = (FT_Face)_ft_face;
    FT_Done_Face(face);
}

bool Font::assure_rendered(const lchar c) {
    if (c < 0) return false;

    // Skipping surrogate range (U+D800 to U+DFFF)
    if (c >= 55296 && c <= 57343) {
        return false;
    }

    // Skipping noncharacters in BMP (U+FDD0 to U+FDEF and last two codepoints of each plane)
    if ((c >= 64976 && c <= 65007) || (c & 0xFFFE) == 0xFFFE) {
        return false;
    }

    // Skipping Private Use Areas
    if ((c >= 57344 && c <= 63743) || // BMP Private Use Area (U+E000 to U+F8FF)
        (c >= 983040 && c <= 1048573) || // Supplementary Private Use Area-A (U+F0000 to U+FFFFD)
        (c >= 1048576 && c <= 1114109)) { // Supplementary Private Use Area-B (U+100000 to U+10FFFD)
        return false;
    }

    // Skipping Plane 3 to 16 (except the Private Use Areas already handled)
    if (c >= 196608 && c <= 1114111) {
        return false;
    }

    s32 atlas_index = c / CHARS_PER_ATLAS;

    if (atlas_index >= _atlases.size()) {
        _atlases.resize(atlas_index + 1);
    }

    auto& atlas = _atlases[atlas_index];

    if (!atlas.rendered) {

        atlas.range_start = atlas_index * CHARS_PER_ATLAS;
        atlas.range_end = atlas.range_start + CHARS_PER_ATLAS;

        FT_Face face = (FT_Face)_ft_face;

        atlas.size = {0, font_size};

        int x = 0;
        //int y = 0;

        for (FT_UInt i = (FT_UInt)atlas.range_start; i < (FT_UInt)atlas.range_end; i++) {
            
            FT_CALL(FT_Load_Char(face, i, FT_LOAD_RENDER));

            FT_UInt glyph_index = FT_Get_Char_Index(face, i);
            if (glyph_index == 0 && i != PLACEHOLDER_CHAR) {
                continue;
            }

            auto adv = face->glyph->advance.x >> 6;

            Char_Info cinfo;
            
            cinfo.has_bitmap = true;
            cinfo.size = { face->glyph->bitmap.width, math::clamp<u32>(face->glyph->bitmap.rows, 0, font_size) };
            cinfo.offset = { face->glyph->bitmap_left, -(cinfo.size.y - face->glyph->bitmap_top) };
            cinfo.atlas_pos = { x, 0 };
            cinfo._pixels = ST_MEM(cinfo.size.x * cinfo.size.y);
            cinfo.advance = (f32)adv;
            memcpy(cinfo._pixels, face->glyph->bitmap.buffer, cinfo.size.x * cinfo.size.y);
            
            size_t local_index = i % (atlas.range_end - atlas.range_start);

            atlas.chars[local_index] = cinfo;

            x += math::max<s32>(adv, cinfo.size.x);
        }

        // No glyphs found
        if (!x) {
            atlas.rendered = true;
            return false;
        }

        atlas.size.x = x;

        atlas.texture = gfx_driver->create_texture(
            (u32)atlas.size.x, (u32)atlas.size.y, 1,
            gfx::TEXTURE2D_FORMAT_RED, 
            gfx::DATA_TYPE_UBYTE, 
            gfx::TEXTURE2D_FORMAT_RED_NORM_BYTE, 
            gfx::TEXTURE_FILTER_MODE_NEAREST, 
            gfx::TEXTURE_FILTER_MODE_LINEAR, 
            gfx::MIPMAP_MODE_NONE,
            gfx::TEXTURE_WRAP_MODE_REPEAT
        );

        size_t atlas_mem = (size_t)(atlas.size.x * atlas.size.y);
        gfx_driver->set_texture2d(atlas.texture, 0, atlas_mem);

        for (auto& cinfo : atlas.chars) {
            if (!cinfo.has_bitmap) continue;

            cinfo.uv = {
                (f32)cinfo.atlas_pos.x / (f32)atlas.size.x,
                (f32)cinfo.atlas_pos.y / (f32)atlas.size.y,
                (f32)(cinfo.atlas_pos.x + cinfo.size.x) / (f32)atlas.size.x,
                (f32)(cinfo.atlas_pos.y + cinfo.size.y) / (f32)atlas.size.y
            };

            gfx_driver->append_to_texture2d(
                atlas.texture, 
                (u32)cinfo.atlas_pos.x, 
                (u32)cinfo.atlas_pos.y, 
                (u32)cinfo.size.x, 
                (u32)cinfo.size.y,
                1,
                cinfo._pixels,
                gfx::TEXTURE2D_FORMAT_RED, gfx::DATA_TYPE_UBYTE // Format of char_pixels
            );

            ST_FREE(cinfo._pixels, cinfo.size.x * cinfo.size.y);
            cinfo._pixels = NULL;
        }

        atlas.rendered = true;
    }

    return atlas.chars[c % (atlas.range_end - atlas.range_start)].has_bitmap;
}

bool Font::assure_rendered(const String& s) {
    char* p = s.str;
    bool any_fail = false;
    while (*p != 0) {
        if (!assure_rendered((lchar)*p)) any_fail = true;
        p++;
    }

    return any_fail;
}
bool Font::assure_rendered(const LString& s) {
    lchar* p = s.str;
    bool any_fail = false;
    while (*p != 0) {
        if (!assure_rendered(*p)) any_fail = true;
        p++;
    }
    return any_fail;
}
mz::fvec2 Font::get_kerning(lchar a, lchar b) {
    FT_Vector ftkern;
    FT_CALL(FT_Get_Kerning((FT_Face)_ft_face, (FT_UInt)a, (FT_UInt)b, FT_KERNING_UNFITTED, &ftkern));

    return { ftkern.x, ftkern.y };
}


mz::fvec4 Font::get_uv(lchar c) {
    return _get_char_info(c).uv;
}
mz::fvec2 Font::measure(lchar c, lchar next) {
    mz::fvec2 kern = next >= 0 ? get_kerning(c, next) : mz::fvec2(0);
    auto& cinfo = _get_char_info(c);
    f32 x = next >= 0 ? cinfo.advance : cinfo.size.x;
    return mz::fvec2(x, cinfo.size.y) + kern; 
}
mz::fvec2 Font::measure(const char* s) {
    const auto* p = s;

    mz::fvec2 m = 0;
    while (*p != 0) {
        auto sz = measure((lchar)*p, *(p + 1) ? *(p + 1) : -1);
        m = mz::fvec2(m.x + sz.x, math::max(m.y, sz.y));
        p++;
    }
    return m;
}
mz::fvec2 Font::measure(const String& s) {
    return measure(s.str);
}
mz::fvec2 Font::measure(const lchar* s) {
    const auto* p = s;

    mz::fvec2 m = 0;
    while (*p != 0) {
        auto sz = measure((lchar)*p, *(p + 1) ? *(p + 1) : -1);
        m = mz::fvec2(m.x + sz.x, math::max(m.y, sz.y));
        p++;
    }
    return m;
}
mz::fvec2 Font::measure(const LString& s) {
    return measure(s.str);
}

const Font::Char_Info& Font::_get_char_info(lchar c) {

    if (assure_rendered(c)) {
        auto atlas_index = c / CHARS_PER_ATLAS;

        return _atlases[atlas_index].chars[c % CHARS_PER_ATLAS];
    }
    return _get_char_info(PLACEHOLDER_CHAR);
}

const Font::Atlas& Font::_get_atlas(lchar c) {
    if (assure_rendered(c)) {
        return _atlases[c / CHARS_PER_ATLAS];
    } else {
        return _atlases[PLACEHOLDER_CHAR / CHARS_PER_ATLAS];
    }
}

NS_END(stallout)