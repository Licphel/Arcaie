#pragma once
#include <cstring>
#include <gfx/image.h>
#include <gfx/brush.h>
#include <gfx/cbuf.h>
#include <memory>
#include <vector>

namespace arcaie::gfx
{

struct mesh
{
    graph_state state;
    shared<complex_buffer> buffer;
    shared<brush> P_brush;

    /* unstable */ unsigned int P_vao, P_vbo, P_ebo;
    /* unstable */ bool P_is_direct;

    mesh();
    ~mesh();
    // clear the mesh and attempt to redraw it.
    brush *retry();
    // record the mesh state and buffer content, and end the drawing.
    void record();
    // draw the mesh with the brush. the brush should be direct-to-screen.
    void draw(brush *gbrush);
};

shared<mesh> make_mesh();

} // namespace arcaie::gfx