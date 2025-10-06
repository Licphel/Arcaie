#pragma once
#include <core/math.h>
#include <core/math.h>
#include <gfx/image.h>
#include <gfx/brush.h>
#include <map>

namespace arcaie::gfx
{

enum font_render_align
{
    ARC_FONT_LEFT_ALIGN = 1LL << 0,
    ARC_FONT_RIGHT_ALIGN = 1LL << 1,
    ARC_FONT_H_CENTER_ALIGN = 1LL << 2,
    ARC_FONT_UP_ALIGN = 1LL << 3,
    ARC_FONT_DOWN_ALIGN = 1LL << 4,
    ARC_FONT_V_CENTER_ALIGN = 1LL << 5
};

struct glyph
{
    shared<texture> texpart;
    vec2 size;
    double advance;
    vec2 offset;

    // scale the glyph.
    inline glyph operator*(double scl)
    {
        return {
            texpart,
            size * scl,
            advance * scl,
            offset * scl,
        };
    }
};

struct font_render_bound
{
    quad region;
    double last_width;
    int lines;
};

struct font
{
    std::map<u32_char, glyph> glyph_map;
    double f_height = 0;
    double f_lspc = 0;
    double f_ascend = 0;
    double f_descend = 0;

    struct _impl;
    unique<_impl> __p;

    font();
    ~font();

    glyph get_glyph(u32_char ch)
    {
        if (glyph_map.find(ch) == glyph_map.end())
            return glyph_map[ch] = make_glyph(ch);
        return glyph_map[ch];
    }

    glyph make_glyph(u32_char ch);
    // draw the std::stringin the font, return the bounding box.
    // and if brush is nullptr, it won't draw anything, just calculating the bounding box.
    font_render_bound make_vtx(brush *brush, const std::string &str, double x, double y, double scale = 1,
                               double max_w = INT_MAX);
};

shared<font> load_font(const hio_path &path, double res_h, double pixel_h);

} // namespace arcaie::gfx
