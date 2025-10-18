#pragma once
#include <core/io.h>
#include <core/math.h>
#include <gfx/brush.h>
#include <gfx/image.h>
#include <unordered_map>


namespace arc::gfx
{

struct font_align
{
    constexpr static long LEFT = 1L << 0;
    constexpr static long RIGHT = 1L << 1;
    constexpr static long H_CENTER = 1L << 2;
    constexpr static long UP = 1L << 3;
    constexpr static long DOWN = 1L << 4;
    constexpr static long V_CENTER = 1L << 5;

    constexpr static long NORMAL = LEFT | UP;
    constexpr static long NORMAL_CENTER = H_CENTER | UP;
};

struct glyph
{
    std::shared_ptr<texture> texpart;
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
    struct P_impl;
    std::unique_ptr<P_impl> P_pimpl;

    std::unordered_map<char32_t, glyph> glyph_map;
    double height = 0;
    double lspc = 0;
    double ascend = 0;
    double descend = 0;

    font();
    ~font();

    glyph get_glyph(char32_t ch)
    {
        if (glyph_map.find(ch) == glyph_map.end())
            return glyph_map[ch] = make_glyph(ch);
        return glyph_map[ch];
    }

    glyph make_glyph(char32_t ch);
    // draw the stringin the font, return the bounding box.
    // and if brush is nullptr, it won't draw anything, just calculating the bounding box.
    font_render_bound make_vtx(std::shared_ptr<brush> brush, const std::string &str, double x, double y,
                               long align = font_align::NORMAL, double max_w = INT_MAX, double scale = 1);
    font_render_bound make_vtx(std::shared_ptr<brush> brush, const std::u32string &str, double x, double y,
                               long align = font_align::NORMAL, double max_w = INT_MAX, double scale = 1);

    static std::shared_ptr<font> load(const path_handle &path, double res_h, double pixel_h);
};

} // namespace arc::gfx
