#pragma once
#include <core/math.h>
#include <gfx/brush.h>
#include <gfx/image.h>
#include <map>

namespace arcaie::gfx
{

enum class font_align : long
{
    LEFT = 1L << 0,
    RIGHT = 1L << 1,
    H_CENTER = 1L << 2,
    UP = 1L << 3,
    DOWN = 1L << 4,
    V_CENTER = 1L << 5
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
    double height = 0;
    double lspc = 0;
    double ascend = 0;
    double descend = 0;

    struct P_impl;
    unique<P_impl> P_pimpl;

    font();
    ~font();

    glyph get_glyph(u32_char ch)
    {
        if (glyph_map.find(ch) == glyph_map.end())
            return glyph_map[ch] = make_glyph(ch);
        return glyph_map[ch];
    }

    glyph make_glyph(u32_char ch);
    // draw the stringin the font, return the bounding box.
    // and if brush is nullptr, it won't draw anything, just calculating the bounding box.
    font_render_bound make_vtx(brush *brush, const std::string &str, double x, double y,
                               bitmask<font_align> align = mask(font_align::UP, font_align::LEFT),
                               double max_w = INT_MAX, double scale = 1);
    font_render_bound make_vtx(brush *brush, const std::u32string &str, double x, double y,
                               bitmask<font_align> align = mask(font_align::UP, font_align::LEFT),
                               double max_w = INT_MAX, double scale = 1);
};

shared<font> load_font(const path_handle &path, double res_h, double pixel_h);

} // namespace arcaie::gfx
