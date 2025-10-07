#include <gfx/image.h>
#include <core/log.h>
#include <gl/glew.h>
#include <gl/gl.h>
#include <gfx/brush.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace arcaie::gfx
{

image::image() = default;

image::image(byte *data, int w, int h) : width(w), height(h), pixels(data)
{
}

image::~image()
{
    if (__is_from_stb)
        stbi_image_free(pixels);
    else
        delete[] pixels;
    pixels = nullptr;
}

shared<image> load_image(const hio_path &path)
{
    shared<image> img = std::make_shared<image>();
    img->pixels = stbi_load(path.absolute.c_str(), &img->width, &img->height, nullptr, 4);
    if (img->pixels == nullptr)
        prtlog_throw(ARC_FATAL, "path not found: {}", path.absolute);
    img->__is_from_stb = true;
    return img;
}

shared<image> make_image(int width, int height, byte *data)
{
    shared<image> img = std::make_shared<image>();
    img->width = width;
    img->height = height;
    img->pixels = data;
    img->__is_from_stb = false;
    return img;
}

texture::~texture()
{
    if (root == nullptr)
        glDeleteTextures(1, &__texture_id);
}

shared<texture> make_texture(shared<image> img)
{
    shared<texture> tex = std::make_shared<texture>();
    unsigned int id;
    glGenTextures(1, &id);
    tex->__texture_id = id;

    if (img != nullptr)
        lazylink_texture_data(tex, img);

    return tex;
}

void set_texture_parameters(shared<texture> tex, texture_parameters param)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->__texture_id);

    GLenum mode0;
    switch (param.uv)
    {
    case texture_parameter::UV_CLAMP:
        mode0 = GL_CLAMP_TO_EDGE;
        break;
    case texture_parameter::UV_MIRROR:
    default:
        mode0 = GL_MIRRORED_REPEAT;
        break;
    case texture_parameter::UV_REPEAT:
        mode0 = GL_REPEAT;
        break;
    }
    GLenum mode1 = param.min_filter == texture_parameter::FILTER_LINEAR ? GL_LINEAR : GL_NEAREST;
    GLenum mode2 = param.mag_filter == texture_parameter::FILTER_LINEAR ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode2);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void lazylink_texture_data(shared<texture> tex, shared<image> img)
{
    tex->__relying_image = img;
    tex->fwidth = tex->width = img->width;
    tex->fheight = tex->height = img->height;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->__texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);

    set_texture_parameters(tex, {});

    glBindTexture(GL_TEXTURE_2D, 0);
}

shared<texture> cut_texture(shared<texture> tex, const quad &src)
{
    shared<texture> ntex = std::make_shared<texture>();
    ntex->width = src.width;
    ntex->height = src.height;
    ntex->fwidth = tex->fwidth;
    ntex->fheight = tex->fheight;
    ntex->u = src.x;
    ntex->v = src.y;
    ntex->root = tex;
    ntex->__relying_image = tex->__relying_image;
    ntex->__texture_id = tex->__texture_id;
    ntex->__is_framebuffer = tex->__is_framebuffer;
    return ntex;
}

void bind_texture(int unit, shared<texture> tex)
{
    if (unit == 0)
        prtlog_throw(ARC_FATAL, "cannot bind to texture unit 0, since it is reserved.");
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex->__texture_id);
}

nine_patches::nine_patches() = default;

nine_patches::nine_patches(shared<texture> tex) : nine_patches(tex, 1)
{
}

nine_patches::nine_patches(shared<texture> tex, double scale) : scale(scale)
{
    double p13 = 1.0 / 3.0 * tex->width;
    double p23 = 2.0 / 3.0 * tex->height;
    tw = p13 * scale;
    th = p13 * scale;

    lt = cut_texture(tex, quad(0, 0, p13, p13));
    t = cut_texture(tex, quad(p13, 0, p13, p13));
    rt = cut_texture(tex, quad(p23, 0, p13, p13));
    l = cut_texture(tex, quad(0, p13, p13, p13));
    c = cut_texture(tex, quad(p13, p13, p13, p13));
    r = cut_texture(tex, quad(p23, p13, p13, p13));
    lb = cut_texture(tex, quad(0, p23, p13, p13));
    b = cut_texture(tex, quad(p13, p23, p13, p13));
    rb = cut_texture(tex, quad(p23, p23, p13, p13));

#ifndef ARC_Y_IS_DOWN
    std::swap(lt, lb);
    std::swap(rt, rb);
    std::swap(t, b);
#endif
}

void nine_patches::make_vtx(brush *brush, const quad &dst) const
{
    double nw = ceil(dst.width / tw);
    double nh = ceil(dst.height / tw);
    double rw = fmod(dst.width, tw);
    if (rw == 0)
        rw = tw;
    double rh = fmod(dst.height, th);
    if (rh == 0)
        rh = th;
    double x = dst.x, y = dst.y;
    double x2 = x + (__enable_overlapping ? dst.width - tw : tw * (nw - 1));
    double y2 = y + (__enable_overlapping ? dst.height - th : th * (nh - 1));

    for (int i = 1; i < nw - 1; i++)
        for (int j = 1; j < nh - 1; j++)
        {
            double w1 = i == nw - 2 ? rw : tw;
            double h1 = j == nh - 2 ? rh : th;
            brush->draw_texture(c, quad(x + i * tw, y + j * th, w1, h1), quad(c->width - w1, 0, w1, h1));
        }

    brush->draw_texture(lt, quad(x, y, tw, th));

    for (int i = 1; i < nh - 1; i++)
    {
        double h1 = i == nh - 2 ? rh : th;
        brush->draw_texture(l, quad(x, y + i * th, tw, h1), quad(0, 0, tw, h1));
    }

    for (int i = 1; i < nw - 1; i++)
    {
        double w1 = i == nw - 2 ? rw : tw;
        brush->draw_texture(t, quad(x + i * tw, y, w1, th), quad(b->width - w1, 0, w1, th));
    }

    brush->draw_texture(rt, quad(x2, y, tw, th));

    for (int i = 1; i < nh - 1; i++)
    {
        double h1 = i == nh - 2 ? rh : th;
        brush->draw_texture(r, quad(x2, y + i * th, tw, h1), quad(0, 0, tw, h1));
    }

    brush->draw_texture(lb, quad(x, y2, tw, th));

    for (int i = 1; i < nw - 1; i++)
    {
        double w1 = i == nw - 2 ? rw : tw;
        brush->draw_texture(b, quad(x + i * tw, y2, w1, th), quad(t->width - w1, 0, w1, th));
    }

    brush->draw_texture(rb, quad(x2, y2, tw, th));
}

} // namespace arcaie::gfx