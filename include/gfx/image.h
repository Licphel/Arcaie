#pragma once
#include <core/def.h>
#include <core/math.h>
#include <functional>
#include <core/io.h>

namespace arc::gfx
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

    static std::shared_ptr<image> load(const path_handle &path);
    static std::shared_ptr<image> make(int width, int height, byte *data);
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

struct texture : std::enable_shared_from_this<texture>
{
    int u = 0;
    int v = 0;
    int width = 0;
    int height = 0;
    int full_width = 0;
    int full_height = 0;
    /* maybe nullptr */ std::shared_ptr<image> P_relying_image = nullptr;
    /* unstable */ unsigned int P_texture_id = 0;
    /* unstable */ bool P_is_framebuffer;
    std::shared_ptr<texture> root = nullptr;

    ~texture();

    static std::shared_ptr<texture> make(std::shared_ptr<image> img);
    void parameters(texture_parameters param);
    std::shared_ptr<texture> cut(const quad &src);
    void P_link_data(std::shared_ptr<image> img);
    void P_bind(int i);
};

struct nine_patches
{
    std::shared_ptr<texture> b;
    std::shared_ptr<texture> c;
    std::shared_ptr<texture> l;
    std::shared_ptr<texture> lb;
    std::shared_ptr<texture> lt;
    std::shared_ptr<texture> r;
    std::shared_ptr<texture> rb;
    std::shared_ptr<texture> rt;
    std::shared_ptr<texture> t;
    double scale;
    double th;
    double tw;
    // enable overlapping can make the size control of nine-patches more accurate.
    // however, if your texture contains some transparent parts, it's better to turn it off.
    bool P_enable_overlapping = true;

    nine_patches();
    nine_patches(std::shared_ptr<texture> tex);
    nine_patches(std::shared_ptr<texture> tex, double scale);

    void make_vtx(std::shared_ptr<brush> brush, const quad &dst) const;
};

} // namespace arc::gfx