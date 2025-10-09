#pragma once
#include <core/embit.h>
#include <core/math.h>
#include <cstring>
#include <functional>
#include <gfx/camera.h>
#include <gfx/cbuf.h>
#include <gfx/color.h>
#include <gfx/device.h>
#include <stack>

namespace arcaie::gfx
{

enum class brush_flag : long
{
    NO = 1L << 0,
    FLIP_X = 1L << 1,
    FLIP_Y = 1L << 2
};

enum class blend_mode
{
    NORMAL,
    ADDITIVE
};

struct graph_state
{
    graph_mode mode = graph_mode::TEXTURED_QUAD;
    shared<texture> texture = nullptr;
    shared<program> prog = nullptr;
    std::function<void(shared<program> program)> callback_uniform;
    std::function<void(unique<complex_buffer> buf)> callback_buffer_append;
};

// pre-declare
struct mesh;

struct brush
{
    color vertex_color[4]{};
    std::stack<transform> P_tstack;
    camera P_camera;
    graph_state P_state;
    shared<program> P_default_colored;
    shared<program> P_default_textured;
    weak<complex_buffer> wbuf;
    mesh *P_mesh_root;
    bool P_is_in_mesh = false;
    // when true, the brush will clear the buffer when flushed.
    bool P_clear_when_flush = true;

    brush();

    // since brush just holds a weak reference to the buffer, this method gets it and check the validity.
    shared<complex_buffer> lock_buffer();
    graph_state &current_state();
    void cl_norm();
    void cl_set(const color &col);
    void cl_mrg(const color &col);
    void cl_mrg(double v);
    void ts_push();
    void ts_pop();
    void ts_load(const transform &t);
    void ts_trs(const vec2 &v);
    void ts_scl(const vec2 &v);
    void ts_shr(const vec2 &v);
    void ts_rot(double r);
    void ts_rot(const vec2 &v, double r);
    transform get_combined_transform();

    void flush();
    void assert_mode(graph_mode mode);
    void assert_texture(shared<texture> tex);
    void use_camera(const camera &cam);
    void use_program(shared<program> program);
    void use_state(const graph_state &sts);

    void draw_texture(shared<texture> tex, const quad &dst, const quad &src, bitmask<brush_flag> flag = brush_flag::NO);
    void draw_texture(shared<texture> tex, const quad &dst, bitmask<brush_flag> flag = brush_flag::NO);
    void draw_rect(const quad &dst);
    void draw_rect_outline(const quad &dst);
    void draw_triagle(const vec2 &p1, const vec2 &p2, const vec2 &p3);
    void draw_line(const vec2 &p1, const vec2 &p2);
    void draw_point(const vec2 &p);
    void draw_oval(const quad &dst, int segs = 16);
    void draw_oval_outline(const quad &dst, int segs = 16);

    void clear(const color &col);
    void viewport(const quad &quad);
    void scissor(const quad &quad);
    void scissor_end();
    void use_blend(blend_mode mode);
};

unique<brush> make_brush(shared<complex_buffer> buf);

} // namespace arcaie::gfx