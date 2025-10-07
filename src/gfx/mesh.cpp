#include <gfx/mesh.h>

// clang-format off
#include <gl/glew.h>
#include <gl/gl.h>
// clang-format on

namespace arcaie::gfx
{

mesh::mesh()
{
    buffer = make_buffer();
    P_brush = make_brush(buffer);
    P_brush->P_mesh_root = this;
}

mesh::~mesh()
{
    glDeleteVertexArrays(1, &P_vao);
    glDeleteBuffers(1, &P_vbo);
    glDeleteBuffers(1, &P_ebo);
}

brush *mesh::retry()
{
    buffer->clear();
    if (!P_is_direct)
        P_brush->P_is_in_mesh = true;
    return P_brush.get();
}

void mesh::record()
{
    state = P_brush->P_state;
    if (!P_is_direct)
        P_brush->P_is_in_mesh = false;
}

void mesh::draw(brush *gbrush)
{
    auto old_state = gbrush->P_state;
    auto old_buf = gbrush->wbuf;
    auto old_msh = gbrush->P_mesh_root;

    gbrush->use(state);
    gbrush->wbuf = buffer;
    gbrush->P_mesh_root = this;
    gbrush->P_clear_when_flush = false;

    gbrush->flush();

    gbrush->P_clear_when_flush = true;
    gbrush->P_mesh_root = old_msh;
    gbrush->wbuf = old_buf;
    gbrush->use(old_state);
}

shared<mesh> make_mesh()
{
    shared<mesh> msh = std::make_unique<mesh>();
    shared<complex_buffer> buf = msh->buffer;

    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    msh->P_vao = vao;
    msh->P_vbo = vbo;
    msh->P_ebo = ebo;

    return msh;
}

} // namespace arcaie::gfx
