#pragma once
#include <core/def.h>
#include <gfx/camera.h>
#include <gfx/color.h>
#include <gfx/state.h>
#include <stack>
#include <gfx/image.h>
#include <gfx/buffer.h>
#include <gfx/shader.h>
#include <core/math.h>
#include <gfx/mesh.h>

namespace arc::gfx
{

struct brush
{
    color vertex_color[4]{};
    std::stack<transform> P_tstack;
    camera P_camera;
    graph_state P_state;
    std::shared_ptr<program> P_default_colored;
    std::shared_ptr<program> P_default_textured;
    complex_buffer *wbuf;
    mesh *P_mesh_root;
    bool P_is_in_mesh = false;
    // when true, the brush will clear the buffer when flushed.
    bool P_clear_when_flush = true;

    brush();

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
    void assert_texture(std::shared_ptr<texture> tex);
    void use_camera(const camera &cam);
    void use_program(std::shared_ptr<program> program);
    void use_state(const graph_state &sts);

    void draw_texture(std::shared_ptr<texture> tex, const quad &dst, const quad &src,
                      long flag = brush_flag::NO);
    void draw_texture(std::shared_ptr<texture> tex, const quad &dst, long flag = brush_flag::NO);
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

} // namespace arc::gfx