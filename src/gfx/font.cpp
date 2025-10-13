#include <core/chcvt.h>
#include <core/log.h>
#include <gfx/atlas.h>
#include <gfx/font.h>
#include <gfx/image.h>
#include <gfx/brush.h>
#include <core/io.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

namespace arc::gfx
{

struct font::P_impl
{
    FT_FaceRec_ *face_ptr;
    FT_LibraryRec_ *lib_ptr;
    std::unordered_map<int, std::shared_ptr<atlas>> codemap;
    double res, pix;
};

font::font() : P_pimpl(std::make_unique<P_impl>())
{
}

font::~font()
{
    FT_Done_Face(P_pimpl->face_ptr);
    FT_Done_FreeType(P_pimpl->lib_ptr);
}

std::shared_ptr<texture> P_flush_codemap(font::P_impl *P_p, u32_char ch, std::shared_ptr<image> img)
{
    int code = (int)floor(static_cast<int>(ch) / 256.0);
    if (P_p->codemap.find(code) == P_p->codemap.end())
    {
        int ats = P_p->res * 16;
        P_p->codemap[code] = atlas::make(ats, ats);
        P_p->codemap[code]->begin();
    }

    auto atl = P_p->codemap[code];
    auto tptr = atl->accept(img);
    atl->end();
    return tptr;
}

glyph font::make_glyph(u32_char ch)
{
    auto face = P_pimpl->face_ptr;
    unsigned int idx = FT_Get_Char_Index(face, ch);

    FT_Set_Pixel_Sizes(face, 0, P_pimpl->res);
    FT_Load_Glyph(face, idx, FT_LOAD_DEFAULT);
    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    FT_Bitmap_ m0 = face->glyph->bitmap;

    int len = (int)(m0.width * m0.rows * 4);
    byte *buf = new byte[len];
    for (int i = 0; i < len; i += 4)
    {
        byte grey = m0.buffer[i / 4];
        buf[i + 0] = 255;
        buf[i + 1] = 255;
        buf[i + 2] = 255;
        buf[i + 3] = grey;
    }

    std::shared_ptr<image> img = image::make((int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows, buf);

    double ds = P_pimpl->res / P_pimpl->pix;

    glyph g;
    g.texpart = P_flush_codemap(P_pimpl.get(), ch, img);
    g.texpart->parameters(texture_parameters(texture_parameter::UV_CLAMP, texture_parameter::FILTER_LINEAR,
                                             texture_parameter::FILTER_LINEAR));
    g.size.x = ch == ' ' ? face->glyph->metrics.horiAdvance / ds / 64.0 : face->glyph->metrics.width / ds / 64.0;
    g.size.y = face->glyph->metrics.height / ds / 64.0;
    g.advance = face->glyph->advance.x / ds / 64.0;
    g.offset.x = face->glyph->metrics.horiBearingX / ds / 64.0;
#ifdef ARC_Y_IS_DOWN
    g.offset.y = -face->glyph->metrics.horiBearingY / ds / 64.0 + height + face->bbox.yMin / ds / 64.0;
#else
    g.offset.y = face->glyph->metrics.horiBearingY / ds / 64.0 - g.size.y;
#endif

    return g;
}

font_render_bound font::make_vtx(std::shared_ptr<brush> brush, const std::string &u8_str, double x, double y,
                                 long align, double max_w, double scale)
{
    static std::u32string P_cvtbuf;
    P_cvt_u32(u8_str, &P_cvtbuf);
    return make_vtx(brush, P_cvtbuf, x, y, align, max_w, scale);
}

font_render_bound font::make_vtx(std::shared_ptr<brush> brush, const std::u32string &str, double x, double y,
                                 long align, double max_w, double scale)
{
    if (str.length() == 0 || str.length() > INT16_MAX)
        return {};

    if (align & font_align::LEFT && align & font_align::UP)
    {
        double h_scaled = scale * lspc;
        double w = 0;
        double lw = 0;
        double h = 0;
        double lh = 0;
        double dx = x;
        double dy = y;
        bool endln = false;
        int lns = 1;

        for (int i = 0; i < (int)str.length(); i++)
        {
            u32_char ch = str[i];

            if (ch == '\n' || endln)
            {
#ifdef ARC_Y_IS_DOWN
                dy += h_scaled;
#else
                dy -= h_scaled;
#endif
                dx = x;
                endln = false;
                w = std::max(w, lw);
                lw = 0;
                h += h_scaled;
                lh = 0;
                lns++;
                continue;
            }

            glyph g = get_glyph(ch) * scale;

            if (dx - x + g.advance >= max_w)
            {
                endln = true;
                i -= 1;
                i = std::max(0, i - 1);
                continue;
            }

            lh = std::max(g.size.y, lh);
            lw += g.advance;

            if (brush != nullptr)
                brush->draw_texture(g.texpart, {dx + g.offset.x, dy + g.offset.y, g.size.x, g.size.y});
            dx += g.advance;

            if (i == (int)str.length() - 1 || (get_glyph(str[i + 1]) * scale).advance + dx - x >= max_w)
                lw += g.size.x - g.advance;
        }

#ifdef ARC_Y_IS_DOWN
        return font_render_bound(quad::corner(x, y, std::max(lw, w), h + height * scale), lw, lns);
#else
        return font_render_bound(quad::corner(x, dy, std::max(lw, w), h + f_height * scale), lw, lns);
#endif
    }

    // align the positions
    auto qd = make_vtx(nullptr, str, x, y, font_align::NORMAL, max_w, scale);
    double w = qd.region.width, h = qd.region.height;

    if (align & font_align::DOWN)
        y -= h;
    if (align & font_align::V_CENTER)
        y -= h / 2;
    if (align & font_align::RIGHT)
        x -= w;
    if (align & font_align::H_CENTER)
        x -= w / 2;

    return make_vtx(brush, str, x, y, font_align::NORMAL, max_w, scale);
}

std::shared_ptr<font> font::load(const path_handle &path, double res_h, double pixel_h)
{
    auto fptr = std::make_shared<font>();

    FT_LibraryRec_ *lib;
    FT_FaceRec_ *face;
    FT_Init_FreeType(&lib);
    FT_New_Face(lib, path.abs_path.c_str(), 0, &face);
    FT_Select_Charmap(face, FT_ENCODING_UNICODE);

    fptr->P_pimpl->face_ptr = face;
    fptr->P_pimpl->lib_ptr = lib;
    fptr->P_pimpl->res = res_h;
    fptr->P_pimpl->pix = pixel_h;
    fptr->height = pixel_h;
    fptr->ascend = face->ascender / 64.0;
    fptr->descend = face->descender / 64.0;
    fptr->lspc = pixel_h + 1;

    return fptr;
}

} // namespace arc::gfx
