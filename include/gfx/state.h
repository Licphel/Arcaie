#pragma once
#include <core/def.h>
#include <gfx/image.h>
#include <gfx/shader.h>

namespace arc::gfx
{

enum class graph_mode
{
    COLORED_POINT = 0,
    COLORED_LINE = 1,
    COLORED_TRIANGLE = 2,
    COLORED_QUAD = 3,

    // TEXTURED_TRIANGLE = 16, <-- not implemented yet
    TEXTURED_QUAD = 17
};

struct brush_flag
{
    constexpr static long NO = 1L << 0;
    constexpr static long FLIP_X = 1L << 1;
    constexpr static long FLIP_Y = 1L << 2;
};

enum class blend_mode
{
    NORMAL,
    ADDITIVE
};

struct graph_state
{
    graph_mode mode = graph_mode::TEXTURED_QUAD;
    std::shared_ptr<texture> texture = nullptr;
    std::shared_ptr<program> prog = nullptr;
    std::function<void(std::shared_ptr<program> program)> callback_uniform;
};

} // namespace arc::gfx