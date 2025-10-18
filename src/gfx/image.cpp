#include <core/io.h>
#include <core/log.h>
#include <gfx/brush.h>
#include <gfx/image.h>


// clang-format off
#include <gl/glew.h>
#include <gl/gl.h>
// clang-format on

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace arc::gfx
{

image::image() = default;

image::image(uint8_t *data, int w, int h) : width(w), height(h), pixels(data)
{
}

image::~image()
{
    if (P_is_from_stb)
        stbi_image_free(pixels);
    else
        delete[] pixels;
    pixels = nullptr;
}

std::shared_ptr<image> image::load(const path_handle &path)
{
    std::shared_ptr<image> img = std::make_shared<image>();
    img->pixels = stbi_load(path.abs_path.c_str(), &img->width, &img->height, nullptr, 4);
    if (img->pixels == nullptr)
        print_throw(ARC_FATAL, "path not found: {}", path.abs_path);
    img->P_is_from_stb = true;
    return img;
}

std::shared_ptr<image> image::make(int width, int height, uint8_t *data)
{
    std::shared_ptr<image> img = std::make_shared<image>();
    img->width = width;
    img->height = height;
    img->pixels = data;
    img->P_is_from_stb = false;
    return img;
}

texture::~texture()
{
    if (root == nullptr)
        glDeleteTextures(1, &P_texture_id);
}

std::shared_ptr<texture> texture::make(std::shared_ptr<image> img)
{
    std::shared_ptr<texture> tex = std::make_shared<texture>();
    unsigned int id;
    glGenTextures(1, &id);
    tex->P_texture_id = id;

    if (img != nullptr)
        tex->P_link_data(img);

    return tex;
}

void texture::parameters(texture_parameters param)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, P_texture_id);

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

void texture::P_link_data(std::shared_ptr<image> img)
{
    P_relying_image = img;
    full_width = width = img->width;
    full_height = height = img->height;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, P_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);

    parameters({});

    glBindTexture(GL_TEXTURE_2D, 0);
}

std::shared_ptr<texture> texture::cut(const quad &src)
{
    std::shared_ptr<texture> ntex = std::make_shared<texture>();
    ntex->width = src.width;
    ntex->height = src.height;
    ntex->full_width = full_width;
    ntex->full_height = full_height;
    ntex->u = src.x;
    ntex->v = src.y;
    ntex->root = shared_from_this();
    ntex->P_relying_image = P_relying_image;
    ntex->P_texture_id = P_texture_id;
    ntex->P_is_framebuffer = P_is_framebuffer;
    return ntex;
}

void texture::P_bind(int unit)
{
    if (unit == 0)
        print_throw(ARC_FATAL, "cannot bind to texture unit 0, since it is reserved.");
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, P_texture_id);
}

nine_patches::nine_patches() = default;

nine_patches::nine_patches(std::shared_ptr<texture> tex) : nine_patches(tex, 1)
{
}

nine_patches::nine_patches(std::shared_ptr<texture> tex, double scale) : scale(scale)
{
    double p13 = 1.0 / 3.0 * tex->width;
    double p23 = 2.0 / 3.0 * tex->height;
    tw = p13 * scale;
    th = p13 * scale;

    lt = tex->cut(quad(0, 0, p13, p13));
    t = tex->cut(quad(p13, 0, p13, p13));
    rt = tex->cut(quad(p23, 0, p13, p13));
    l = tex->cut(quad(0, p13, p13, p13));
    c = tex->cut(quad(p13, p13, p13, p13));
    r = tex->cut(quad(p23, p13, p13, p13));
    lb = tex->cut(quad(0, p23, p13, p13));
    b = tex->cut(quad(p13, p23, p13, p13));
    rb = tex->cut(quad(p23, p23, p13, p13));

#ifndef ARC_Y_IS_DOWN
    std::swap(lt, lb);
    std::swap(rt, rb);
    std::swap(t, b);
#endif
}

void nine_patches::make_vtx(std::shared_ptr<brush> brush, const quad &dst) const
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
    double x2 = x + (P_enable_overlapping ? dst.width - tw : tw * (nw - 1));
    double y2 = y + (P_enable_overlapping ? dst.height - th : th * (nh - 1));

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

} // namespace arc::gfx