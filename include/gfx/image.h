#pragma once
#include <core/io.h>
#include <core/def.h>
#include <core/math.h>

namespace arcaie::gfx
{

struct brush;

struct image
{
    int width, height;
    byte *pixels;
    /* unstable */ bool P_is_from_stb = false;

    image();
    image(byte *data, int w, int h);
    ~image();
};

struct texture
{
    int u = 0;
    int v = 0;
    int width = 0;
    int height = 0;
    int fwidth = 0;
    int fheight = 0;
    /* maybe nullptr */ shared<image> P_relying_image = nullptr;
    /* unstable */ unsigned int P_texture_id = 0;
    /* unstable */ bool P_is_framebuffer;
    shared<texture> root = nullptr;

    ~texture();
};

enum class texture_parameter
{
    UV_REPEAT,
    UV_CLAMP,
    UV_MIRROR,
    FILTER_NEAREST,
    FILTER_LINEAR
};

struct texture_parameters
{
    texture_parameter uv = texture_parameter::UV_REPEAT;
    texture_parameter min_filter = texture_parameter::FILTER_NEAREST;
    texture_parameter mag_filter = texture_parameter::FILTER_NEAREST;
};

shared<image> load_image(const path_handle &path);
shared<image> make_image(int width, int height, byte *data);
shared<texture> make_texture(shared<image> img);
void set_texture_parameters(shared<texture> tex, texture_parameters param);
void lazylink_texture_data(shared<texture> tex, shared<image> img);
shared<texture> cut_texture(shared<texture> tex, const quad &src);
void bind_texture(int i, shared<texture> tex);

struct nine_patches
{
    shared<texture> b;
    shared<texture> c;
    shared<texture> l;
    shared<texture> lb;
    shared<texture> lt;
    shared<texture> r;
    shared<texture> rb;
    shared<texture> rt;
    shared<texture> t;
    double scale;
    double th;
    double tw;
    // enable overlapping can make the size control of nine-patches more accurate.
    // however, if your texture contains some transparent parts, it's better to turn it off.
    bool P_enable_overlapping = true;

    nine_patches();
    nine_patches(shared<texture> tex);
    nine_patches(shared<texture> tex, double scale);

    void make_vtx(brush *brush, const quad &dst) const;
};

} // namespace arcaie::gfx