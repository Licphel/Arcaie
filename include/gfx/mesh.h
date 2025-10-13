#pragma once
#include <core/def.h>
#include <gfx/buffer.h>
#include <gfx/state.h>

namespace arc::gfx
{

struct brush;

struct mesh
{
    graph_state state;
    std::shared_ptr<complex_buffer> buffer;
    std::shared_ptr<brush> P_brush;

    /* unstable */ unsigned int P_vao, P_vbo, P_ebo;
    /* unstable */ bool P_is_direct;

    mesh();
    ~mesh();
    // clear the mesh and attempt to redraw it.
    std::shared_ptr<brush> retry();
    // record the mesh state and buffer content, and end the drawing.
    void record();
    // draw the mesh with the brush. the brush should be direct-to-screen.
    void draw(std::shared_ptr<brush> gbrush);

    static std::shared_ptr<mesh> make();
};

} // namespace arc::gfx