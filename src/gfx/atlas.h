#pragma once
#include <core/def.h>
#include <gfx/image.h>

namespace arc::gfx
{

struct atlas
{
    struct P_impl;
    std::unique_ptr<P_impl> P_p;

    int width;
    int height;
    uint8_t *pixels;
    std::shared_ptr<image> output_image = nullptr;
    std::shared_ptr<texture> output_texture = nullptr;

    atlas(int w, int h);
    ~atlas();

    void begin();
    void end();
    // add an image to the atlas, and get its texture.
    std::shared_ptr<texture> accept(std::shared_ptr<image> image);
    // write an image to the atlas.
    void imgcpy(std::shared_ptr<image> image, int dest_x, int dest_y);

    static std::shared_ptr<atlas> make(int w, int h);
};

} // namespace arc::gfx